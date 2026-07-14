#include <esp_log.h>
#include "serial_screen.h"
#include "lv_exmenu.h"
#include "lv_i18n.h"
#include "assets.h"
#include "buzzer.h"
#include "main_menu_screen.h"

#define CLOSE_BTN_MINIMISE_SIZE 32 // 取消按键的最小化尺寸，单位：像素
#define CLOSE_BTN_MAXIMISE_SIZE 50 // 取消按键的最大化尺寸，单位：像素

static const char *TAG = "serial_screen";
static lv_exmenu_handle_t exmenu_handle;
static lv_obj_t *close_btn;
static lv_obj_t *cont_col;
static lv_group_t *man_group;

static const lv_exmenu_item_cfg_t item_cfg_list[SERIAL_SCREEN_ENUM_COUNT] = {
    {
        .name = "USB_VCOM",
        .icon = &serial_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 128,
    },
    {
        .name = "USB_VCOM_X2",
        .icon = &serial_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 128,
    },
    {
        .name = "USB_RS485",
        .icon = &serial_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 128,
    },
    {
        .name = "RECORDER",
        .icon = &serial_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 128,
    },
};

/*******************************************************************************
 * @brief 回调函数：关闭按钮
 * @param None
 * @return None
 * @ref lv_exmenu_item_get_name_cb_t
 ******************************************************************************/
static esp_err_t exmenu_get_name_cb(lv_exmenu_handle_t *handle, uint32_t index, char *out_buf, size_t buf_size)
{
    bool is_found = false;
    const char *str = NULL;

    if (index == SERIAL_SCREEN_USB_VCOM)
    {
        str = _("settings_screen.usb_vcom");
        is_found = true;
    }
    else if (index == SERIAL_SCREEN_USB_VCOM_X2)
    {
        str = _("settings_screen.usb_vcom_x2");
        is_found = true;
    }
    else if (index == SERIAL_SCREEN_USB_RS485)
    {
        str = _("settings_screen.usb_rs485");
        is_found = true;
    }
    else if (index == SERIAL_SCREEN_USB_RS485)
    {
        str = _("settings_screen.usb_rs485");
        is_found = true;
    }
    else if (index == SERIAL_SCREEN_RECORDER)
    {
        str = _("settings_screen.recorder");
        is_found = true;
    }

    if (is_found == false)
    {
        return ESP_ERR_NOT_FOUND;
    }

    size_t str_len = strlen(str);

    size_t real_len = (str_len < buf_size - 1) ? str_len : (buf_size - 1);
    memcpy(out_buf, str, real_len);
    out_buf[real_len] = '\0';

    return ESP_OK;
}

/*******************************************************************************
 * @brief 回调函数：关闭按钮点击事件
 * @param None
 * @return None
 ******************************************************************************/
static void close_btn_clicked_cb(lv_event_t *event)
{
    buzzer_set(50, 1, 0);
    screen_set_load_anim(LV_SCREEN_LOAD_ANIM_OUT_RIGHT);
    screen_switch(&main_menu_screen);
}

/*******************************************************************************
 * @brief 回调函数：关闭按钮聚焦事件
 * @param None
 * @return None
 ******************************************************************************/
static void close_btn_focus_event_handler(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_FOCUSED)
    {
        lv_exmenu_set_focus(&exmenu_handle, -1, true);
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
    }

}

/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 * @ref screen_init_cb_t
 ******************************************************************************/
