#ifndef MAIN_BOARD_H
#define MAIN_BOARD_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <led_strip.h>
#include <driver/ledc.h>

/*******************************************************************************
 *
 * 引脚定义
 *
 ******************************************************************************/

#define LCD_BL_GPIO (GPIO_NUM_9)    // LCD屏幕：背光
#define LCD_CS_GPIO (GPIO_NUM_14)   // LCD屏幕：使能
#define LCD_SCLK_GPIO (GPIO_NUM_13) // LCD屏幕：时钟
#define LCD_MOSI_GPIO (GPIO_NUM_11) // LCD屏幕：数据输出
#define LCD_DC_GPIO (GPIO_NUM_12)   // LCD屏幕：数据/命令选择
#define LCD_RST_GPIO (GPIO_NUM_15)  // LCD屏幕：复位

#define BUZZER_GPIO (GPIO_NUM_21)   // 蜂鸣器

#define STA_LED_GPIO (GPIO_NUM_48) // 状态指示灯

#define BOOT_KEY_GPIO (GPIO_NUM_0) // BOOT按键

#define KEY1_GPIO (GPIO_NUM_18) // 按键
#define KEY2_GPIO (GPIO_NUM_17) // 按键
#define KEY3_GPIO (GPIO_NUM_16) // 按键

#define read_key1_io() (gpio_get_level(KEY1_GPIO) == 0)
#define read_key2_io() (gpio_get_level(KEY2_GPIO) == 0)
#define read_key3_io() (gpio_get_level(KEY3_GPIO) == 0)

/*******************************************************************************
 *
 * 封装引脚操作
 *
 ******************************************************************************/

typedef enum
{
    BOARD_STA_LED_OFF = 0,
    BOARD_STA_LED_BLUE,
    BOARD_STA_LED_GREEN,
} board_sta_led_mode_t;

typedef union
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t resv;
    } rgb;
    uint32_t color;
} led_color_t;

#define LED_COLOR_MAKE(r, g, b) ((led_color_t){{r, g, b, 0}})

#define LED_COLOR_BLACK LED_COLOR_MAKE(0, 0, 0)       // 黑色
#define LED_COLOR_WHITE LED_COLOR_MAKE(255, 255, 255) // 白色

#define LED_COLOR_RED LED_COLOR_MAKE(255, 0, 0)       // 红色
#define LED_COLOR_GREEN LED_COLOR_MAKE(0, 255, 0)     // 绿色
#define LED_COLOR_BLUE LED_COLOR_MAKE(0, 0, 255)      // 蓝色

#define LED_COLOR_PURPLE LED_COLOR_MAKE(255, 0, 255)      // 紫色
#define LED_COLOR_CYAN LED_COLOR_MAKE(0, 255, 255)      // 青色
#define LED_COLOR_YELLOW LED_COLOR_MAKE(255, 255, 0)      // 黄色

typedef struct GLOBAL_BOARD_T
{
    led_strip_handle_t led_strip;
    // temperature_sensor_handle_t temp_handle;
    // float inte_temp;
} global_board_t;

extern global_board_t g_board;

/*******************************************************************************
 *
 * 导出函数
 *
 ******************************************************************************/

void board_init(void);
led_color_t led_color_make(uint8_t r, uint8_t g, uint8_t b);
void set_led_color(led_color_t color);


#endif // MAIN_BOARD_H
