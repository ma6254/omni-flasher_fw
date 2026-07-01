#include <esp_log.h>
#include "settings_screen.h"
#include "main_menu_screen.h"
#include "buzzer.h"

static const char *TAG = "settings_screen";
static bool is_first_focus = true;

static settings_screen_item_cfg_t btn_cfg_list[SETTINGS_SCREEN_ENUM_COUNT] = {
    {"Brightness", .is_impled = true}, //
};

/*******************************************************************************
 * @brief 回调函数：按钮组
 * @param None
 * @return None
 ******************************************************************************/
static void btns_clicked_cb(lv_event_t *event)
{
    uint32_t index = (uint32_t)lv_event_get_user_data(event);

    // printf("[main_screen] btns_clicked_cb index:%d\n", index);
    ESP_LOGI(TAG, "btns_clicked_cb index:%" PRId32, index);

    switch (index)
    {
    case SETTINGS_SCREEN_BRIGHTNESS:
        break;
    default:
        return;
    }
}

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

static void settings_screen_screen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_FOCUSED)
    {
        // if (is_first_focus)
        // {
        //     is_first_focus = false;
        //     return;
        // }

        buzzer_set(100, 1, 0);
    }
}

/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 ******************************************************************************/
static void settings_screen_init(lv_obj_t *parent)
{
    is_first_focus = true;

    // lv_obj_clean(parent); // 清空屏幕

    lv_obj_set_style_bg_color(parent, lv_color_white(), 0); // 背景色

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
    lv_label_set_text(title_btn_label, "Settings");
    lv_obj_center(title_btn_label);
    // lv_obj_set_style_text_font(title_btn_label, &main_font, LV_PART_MAIN);

    lv_obj_t *cont_col = lv_obj_create(cont);
    lv_obj_set_flex_grow(cont_col, 1);
    lv_obj_set_size(cont_col, lv_pct(80), LV_SIZE_CONTENT);
    lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);

    lv_obj_set_style_width(cont_col, 20, LV_PART_SCROLLBAR);

    if (lv_group_get_default())
    {
        lv_group_delete(lv_group_get_default());
    }

    lv_group_t *group = lv_group_create();
    lv_group_set_editing(group, false);
    lv_group_remove_all_objs(group);
    lv_group_set_wrap(group, true);

    lv_group_set_default(group);
    lv_indev_set_group(screen_get_indev(), group);

    // 关闭按钮
    lv_obj_t *close_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(close_btn, close_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(close_btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
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

    for (uint32_t i = 0; i < SETTINGS_SCREEN_ENUM_COUNT; i++)
    {
        settings_screen_item_cfg_t *item_cfg = &btn_cfg_list[i];

        lv_obj_t *btn = lv_btn_create(cont_col);
        lv_obj_set_size(btn, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_align(btn, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
        lv_obj_add_event_cb(btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
        lv_group_add_obj(group, btn);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%s", item_cfg->name);
        lv_obj_center(label);
        // lv_obj_set_style_text_font(label, &main_font, LV_PART_MAIN);

        if (item_cfg->is_impled == false)
        {
            lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_group_focus_obj(lv_obj_get_child(cont_col, 0));
    
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void settings_screen_deinit(void)
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
 ******************************************************************************/
static void settings_screen_loop(void)
{
}

/*******************************************************************************
 * @brief 界面描述结构体
 ******************************************************************************/
const screen_t settings_screen = {
    .name = "settings_screen",
    .init_cb = settings_screen_init,
    .deinit_cb = settings_screen_deinit,
    .loop_cb = settings_screen_loop,
};
