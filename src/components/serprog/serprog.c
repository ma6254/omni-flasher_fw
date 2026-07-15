#include <esp_log.h>
#include <esp_mac.h>
#include <esp_task_wdt.h>
#include <esp_vfs_dev.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "serprog.h"
#include "tinyusb.h"
#include "driver/spi_master.h"
#include "board.h"

#include "tinyusb_default_config.h"
#include "tinyusb_cdc_acm.h"
#include "tusb.h"

#define SERPROG_USB_STRING_DESC_COUNT 5
#define SERPROG_IFACE_VERSION 0x01
#define SERPROG_SPI_FREQ_DEFAULT 10000000 // 10 MHz

#define SERPROG_SPI_HOST SPI3_HOST

static const uint32_t SERPROG_CMD_MAP = //
    (1 << SERPROG_CMD_NOP) |            //
    (1 << SERPROG_CMD_Q_IFACE) |        //
    (1 << SERPROG_CMD_Q_CMDMAP) |       //
    (1 << SERPROG_CMD_Q_PGMNAME) |      //
    (1 << SERPROG_CMD_Q_SERBUF) |       //
    (1 << SERPROG_CMD_Q_BUSTYPE) |      //
    (1 << SERPROG_CMD_SYNCNOP) |        //
    (1 << SERPROG_CMD_O_SPIOP) |        //
    (1 << SERPROG_CMD_S_BUSTYPE) |      //
    (1 << SERPROG_CMD_S_SPI_FREQ) |     //
    (1 << SERPROG_CMD_Q_WRNMAXLEN) |    //
    (1 << SERPROG_CMD_Q_RDNMAXLEN);     //

#define SERPROG_BUS_SPI (1 << 3)

#define SERPROG_SUPPORTED_BUS SERPROG_BUS_SPI

#define SERPROG_CDCACM_ITF TINYUSB_CDC_ACM_0

static const char *TAG = "serprog";
static char serprog_langid_desc[] = {0x09, 0x04}; // 0: LangID = 0x0409 (English)
const char *hex_char = "0123456789ABCDEF";
static uint8_t mac[6];
static char mac_str[13]; // 12 characters + null terminator
static const char *serprog_string_desc[SERPROG_USB_STRING_DESC_COUNT] = {
    serprog_langid_desc,
    "ma6254",             // 1: Manufacturer
    "OmniFlasher",        // 2: Product (设备名)
    mac_str,              // 3: Serial
    "OmniFlasher serprog" // 4: CDC Interface STRID_CDC_INTERFACE
};

static QueueHandle_t serprog_rx_queue = NULL;
static TickType_t last_rx_tick = 0;
static bool serprog_rx_active = false;
static spi_device_handle_t serprog_spi_device_handle = NULL;
static uint32_t serprog_spi_freq = SERPROG_SPI_FREQ_DEFAULT;
static bool serprog_spi_initialized = false;

static esp_err_t serprog_spi_dev_init(void);
static void serprog_task(void *arg);

static void serprog_tinyusb_event_callback(tinyusb_event_t *event, void *arg)
{
    (void)arg;
    switch (event->id)
    {
    case TINYUSB_EVENT_ATTACHED:
        ESP_LOGI(TAG, "USB Mounted");
        break;
    case TINYUSB_EVENT_DETACHED:
        ESP_LOGI(TAG, "USB Unmounted");
        break;
#ifdef CONFIG_TINYUSB_SUSPEND_CALLBACK
    case TINYUSB_EVENT_SUSPENDED:
        ESP_LOGI(TAG, "USB Suspend");
        break;
#endif
#ifdef CONFIG_TINYUSB_RESUME_CALLBACK
    case TINYUSB_EVENT_RESUMED:
        ESP_LOGI(TAG, "USB Resume");
        break;
#endif
    default:
        break;
    }
}

#define SERPROG_SPI_BUF_SIZE 4092

static uint8_t rx_buf[1024] = {0};
static uint8_t tx_buf[1024] = {0};

static uint8_t *serprog_write_buf = NULL;
static uint8_t *serprog_read_buf = NULL;

static void serprog_flush_tx(void)
{
    esp_err_t err = tinyusb_cdcacm_write_flush(SERPROG_CDCACM_ITF, portMAX_DELAY);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to flush CDC ACM. Error: %s", esp_err_to_name(err));
    }
}

