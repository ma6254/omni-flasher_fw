#ifndef SERIAL_SCREEN_H
#define SERIAL_SCREEN_H

#include "lvgl_app.h"

typedef enum
{
    SERIAL_SCREEN_USB_VCOM = 0, // USB虚拟串口带硬件流控信号
    SERIAL_SCREEN_USB_VCOM_X2,  // USB虚拟双路串口
    SERIAL_SCREEN_USB_RS485,    // USB转RS485带DE信号
    SERIAL_SCREEN_RECORDER,     // 串口记录仪
    SERIAL_SCREEN_ENUM_COUNT,   // 枚举个数
} serial_screen_item_t;

extern const screen_t serial_screen;

#endif // SERIAL_SCREEN_H
