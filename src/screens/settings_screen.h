#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H


#include "lvgl_app.h"

typedef struct
{
    char *name;
    bool is_impled; // 是否已实现
} settings_screen_item_cfg_t;

typedef enum
{
    SETTINGS_SCREEN_BRIGHTNESS = 0, // 屏幕亮度设置
    SETTINGS_SCREEN_ENUM_COUNT,     // 枚举个数
} settings_screen_item_t;

extern const screen_t settings_screen;

#endif // SETTINGS_SCREEN_H
