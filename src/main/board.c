#include <string.h>
#include "board.h"
#include <esp_log.h>

global_board_t g_board;
static const char *TAG = "board";

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

