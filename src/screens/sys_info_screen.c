#include <esp_mac.h>
#include "sys_info_screen.h"
#include "main_menu_screen.h"
#include "utils.h"
#include "buzzer.h"

static void item_update_cb(sys_info_screen_item_t *item);

static sys_info_screen_item_t item_list[SYSINFO_ITEM_ENUM_COUNT] = {
    {
        .name = "Product Name",
        .neet_updated = false,
        .update_cb = item_update_cb,
        .update_cb_user_data = (void *)SYSINFO_ITEM_PRODUCT_NAME,

    },
    {
        .name = "Build Time",
        .neet_updated = false,
        .update_cb = item_update_cb,
        .update_cb_user_data = (void *)SYSINFO_ITEM_BUILD_TIME,

    },
    {
        .name = "MAC",
        .neet_updated = false,
        .update_cb = item_update_cb,
        .update_cb_user_data = (void *)SYSINFO_ITEM_MAC,

    },
    {
        .name = "Total Write",
        .neet_updated = false,
        .update_cb = item_update_cb,
        .update_cb_user_data = (void *)SYSINFO_ITEM_TOTAL_WRITRE,

    },
};

/*******************************************************************************
 * @brief 回调函数：关闭按钮
 * @param None
 * @return None
 ******************************************************************************/
static void close_btn_clicked_cb(lv_event_t *event)
{
    buzzer_set(100, 1, 0);
    screen_set_load_anim(LV_SCREEN_LOAD_ANIM_OUT_RIGHT);
    screen_switch(&main_menu_screen);
}

/*******************************************************************************
 * @brief 回调函数：项目内容刷新
 * @param None
 * @return None
 ******************************************************************************/