static void serprog_put(const uint8_t *data, size_t len)
{
    // ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);

    size_t bytes_written = tinyusb_cdcacm_write_queue(SERPROG_CDCACM_ITF, data, len);
    if (bytes_written != len)
    {
        ESP_LOGE(TAG, "Failed to write all bytes to CDC ACM. Written: %d, Expected: %d", bytes_written, len);
    }


}

static void serprog_put_u8(uint8_t data)
{
    // ESP_LOGE(TAG, "put_u8: %02" PRIX8, data);

    size_t bytes_written = tinyusb_cdcacm_write_queue(SERPROG_CDCACM_ITF, &data, 1);
    if (bytes_written != 1)
    {
        ESP_LOGE(TAG, "Failed to write byte to CDC ACM. Written: %d, Expected: 1", bytes_written);
    }
}

static void serprog_put_u32(uint32_t data)
{
    // ESP_LOGE(TAG, "put_u32: %08" PRIX32, data);

    uint8_t buf[4];
    buf[0] = data & 0xFF;
    buf[1] = (data >> 8) & 0xFF;
    buf[2] = (data >> 16) & 0xFF;
    buf[3] = (data >> 24) & 0xFF;

    size_t bytes_written = tinyusb_cdcacm_write_queue(SERPROG_CDCACM_ITF, buf, 4);
    if (bytes_written != 4)
    {
        ESP_LOGE(TAG, "Failed to write 32-bit value to CDC ACM. Written: %d, Expected: 4", bytes_written);
    }
}

static esp_err_t serprog_get_cmd(uint8_t *data)
{
    uint8_t received_byte;

    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "RX queue is not initialized");
        return ESP_FAIL;
    }

    if (xQueueReceive(serprog_rx_queue, &received_byte, 0) != pdTRUE)
    {
        // ESP_LOGE(TAG, "Failed to receive byte from RX queue");
        return ESP_FAIL;
    }

    *data = received_byte;
    return ESP_OK;
}

static esp_err_t serprog_get_u8(uint8_t *data)
{
    uint8_t received_byte;

    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "RX queue is not initialized");
        return ESP_FAIL;
    }

    if (xQueueReceive(serprog_rx_queue, &received_byte, portMAX_DELAY) != pdTRUE)
    {
        // ESP_LOGE(TAG, "Failed to receive byte from RX queue");
        return ESP_FAIL;
    }

    *data = received_byte;
    return ESP_OK;
}

static esp_err_t serprog_get_u24(uint32_t *data)
{
    uint8_t rx_data[3];

    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "RX queue is not initialized");
        return ESP_FAIL;
    }

    for (uint32_t i = 0; i < 3; i++)
    {
        uint8_t a;

        if (xQueueReceive(serprog_rx_queue, &a, portMAX_DELAY) != pdTRUE)
        {
            // ESP_LOGE(TAG, "Failed to receive byte from RX queue");
            return ESP_FAIL;
        }

        rx_data[i] = a;
    }

    *data = (rx_data[0] | (rx_data[1] << 8) | (rx_data[2] << 16));
    return ESP_OK;
}
static esp_err_t serprog_get_u32(uint32_t *data)
{
    uint8_t rx_data[4];

    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "RX queue is not initialized");
        return ESP_FAIL;
    }

    for (uint32_t i = 0; i < 4; i++)
    {
        uint8_t a;

        if (xQueueReceive(serprog_rx_queue, &a, portMAX_DELAY) != pdTRUE)
        {
            // ESP_LOGE(TAG, "Failed to receive byte from RX queue");
            return ESP_FAIL;
        }

        rx_data[i] = a;
    }

    *data = (rx_data[0] | (rx_data[1] << 8) | (rx_data[2] << 16) | (rx_data[3] << 24));
    return ESP_OK;
}

static esp_err_t serprog_get_all(uint8_t *data, size_t len, uint32_t *out_len)
{
    uint8_t received_byte;
    uint32_t bytes_received = 0;

    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "RX queue is not initialized");
        return ESP_FAIL;
    }

    if (len == 0)
    {
        ESP_LOGE(TAG, "Length is zero");
        return ESP_FAIL;
    }

    while (1)
    {
        BaseType_t xReturned = xQueueReceive(serprog_rx_queue, &received_byte, 0);
        if (xReturned != pdTRUE)
            break;

        *data++ = received_byte;
        bytes_received++;
        len--;
        if (len == 0)
            break;
    }

    if (out_len)
        *out_len = bytes_received;

    return ESP_OK;
}

