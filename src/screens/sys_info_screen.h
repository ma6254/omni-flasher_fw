#ifndef SYS_INFO_SCREEN_H
#define SYS_INFO_SCREEN_H

#include "lvgl_app.h"

typedef struct SYS_INFO_SCREEN_ITEM_T sys_info_screen_item_t;

typedef void (*sys_info_screen_item_update_cb_t)(sys_info_screen_item_t *item);

typedef struct SYS_INFO_SCREEN_ITEM_T
{
    const char *name;                           // 项目显示名称
    bool neet_updated;                          // 是否需要实时刷新，否则只会在初始化时取值
    sys_info_screen_item_update_cb_t update_cb; // 刷新函数
    void *update_cb_user_data;
    lv_obj_t *label;

} sys_info_screen_item_t;


typedef enum
{
    SYSINFO_ITEM_PRODUCT_NAME = 0, // 产品名称
    SYSINFO_ITEM_BUILD_TIME,       // 构建时间
    SYSINFO_ITEM_MAC,              // 机器码
    SYSINFO_ITEM_TOTAL_WRITRE,     // 总写入量
    SYSINFO_ITEM_ENUM_COUNT,
} sysinfo_item_enum_t;


extern const screen_t sys_info_screen;

#endif // SYS_INFO_SCREEN_H
