#ifndef MAIN_MENU_SCREEN_H
#define MAIN_MENU_SCREEN_H


#include "lvgl_app.h"

typedef struct
{
    char *name;
    bool is_impled; // 是否已实现
    const lv_image_dsc_t *icon;
    int32_t icon_scale_a;
    int32_t icon_scale_b;
} main_menu_screen_item_cfg_t;

typedef struct
{
    const main_menu_screen_item_cfg_t *cfg;
    lv_obj_t *cont;
    lv_obj_t *btn;
    lv_obj_t *icon_img;
} main_menu_screen_item_handle_t;

typedef enum
{
    MAIN_MENU_SCREEN_REMOTE = 0, // 在线模式
    MAIN_MENU_SCREEN_FLASH,      // Flash芯片
    MAIN_MENU_SCREEN_SD,         // SD卡
    MAIN_MENU_SCREEN_SERIAL,     // 串口
    MAIN_MENU_SCREEN_DAPLINK,    // DAP在线调试烧录
    MAIN_MENU_SCREEN_SETTINGS,   // 设置
    MAIN_MENU_SCREEN_SYS_INFO,   // 系统信息
    MAIN_MENU_SCREEN_ENUM_COUNT, // 枚举个数
} main_menu_screen_item_t;

extern const screen_t main_menu_screen;

#endif // MAIN_MENU_SCREEN_H
