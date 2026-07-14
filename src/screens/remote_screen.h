#ifndef REMOTE_SCREEN_H
#define REMOTE_SCREEN_H

#include "lvgl_app.h"

// <https://www.flashrom.org/supported_hw/supported_prog/serprog/overview.html>

typedef enum
{
    REMOTE_SCREEN_USB = 0,    // USB远程控制
    REMOTE_SCREEN_WIFI_WEB,   // WIFI网页远程控制
    REMOTE_SCREEN_SERPROG,    // FlashROM的serProg协议
    REMOTE_SCREEN_ENUM_COUNT, // 枚举个数
} remote_screen_item_t;

extern const screen_t remote_screen;

#endif // REMOTE_SCREEN_H
