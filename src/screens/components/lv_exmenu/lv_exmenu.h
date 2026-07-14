#ifndef LV_MENU_WIDGET_H
#define LV_MENU_WIDGET_H

#include "lvgl_app.h"

#define LV_EXMENU_ITEM_COUNT_MAX 16 // 最大菜单项数量

typedef struct LV_EXMENU_HANDLE_T lv_exmenu_handle_t;
typedef struct LV_EXMENU_ITEM_EVENT_T lv_exmenu_item_event_t;

typedef esp_err_t (*lv_exmenu_item_get_name_cb_t)( //
    lv_exmenu_handle_t *handle,                    //
    uint32_t index,                                //
    char *out_buf, size_t buf_size);               //

typedef esp_err_t (*lv_exmenu_item_clicked_cb_t)( //
    lv_exmenu_handle_t *handle,                   //
    uint32_t index);                              //

typedef struct LV_EXMENU_ITEM_EVENT_T
{
    lv_exmenu_handle_t *handle;
    uint32_t index;
} lv_exmenu_item_event_t;

typedef struct
{
    char *name;
    bool is_impled; // 是否已实现
    const lv_image_dsc_t *icon;
    int32_t icon_scale_a;
    int32_t icon_scale_b;
} lv_exmenu_item_cfg_t;

typedef struct
{
    lv_obj_t *btn;
    lv_obj_t *icon_img;
} lv_exmenu_item_handle_t;

typedef struct
{
    const lv_exmenu_item_cfg_t *item_cfg_list;
    uint32_t item_count;
    const lv_font_t *item_font;
    lv_group_t *group;
    lv_obj_t *parent;
    lv_exmenu_item_get_name_cb_t get_name_cb;
    lv_exmenu_item_clicked_cb_t clicked_cb;
} lv_exmenu_cfg_t;

typedef struct LV_EXMENU_HANDLE_T
{
    lv_exmenu_cfg_t cfg;
    lv_group_t *group;
    lv_exmenu_item_handle_t item_handle_list[LV_EXMENU_ITEM_COUNT_MAX];
    lv_exmenu_item_event_t item_btn_event_list[LV_EXMENU_ITEM_COUNT_MAX];
    int32_t prev_focused_index;
    int32_t pending_focused_index;
} lv_exmenu_handle_t;

esp_err_t lv_exmenu_init(lv_exmenu_handle_t *handle, const lv_exmenu_cfg_t *cfg);
esp_err_t lv_exmenu_set_focus(lv_exmenu_handle_t *handle, int32_t index, bool anim_on);

#endif // LV_MENU_WIDGET_H