static void serprog_spi_init(void)
{
    esp_err_t err;

    set_ic_vcc_on();
    set_ic_oe(IC_OE_1);

    spi_bus_config_t buscfg = {
        .sclk_io_num = IC_SPI1_CLK_GPIO,
        .data0_io_num = IC_SPI1_IO0_GPIO,
        .data1_io_num = IC_SPI1_IO1_GPIO,
        .data2_io_num = IC_SPI1_IO2_GPIO,
        .data3_io_num = IC_SPI1_IO3_GPIO,
        .max_transfer_sz = SERPROG_SPI_BUF_SIZE,
    };

    err = spi_bus_initialize(SERPROG_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(err);

    err = serprog_spi_dev_init();
    ESP_ERROR_CHECK(err);
}

static esp_err_t serprog_spi_dev_init(void)
{
    esp_err_t err;

    if (serprog_spi_device_handle != NULL)
    {
        spi_bus_remove_device(serprog_spi_device_handle);
        serprog_spi_device_handle = NULL;
    }

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = serprog_spi_freq, // Clock out at 10 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = IC_SPI1_CS_GPIO,    // CS pin
        .queue_size = 7,                    // We want to be able to queue 7 transactions at a time
        .pre_cb = NULL,                     // Specify pre-transfer callback to handle D/C line
    };

    err = spi_bus_add_device(SERPROG_SPI_HOST, &dev_cfg, &serprog_spi_device_handle);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

static void serprog_rx_process(uint8_t cmd)
{
    esp_err_t err;
    // ESP_LOGI(TAG, "rx cmd: 0x%02X", cmd);

    switch (cmd)
    {
    case SERPROG_CMD_NOP:
    {
        /***********************************************************************
         * <SERPROG_CMD_NOP> 0x00 No operation
         **********************************************************************/
        serprog_put_u8(SERPROG_ACK);
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_IFACE:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_IFACE> 0x01 Query interface version
         **********************************************************************/
        serprog_put_u8(SERPROG_ACK);
        serprog_put_u8(SERPROG_IFACE_VERSION);
        serprog_put_u8(0);
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_CMDMAP:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_CMDMAP> 0x02 Query command map
         **********************************************************************/

        serprog_put_u8(SERPROG_ACK);
        serprog_put_u32(SERPROG_CMD_MAP);

        for (uint32_t i = 0; i < 32 - sizeof(uint32_t); i++)
        {
            serprog_put_u8(0);
        }

        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_PGMNAME:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_PGMNAME> 0x03 Query programmer name
         **********************************************************************/
        char pgm_name[16];

        serprog_put_u8(SERPROG_ACK);
        memset(pgm_name, 0, sizeof(pgm_name));
        snprintf(pgm_name, sizeof(pgm_name), "%s", "OmniFlasher");
        serprog_put((const uint8_t *)pgm_name, sizeof(pgm_name));
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_SERBUF:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_SERBUF> 0x04 Query serial buffer size
         **********************************************************************/
        serprog_put_u8(SERPROG_ACK);

        // Pretend to be 64K (0xffff)
        serprog_put_u8(0xFF);
        serprog_put_u8(0xFF);
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_BUSTYPE:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_BUSTYPE> 0x05 Query supported bus type(s).
         **********************************************************************/

        serprog_put_u8(SERPROG_ACK);
        serprog_put_u8(SERPROG_SUPPORTED_BUS);
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_WRNMAXLEN:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_WRNMAXLEN> 0x08 Query maximum write length
         **********************************************************************/

        serprog_put_u8(SERPROG_ACK);

        uint32_t word_size = SERPROG_SPI_BUF_SIZE / 2;

        serprog_put_u8(word_size & 0xFF);
        serprog_put_u8((word_size >> 8) & 0xFF);
        serprog_put_u8((word_size >> 16) & 0xFF);

        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_SYNCNOP:
    {
        /***********************************************************************
         * <SERPROG_CMD_SYNCNOP> 0x10 Special no-operation that returns NAK+ACK
         **********************************************************************/

        serprog_put_u8(SERPROG_NACK);
        serprog_put_u8(SERPROG_ACK);
        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_Q_RDNMAXLEN:
    {
        /***********************************************************************
         * <SERPROG_CMD_Q_WRNMAXLEN> 0x08 Query maximum write length
         **********************************************************************/

        serprog_put_u8(SERPROG_ACK);

        uint32_t word_size = SERPROG_SPI_BUF_SIZE / 2;

        serprog_put_u8(word_size & 0xFF);
        serprog_put_u8((word_size >> 8) & 0xFF);
        serprog_put_u8((word_size >> 16) & 0xFF);

        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_S_BUSTYPE:
    {
        /***********************************************************************
         * <SERPROG_CMD_S_BUSTYPE> 0x12 Set used bus type(s).
         **********************************************************************/

        uint8_t bustype = 0;

        err = serprog_get_u8(&bustype);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get bustype: %s", esp_err_to_name(err));
            return;
        }

        if ((bustype != 0) && ((bustype | SERPROG_SUPPORTED_BUS) == SERPROG_SUPPORTED_BUS))
        {
            serprog_put_u8(SERPROG_ACK);
        }
        else
        {
            serprog_put_u8(SERPROG_NACK);
        }

        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_O_SPIOP:
    {
        /***********************************************************************
         * <SERPROG_CMD_O_SPIOP> 0x13 Perform SPI operation
         **********************************************************************/
        uint32_t wlen = 0;
        uint32_t rlen = 0;

        serprog_get_u24(&wlen);
        serprog_get_u24(&rlen);

        // ESP_LOGE(TAG, "spi_operation: wlen=%" PRIu32 ", rlen=%" PRIu32, wlen, rlen);
        
        if (wlen > 0)
        {
            serprog_get_all(serprog_write_buf, wlen, NULL);

            // ESP_LOGE(TAG, "spi_write_buffer:");
            // ESP_LOG_BUFFER_HEXDUMP(TAG, serprog_write_buf, wlen, ESP_LOG_INFO);
        }

        if (serprog_spi_initialized == false)
        {
            err = serprog_spi_dev_init();
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to initialize SPI device: %s", esp_err_to_name(err));
                serprog_put_u8(SERPROG_NACK);
                serprog_flush_tx();
                break;
            }
            serprog_spi_initialized = true;
        }

        err = spi_device_acquire_bus(serprog_spi_device_handle, portMAX_DELAY);
        if (err != ESP_OK)
        {
            serprog_put_u8(SERPROG_NACK);
            serprog_flush_tx();
            break;
        }

        uint32_t total_len = wlen + rlen;

        spi_transaction_t trans = {
            .flags = 0,
            .length = total_len * 8,
            .tx_buffer = serprog_write_buf,
            .rxlength = total_len * 8,
            .rx_buffer = serprog_read_buf,
        };

        err = spi_device_polling_transmit(serprog_spi_device_handle, &trans);
        if (err != ESP_OK)
        {
            spi_device_release_bus(serprog_spi_device_handle);
            serprog_put_u8(SERPROG_NACK);
            serprog_flush_tx();
            break;
        }

        spi_device_release_bus(serprog_spi_device_handle);

        serprog_put_u8(SERPROG_ACK);

        if (rlen > 0)
        {
            serprog_put(serprog_read_buf + wlen, rlen);
        }

        serprog_flush_tx();
    }
    break;
    case SERPROG_CMD_S_SPI_FREQ:
    {
        /***********************************************************************
         * <SERPROG_CMD_S_SPI_FREQ> 0x14 Set SPI frequency
         **********************************************************************/

        uint32_t freq = 0;

        
        err = serprog_get_u32(&freq);
        if (err != ESP_OK)
        {
            serprog_put_u8(SERPROG_NACK);
            serprog_flush_tx();
            ESP_LOGE(TAG, "Failed to get SPI frequency: %s", esp_err_to_name(err));
            return;
        }

        ESP_LOGI(TAG, "set SPI frequency: %" PRIu32, freq);
        serprog_put_u8(SERPROG_ACK);
        serprog_flush_tx();

        serprog_spi_freq = freq;
        serprog_spi_initialized = false;
    }
    break;
    case SERPROG_CMD_S_PIN_STATE:
    {
        /***********************************************************************
         * <SERPROG_CMD_S_PIN_STATE> 0x15 Enable/disable output drivers
         **********************************************************************/

        uint8_t pin_state = 0;

        err = serprog_get_u8(&pin_state);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get pin_state: %s", esp_err_to_name(err));
            return;
        }

        // Here you would typically set the pin state based on the received value.
        // For now, we just acknowledge the command.

        serprog_put_u8(SERPROG_ACK);
        serprog_flush_tx();
    }
    break;
    default:
    {
        ESP_LOGW(TAG, "Unsupported cmd: 0x%02X", cmd);
        serprog_put_u8(SERPROG_NACK);
        serprog_flush_tx();

        while (1)
        {
            esp_task_wdt_reset();
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    break;
    }
}

static void serprog_tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    // ESP_LOGI(TAG, "CDC RX callback: itf=%d, event=%d", itf, event->type);

    /* initialization */
    size_t rx_size = 0;
    /* read from usb */
    esp_err_t ret = tinyusb_cdcacm_read(itf, rx_buf, 1024, &rx_size);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read from CDC ACM: %s", esp_err_to_name(ret));
        return;
    }
    for (size_t i = 0; i < rx_size; i++)
    {
        if (xQueueSend(serprog_rx_queue, &rx_buf[i], 0) != pdTRUE)
        {
            ESP_LOGW(TAG, "RX queue full, drop byte at idx=%u", (unsigned)i);
            break;
        }
    }
    last_rx_tick = xTaskGetTickCount();
    serprog_rx_active = true;

    // ESP_LOGI(TAG, "CDC RX callback: rx_size=%d", rx_size);
}

static void serprog_tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
}

static void serprog_tinyusb_cdc_line_coding_changed_callback(int itf, cdcacm_event_t *event)
{
}

/*******************************************************************************
 * @brief 初始化serprog模块
 * @param None
 * @return none
 ******************************************************************************/
esp_err_t serprog_init(void)
{
    esp_err_t err;

    serprog_spi_init();

    serprog_write_buf = heap_caps_malloc(SERPROG_SPI_BUF_SIZE,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if(serprog_write_buf==NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate write buffer");
        return ESP_FAIL;
    }

    serprog_read_buf = heap_caps_malloc(SERPROG_SPI_BUF_SIZE,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if(serprog_read_buf==NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate read buffer");
        return ESP_FAIL;
    }

    serprog_rx_queue = xQueueCreate(1024, sizeof(uint8_t));
    if (serprog_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create RX queue");
        return ESP_FAIL;
    }
    last_rx_tick = xTaskGetTickCount();

    tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG(serprog_tinyusb_event_callback);

    err = esp_read_mac(mac, ESP_MAC_BASE);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read MAC address: %s", esp_err_to_name(err));
    }

    for (uint32_t i = 0; i < sizeof(mac); i++)
    {
        mac_str[i * 2] = hex_char[(mac[i] >> 4) & 0x0F];
        mac_str[i * 2 + 1] = hex_char[mac[i] & 0x0F];
    }
    mac_str[12] = '\0'; // Null-terminate the string

    tusb_cfg.descriptor.string = (const char **)serprog_string_desc;
    tusb_cfg.descriptor.string_count = SERPROG_USB_STRING_DESC_COUNT;
    err = tinyusb_driver_install(&tusb_cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install TinyUSB driver: %s", esp_err_to_name(err));
        return err;
    }

    tinyusb_config_cdcacm_t amc_cfg = {
        .cdc_port = SERPROG_CDCACM_ITF,
        .callback_rx = &serprog_tinyusb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &serprog_tinyusb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = &serprog_tinyusb_cdc_line_coding_changed_callback,
    };

    err = tinyusb_cdcacm_init(&amc_cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize TinyUSB CDC ACM: %s", esp_err_to_name(err));
        return err;
    }

    xTaskCreate(serprog_task,   //
                "serprog_task", //
                8192,           //
                NULL,           //
                5,              //
                NULL);          //

    ESP_LOGI(TAG, "serprog initialized");
    return ESP_OK;
}

void serprog_deinit(void)
{
    tinyusb_driver_uninstall();
    ESP_LOGI(TAG, "serprog deinitialized");
}

static void serprog_task(void *arg)
{
    esp_err_t err;
    (void)arg;
    esp_task_wdt_add(NULL);
    serprog_flush_tx();

    while (1)
    {
        esp_task_wdt_reset();
        // TickType_t current_tick = xTaskGetTickCount();

        // if (serprog_rx_active && ((current_tick - last_rx_tick) > pdMS_TO_TICKS(1)))
        // {
        //     serprog_rx_active = false;
        //     last_rx_tick = current_tick; // Reset the tick count to avoid repeated logs

        //     // uint32_t rx_len = 0;
        //     // serprog_get_all(rx_buf, sizeof(rx_buf), &rx_len);
        //     // ESP_LOGI(TAG, "Received data from %" PRIu32 " bytes", rx_len);

        //     // ESP_LOG_BUFFER_HEXDUMP(TAG, rx_buf, rx_len, ESP_LOG_INFO);

        // }

        uint8_t cmd = 0;

        err = serprog_get_cmd(&cmd);
        if (err != ESP_OK)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
            continue;
        }

        serprog_rx_process(cmd);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
