#include <string.h>
#include "board.h"
#include "esp_err.h"
#include <esp_log.h>
#include <driver/uart.h>

global_board_t g_board;
static const char *TAG = "board";
QueueHandle_t target_uart_queue = NULL;

/*******************************************************************************
 * @brief 目标UART初始化
 * @param None
 * @return none
 ******************************************************************************/
esp_err_t board_target_uart_init(const uart_config_t *uart_config)
{
    esp_err_t err;

    if (uart_config != NULL)
    {
        ESP_LOGI(TAG, "board_target_uart_init: baud_rate=%d, data_bits=%d, parity=%d, stop_bits=%d, flow_ctrl=%d",
                 uart_config->baud_rate,
                 uart_config->data_bits,
                 uart_config->parity,
                 uart_config->stop_bits,
                 uart_config->flow_ctrl);
    }

    // 设置通信管脚
    err = uart_set_pin(     //
        TAR_UART_PORT,      //
        TAR_UART_TXD_GPIO,  //
        TAR_UART_RXD_GPIO,  //
        UART_PIN_NO_CHANGE, //
        UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(err));
    }

    if (uart_is_driver_installed(TAR_UART_PORT))
    {
        ESP_LOGW(TAG, "UART driver already installed, deleting it first");
        err = uart_driver_delete(TAR_UART_PORT);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to delete UART driver: %s", esp_err_to_name(err));
        }
    }

    err = uart_driver_install( //
        TAR_UART_PORT,         //
        256,                   //
        256,                   //
        10,                    //
        &target_uart_queue,    //
        0);                    //
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(err));
    }

    if (uart_config != NULL)
    {
        err = uart_param_config(TAR_UART_PORT, uart_config);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to configure UART parameters: %s", esp_err_to_name(err));
        }
    }

    // 通信方式选择
   err=uart_set_mode(TAR_UART_PORT, UART_MODE_UART);
   if (err != ESP_OK)
   {
       ESP_LOGE(TAG, "Failed to set UART mode: %s", esp_err_to_name(err));
   }

   err = uart_set_rx_timeout(TAR_UART_PORT, 50);
   if (err != ESP_OK)
   {
       ESP_LOGE(TAG, "Failed to set UART RX timeout: %s", esp_err_to_name(err));
   }

    return ESP_OK;
}

/*******************************************************************************
 * @brief 板级引脚端口初始
 * @param None
 * @return none
 ******************************************************************************/
void board_init(void)
{
    // esp_err_t err;

    memset(&g_board, 0, sizeof(g_board));

    gpio_config_t key_gpio_conf = {
        .pin_bit_mask = (1ULL << KEY1_GPIO) | (1ULL << KEY2_GPIO) | (1ULL << KEY3_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&key_gpio_conf));

    gpio_config_t ic_oe_gpio_conf = {
        .pin_bit_mask = (1ULL << IC_OE1_GPIO) | (1ULL << IC_OE2_GPIO) | (1ULL << IC_OE3_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&ic_oe_gpio_conf));
    set_ic_oe(IC_OE_OFF);

    gpio_config_t vcc_en_gpio_conf = {
        .pin_bit_mask = (1ULL << IC_VCC_EN_GPIO) | (1ULL << JTAG_VCC_EN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&vcc_en_gpio_conf));
    set_ic_vcc_off();
    set_jtag_vcc_off();

    // RGB_LED灯配置
    {
        led_strip_config_t strip_config = {
            .strip_gpio_num = STA_LED_GPIO,                              // 数据引脚
            .max_leds = 1,                                               // 灯珠数量
            .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // 颜色顺序，WS2812一般是GRB
            .flags.invert_out = false,                                   // 如果你的电路有反相器，设为true
        };

        led_strip_rmt_config_t rmt_config = {
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10 * 1000 * 1000, // 10MHz
            .mem_block_symbols = 0,            // 0=默认
            .flags.with_dma = false,
        };

        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &g_board.led_strip));
    }

    // 清空所有灯珠（全部熄灭）
    ESP_ERROR_CHECK(led_strip_clear(g_board.led_strip));

    set_led_color(LED_COLOR_BLACK); // 黑色

    // // 蜂鸣器PWM初始化
    // ledc_timer_config_t ledc_timer = {
    //     .speed_mode = LEDC_LOW_SPEED_MODE,
    //     .duty_resolution = LEDC_TIMER_10_BIT,
    //     .timer_num = BUZZER_LEDC_TIMER,
    //     .freq_hz = BUZZER_LEDC_FREQ,
    //     .clk_cfg = LEDC_AUTO_CLK,
    // };
    // ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // ledc_channel_config_t ledc_channel = {
    //     .speed_mode = LEDC_LOW_SPEED_MODE,
    //     .channel = BUZZER_LEDC_CHANNEL,
    //     .timer_sel = BUZZER_LEDC_TIMER,
    //     .intr_type = LEDC_INTR_DISABLE,
    //     .gpio_num = BUZZER_GPIO,
    //     .duty = 0, // Set duty to 50%
    //     .hpoint = 0,
    // };
    // ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    // set_buzzer_io_off();

    set_ic_oe(IC_OE_3);
    set_ic_vcc_off();
    set_jtag_vcc_off();

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(board_target_uart_init(&uart_config));

    ESP_LOGI(TAG, "init done");
}

led_color_t led_color_make(uint8_t r, uint8_t g, uint8_t b)
{
    led_color_t color;

    memset(&color, 0, sizeof(color));

    color.rgb.r = r;
    color.rgb.g = g;
    color.rgb.b = b;

    return color;
}

void set_led_color(led_color_t color)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(g_board.led_strip, 0, color.rgb.r, color.rgb.g, color.rgb.b));
    ESP_ERROR_CHECK(led_strip_refresh(g_board.led_strip));
}

void set_ic_oe(ic_oe_mode_t mode)
{
    uint8_t mask = 0; 

    switch (mode)
    {
    case IC_OE_OFF:
        mask = 0;
        break;
    case IC_OE_1:
        mask = 1 << 0;
        break;
    case IC_OE_2:
        mask = 1 << 1;
        break;
    case IC_OE_3:
        mask = 1 << 2;
        break;
    default:
        mask = 0;
        break;
    }

    gpio_set_level(IC_OE1_GPIO, (mask & (1 << 0)) ? 1 : 0);
    gpio_set_level(IC_OE2_GPIO, (mask & (1 << 1)) ? 1 : 0);
    gpio_set_level(IC_OE3_GPIO, (mask & (1 << 2)) ? 1 : 0);
}
