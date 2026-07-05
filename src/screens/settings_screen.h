#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H


#include "lvgl_app.h"

typedef struct
{
    char *name;
    bool is_impled; // 是否已实现
    int32_t icon_scale_a;
    int32_t icon_scale_b;
} settings_screen_item_cfg_t;

typedef struct
{
    const settings_screen_item_cfg_t *cfg;
    lv_obj_t *cont;
    lv_obj_t *btn;
    lv_obj_t *title_label;
} settings_screen_item_handle_t;

typedef enum
{
    SETTINGS_SCREEN_LANGUAGE = 0, // 语言设置
    SETTINGS_SCREEN_BRIGHTNESS,   // 屏幕亮度设置
    SETTINGS_SCREEN_BUZZER,       // 蜂鸣器设置
    SETTINGS_SCREEN_INFO,         // 设备信息
    SETTINGS_SCREEN_ABOUT,        // 关于
    SETTINGS_SCREEN_ENUM_COUNT,   // 枚举个数
} settings_screen_item_t;

extern const screen_t settings_screen;

#endif // SETTINGS_SCREEN_H