static void serial_screen_init(lv_obj_t *parent)
{
    esp_err_t err;

    close_btn = NULL;
    cont_col = NULL;

    ESP_LOGI(TAG, "initnal begin");

    if (lv_group_get_default())
    {
        lv_group_delete(lv_group_get_default());
    }
    lv_group_set_default(NULL);
    lv_indev_set_group(screen_get_indev(), NULL);

    man_group = lv_group_create();
    lv_group_set_editing(man_group, false);
    lv_group_set_wrap(man_group, false);
    lv_group_remove_all_objs(man_group);
    
    memset(&exmenu_handle, 0, sizeof(exmenu_handle));

    set_key_map_mode(KEY_MAP_NONE);

    // 背景色
    lv_obj_set_style_bg_color(parent, lv_color_make(0x2D, 0x43, 0x56), 0);

    // 顶级容器
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_set_align(cont, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(cont, 0, LV_PART_MAIN);
    lv_add_debug_border(cont);

    // 标题栏
    lv_obj_t *title_btn = lv_btn_create(cont);
    lv_obj_set_flex_grow(title_btn, 0);
    lv_obj_set_size(title_btn, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(title_btn, lv_color_make(0x00, 0x00, 0xFF), LV_PART_MAIN);
    lv_obj_set_style_radius(title_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(title_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(title_btn, 0, LV_PART_MAIN);
    lv_obj_clear_flag(title_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_add_debug_border(title_btn);
    lv_obj_t *title_btn_label = lv_label_create(title_btn);
    lv_label_set_text(title_btn_label, _("main_menu_screen.serial"));
    lv_obj_center(title_btn_label);
    lv_add_debug_border(title_btn_label);
    lv_obj_set_style_text_font(title_btn_label, &main_menu_screen_item_font, LV_PART_MAIN);

    // 关闭按钮
    close_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(close_btn, close_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(close_btn, close_btn_focus_event_handler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(close_btn, close_btn_focus_event_handler, LV_EVENT_DEFOCUSED, NULL);
    lv_obj_set_style_bg_color(close_btn, lv_color_make(0xFF, 0x00, 000), LV_PART_MAIN);
    lv_obj_set_style_radius(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(close_btn, LV_ALIGN_TOP_LEFT);
    lv_obj_set_size(close_btn, CLOSE_BTN_MINIMISE_SIZE, CLOSE_BTN_MINIMISE_SIZE);
    // lv_obj_set_style_outline_width(close_btn, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_outline_opa(close_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_t *close_btn_label = lv_label_create(close_btn);
    lv_label_set_text(close_btn_label, "<");
    lv_obj_center(close_btn_label);
    // lv_group_add_obj(main_group, close_btn);
    // lv_obj_set_style_text_font(close_btn_label, &main_font, LV_PART_MAIN);
    lv_add_debug_border(close_btn);
    // lv_group_add_obj(man_group, close_btn);

    // 窗口内容器
    cont_col = lv_obj_create(cont);
    lv_obj_set_flex_grow(cont_col, 1);
    lv_obj_set_size(cont_col, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cont_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont_col, 10, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(cont_col, LV_SCROLLBAR_MODE_OFF);
    lv_add_debug_border(cont_col);

    lv_obj_set_style_width(cont_col, 20, LV_PART_SCROLLBAR);

    lv_group_add_obj(man_group, close_btn);

    lv_exmenu_cfg_t exmenu_cfg = {
        .item_cfg_list = item_cfg_list,
        .item_count = SERIAL_SCREEN_ENUM_COUNT,
        .item_font = &serial_screen_item_font,
        .group = man_group,
        .parent = cont_col,
        .get_name_cb = exmenu_get_name_cb,
    };
    err = lv_exmenu_init(&exmenu_handle, &exmenu_cfg);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "lv_exmenu_init failed, err=%s", esp_err_to_name(err));
        return;
    }

    man_group = exmenu_handle.group;
    if (man_group == NULL)
    {
        ESP_LOGE(TAG, "menu group is NULL");
        return;
    }

    lv_group_set_default(man_group);
    lv_indev_set_group(screen_get_indev(), man_group);
    lv_group_focus_obj(exmenu_handle.item_handle_list[0].btn);
    set_key_map_mode(KEY_MAP_NAV);

    ESP_LOGI(TAG, "initnal success");
}

/*******************************************************************************
 * @brief 界面去初始化
 * @param None
 * @return None
 * @ref screen_deinit_cb_t
 ******************************************************************************/
static void serial_screen_deinit(void)
{
    if (lv_group_get_default())
    {
        lv_group_delete(lv_group_get_default());
    }
}

/*******************************************************************************
 * @brief 界面循环
 * @param None
 * @return None
 * @ref screen_loop_cb_t
 ******************************************************************************/
static void serial_screen_loop(void)
{
}

/*******************************************************************************
 * @brief 界面描述结构体
 ******************************************************************************/
const screen_t serial_screen = {
    .name = "serial_screen",
    .init_cb = serial_screen_init,
    .deinit_cb = serial_screen_deinit,
    .loop_cb = serial_screen_loop,
};