static void item_update_cb(sys_info_screen_item_t *item)
{
    if (item == NULL)
        return;

    uint32_t item_i = (uint32_t)(item->update_cb_user_data);

    switch (item_i)
    {
    case SYSINFO_ITEM_PRODUCT_NAME:
    {
        lv_label_set_text_fmt(item->label, "%s", "OmniFlasher");
    }
    break;
    case SYSINFO_ITEM_BUILD_TIME:
    {
        lv_label_set_text_fmt(item->label, "%04d-%02d-%02d %s", __YEAR__, __MONTH__, __DAY__, __TIME__);
    }
    break;
    case SYSINFO_ITEM_MAC:
    {
        uint8_t mac[6] = {0};
        esp_read_mac(mac, ESP_MAC_BASE);
        lv_label_set_text_fmt(item->label, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    break;
    case SYSINFO_ITEM_TOTAL_WRITRE:
    {
        uint8_t mac[6] = {0};
        esp_read_mac(mac, ESP_MAC_BASE);
        lv_label_set_text_fmt(item->label, "Count:%d Size:%dMB", 114514, 114514);
    }
    break;
    }
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void sys_info_screen_init(lv_obj_t *parent)
{
    // lv_obj_clean(lv_scr_act()); // 清空屏幕

    lv_obj_set_style_bg_color(parent, lv_color_black(), 0); // 背景色

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

    lv_obj_t *title_btn = lv_btn_create(cont);
    lv_obj_set_flex_grow(title_btn, 0);
    lv_obj_set_size(title_btn, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(title_btn, lv_color_make(0x00, 0x00, 0xFF), LV_PART_MAIN);
    lv_obj_set_style_radius(title_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(title_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(title_btn, 0, LV_PART_MAIN);
    lv_obj_clear_flag(title_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *title_btn_label = lv_label_create(title_btn);
    lv_label_set_text(title_btn_label, "System Info");
    lv_obj_center(title_btn_label);
    // lv_obj_set_style_text_font(title_btn_label, &main_font, LV_PART_MAIN);

    lv_obj_t *cont_col = lv_obj_create(cont);
    lv_obj_set_flex_grow(cont_col, 1);
    lv_obj_set_size(cont_col, lv_pct(95), LV_SIZE_CONTENT);
    lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);

    lv_obj_set_style_width(cont_col, 20, LV_PART_SCROLLBAR);

    lv_obj_set_style_pad_gap(cont_col, 20, LV_PART_MAIN);

    lv_group_t *group = lv_group_create();
    lv_group_set_editing(group, false);
    lv_group_remove_all_objs(group);
    lv_group_set_wrap(group, true);

    lv_group_set_default(group);
    lv_indev_set_group(screen_get_indev(), group);

        // 关闭按钮
    lv_obj_t *close_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(close_btn, close_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_event_cb(close_btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(close_btn, lv_color_make(0xFF, 0x00, 000), LV_PART_MAIN);
    lv_obj_set_style_radius(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(close_btn, LV_ALIGN_TOP_LEFT);
    lv_obj_t *close_btn_label = lv_label_create(close_btn);
    lv_label_set_text(close_btn_label, "<");
    lv_obj_center(close_btn_label);
    // lv_obj_set_style_text_font(close_btn_label, &main_font, LV_PART_MAIN);
    lv_group_add_obj(group, close_btn);

    lv_obj_t *tmp_first_focus_obj = NULL;

        for (uint32_t i = 0; i < SYSINFO_ITEM_ENUM_COUNT; i++)
    {
        sys_info_screen_item_t *curr_item = &item_list[i];

        lv_obj_t *item_cont_col = lv_obj_create(cont_col);
        lv_obj_set_flex_grow(item_cont_col, 0);
        lv_obj_set_size(item_cont_col, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_align(item_cont_col, LV_ALIGN_CENTER);
        lv_obj_set_flex_flow(item_cont_col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(item_cont_col, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_SPACE_AROUND);
        lv_obj_set_style_bg_opa(item_cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(item_cont_col, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(item_cont_col, 0, LV_PART_MAIN);

        lv_obj_t *item_title_btn = lv_btn_create(item_cont_col);
        lv_group_add_obj(group, item_title_btn);
        lv_obj_set_size(item_title_btn, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(item_title_btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(item_title_btn, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(item_title_btn, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(item_title_btn, 0, LV_PART_MAIN);
        // lv_obj_add_event_cb(item_title_btn, close_btn_clicked_cb, LV_EVENT_CLICKED,
        //                     (void *)index);
        lv_obj_set_align(item_title_btn, LV_ALIGN_TOP_LEFT);
        lv_obj_t *item_title_btn_label = lv_label_create(item_title_btn);
        lv_label_set_text_static(item_title_btn_label, curr_item->name);
        lv_label_set_long_mode(item_title_btn_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_align(item_title_btn_label, LV_ALIGN_BOTTOM_LEFT);
        lv_obj_set_size(item_title_btn_label, lv_pct(100), LV_SIZE_CONTENT);
        // lv_obj_set_style_text_font(item_title_btn_label, &theory_question_font, LV_PART_MAIN);

        lv_obj_t *item_val_btn = lv_btn_create(item_cont_col);
        lv_group_add_obj(group, item_val_btn);
        lv_obj_set_size(item_val_btn, lv_pct(100), LV_SIZE_CONTENT);
        // lv_obj_add_event_cb(item_val_btn, close_btn_clicked_cb, LV_EVENT_CLICKED,
        //                     (void *)index);
        lv_obj_set_align(item_val_btn, LV_ALIGN_TOP_LEFT);
        lv_obj_t *item_val_btn_label = lv_label_create(item_val_btn);
        lv_label_set_long_mode(item_val_btn_label, LV_LABEL_LONG_WRAP);
        lv_label_set_text_fmt(item_val_btn_label, "{{%s}}", curr_item->name);
        lv_obj_set_align(item_val_btn_label, LV_ALIGN_TOP_LEFT);
        lv_obj_set_size(item_val_btn_label, lv_pct(100), LV_SIZE_CONTENT);
        // lv_obj_set_style_text_font(item_val_btn_label, &theory_question_font, LV_PART_MAIN);

        if (i == 0)
        {
            tmp_first_focus_obj = item_title_btn;
        }

        curr_item->label = item_val_btn_label;

        // 存在刷新函数吗？
        if ((curr_item->update_cb != NULL) && (curr_item->label != NULL))
        {
            curr_item->update_cb(curr_item);
        }
    }

    lv_group_focus_obj(tmp_first_focus_obj);
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void sys_info_screen_deinit(void)
{
}

/*******************************************************************************
 * @brief 界面循环
 * @param None
 * @return None
 ******************************************************************************/
static void sys_info_screen_loop(void)
{
}

/*******************************************************************************
 * @brief 界面描述结构体
 ******************************************************************************/
const screen_t sys_info_screen = {
    .name = "sys_info_screen",
    .init_cb = sys_info_screen_init,
    .deinit_cb = sys_info_screen_deinit,
    .loop_cb = sys_info_screen_loop,
};
