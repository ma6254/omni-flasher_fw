#ifndef LVGL_APP_H
#define LVGL_APP_H

#include <stdint.h>
#include <lvgl.h>
#include <esp_lvgl_port.h>
// #include "config.h"

/*******************************************************************************
 *
 * 参数配置
 *
 ******************************************************************************/

#define LCD_HOR_PIXEL (240) // LCD水平像素数
#define LCD_VER_PIXEL (320) // LCD垂直像素数

#define LCD_DRAW_BUFFER_HEIGHT (40) // LVGL绘图缓冲区高度，单位像素行数，建议设置为屏幕水平像素数的整数倍

#define LCD_SPI_NUM (SPI2_HOST)                   //
#define LCD_PIXEL_CLOCK_HZ (80 * 1000 * 1000)     //
#define LCD_CMD_BITS (8)                          //
#define LCD_PARAM_BITS (8)                        // 
#define LCD_COLOR_SPACE (ESP_LCD_COLOR_SPACE_RGB) // 颜色通道顺序
#define LCD_BITS_PER_PIXEL (16)                   // 每个像素占用的位数，RGB565是16位

#define SCREEN_ENCODER_SLEEP_TIMEOUT_MS (30 * 1000) // 旋钮睡眠时间，单位ms

#define LCD_BL_LEDC_FREQ (16000) // 16 kHz
#define LCD_BL_LEDC_TIMER LEDC_TIMER_1
#define LCD_BL_LEDC_CHANNEL LEDC_CHANNEL_1

/*******************************************************************************
 *
 * 结构定义
 *
 ******************************************************************************/

typedef enum
{
    SCREEN_BRIGHTNESS_1 = 1,
    SCREEN_BRIGHTNESS_2,
    SCREEN_BRIGHTNESS_3,
    SCREEN_BRIGHTNESS_4,
    SCREEN_BRIGHTNESS_5,
} screen_brightness_t;

#define SCREEN_BRIGHTNESS_MIN SCREEN_BRIGHTNESS_1
#define SCREEN_BRIGHTNESS_MAX SCREEN_BRIGHTNESS_5

typedef void (*screen_init_cb_t)(void);
typedef void (*screen_deinit_cb_t)(void);
typedef void (*screen_loop_cb_t)(void);

typedef struct
{
    const char *name;             // 界面名称
    screen_init_cb_t init_cb;     // 界面初始化
    screen_deinit_cb_t deinit_cb; // 界面反初始化
    screen_loop_cb_t loop_cb;     // 界面循环

    void *switch_screen_user_data; // 界面切换时的用户数据
} screen_t;

/*******************************************************************************
 *
 * 导出函数
 *
 ******************************************************************************/
 
void set_lcd_bl_brightness(uint32_t brightness);
void set_lcd_bl_on(void);

#define set_lcd_bl_off() (set_lcd_bl_brightness(0))

extern uint32_t screen_encoder_sleep_flag;
extern uint32_t screen_encoder_activity_flag;

void lvgl_app_init(void);
void set_lcd_on(void);

void screen_init(void);
esp_err_t screen_switch(const screen_t *screen);
esp_err_t screen_switch_arg(const screen_t *screen, void *user_data);
screen_t *screen_get_prev(void);
lv_indev_t *screen_get_indev(void);
void screen_clear(void);
void screen_loop_handler(void);

void screen_set_encoder_sleep_timer_period(uint32_t period);
void screen_reset_encoder_sleep_timer_reset(void);

void *screen_get_switch_user_data(void);

#endif // LVGL_APP_H
