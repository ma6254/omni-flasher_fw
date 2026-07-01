/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <driver/uart.h>
#include "utils.h"
#include "board.h"
#include "key.h"
#include "buzzer.h"
#include "lvgl_app.h"
#include "lvgl_benchmark.h"

static const char *TAG = "main";

static led_color_t led_color_list[] = {
    LED_COLOR_BLACK,  LED_COLOR_RED,  LED_COLOR_GREEN,  LED_COLOR_BLUE,
    LED_COLOR_PURPLE, LED_COLOR_CYAN, LED_COLOR_YELLOW, LED_COLOR_WHITE};

const size_t LED_COLOR_LIST_LEN =
    sizeof(led_color_list) / sizeof(led_color_list[0]);

int led_color_list_index = -1;
int ic_oe_index = -1;

uint32_t led_color_time_cnt = 0;

static char target_uart_rx_buf[1024] = {0};

static esp_err_t target_uart_rx_process(TickType_t ticks_to_wait)
{
    BaseType_t xReturned;
    uart_event_t event;
    int rx_len = 0;

    xReturned = xQueueReceive(target_uart_queue, &event, ticks_to_wait);
    if (xReturned == pdFAIL)
    {
        return ESP_ERR_TIMEOUT; // 超时
    }

    target_uart_rx_buf[0] = 0;

    switch (event.type)
    {
    case UART_DATA:
    {
        /***********************************************************************
         * 处理接收到的UART数据
         **********************************************************************/

        // ESP_LOGI(TAG, "Received UART data");

        // uart_flush_input(TAR_UART_PORT);

        ESP_ERROR_CHECK(uart_get_buffered_data_len(TAR_UART_PORT, (size_t *)&rx_len));

        // ESP_LOGI(TAG, "Received UART data len:%d", rx_len);

        rx_len = uart_read_bytes(TAR_UART_PORT, target_uart_rx_buf, rx_len, 0);
        if (rx_len == -1)
        {
            ESP_LOGW(TAG, "Failed to read UART data %d", event.type);
            break;
        }

        uart_flush_input(TAR_UART_PORT);

        target_uart_rx_buf[rx_len] = 0;
        ESP_LOGI(TAG, "Received UART len:%d data:%s", rx_len, target_uart_rx_buf);
        // print_quote_string((char *)target_uart_rx_buf);
        // rx_len++;

        // ESP_LOG_BUFFER_HEX_LEVEL(TAG, target_uart_rx_buf, rx_len, ESP_LOG_INFO);
    }
    break;
    default:
        ESP_LOGI(TAG, "Unhandled UART event: %d", event.type);
        return ESP_ERR_INVALID_STATE;
    }

    return ESP_OK;
}

/*******************************************************************************
 * @brief 主函数，程序入口
 * @param None
 * @return none
 ******************************************************************************/
void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "Build time: %04d-%02d-%02d %s", __YEAR__, __MONTH__, __DAY__, __TIME__);

    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();

    esp_task_wdt_add(NULL);
    board_init();
    key_init();
    buzzer_init();
    lvgl_app_init();
    screen_init();
    // lv_demo_benchmark_init();
    
    vTaskDelay(250 / portTICK_PERIOD_MS);
    
    set_buzzer_io_on();
    vTaskDelay(250 / portTICK_PERIOD_MS);
    set_buzzer_io_off();
    
    // uint8_t key1_state = 0;
    // set_lcd_bl_on();

    while (1)
    {
        esp_task_wdt_reset();
        screen_loop_handler();
        key_task_handler();
        buzzer_task_process();
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if(led_color_time_cnt > 0)
        {
            led_color_time_cnt--;
        }
        else
        {
            led_color_time_cnt = 50;
        }

        if (led_color_time_cnt == 0)
        {
            
            // led_color_list_index++;
            // led_color_list_index %= LED_COLOR_LIST_LEN;
            // led_color_t color = led_color_list[led_color_list_index];
            // set_led_color(color);

            // if (led_color_list_index % 2 == 0)
            // {
            //     set_lcd_bl_on();
            // }
            // else
            // {
            //     set_lcd_bl_off();
            // }

            // ic_oe_index++;
            // ic_oe_index %= 2;

            // if(ic_oe_index == 0)
            // {
            //     set_ic_vcc_on();
            //     set_jtag_vcc_off();
            // }
            // else
            // {
            //     set_ic_vcc_off();
            //     set_jtag_vcc_on();
            // }

            // buzzer_set(100, 1, true);


            // uart_wait_tx_done(TAR_UART_PORT, portMAX_DELAY);
            // uart_write_bytes(TAR_UART_PORT, "hello\r\n", strlen("hello\r\n"));
        }

        target_uart_rx_process(0);
    }
}
 