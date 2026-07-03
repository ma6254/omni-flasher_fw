#include <string.h>
#include <esp_log.h>
#include "main_menu_screen.h"
#include "settings_screen.h"
#include "sys_info_screen.h"
#include "buzzer.h"
#include "assets.h"

static const char *TAG = "main_menu_screen";
static bool is_first_focus = true;
static int32_t prev_focused_index = -1;
static main_menu_screen_item_handle_t item_handle_list[MAIN_MENU_SCREEN_ENUM_COUNT] = {0};

static const main_menu_screen_item_cfg_t btn_cfg_list[MAIN_MENU_SCREEN_ENUM_COUNT] = {
    {
        "Flash IC",
        .is_impled = true,
        .icon = &flash_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 72,
    },
    {
        "SD Card",
        .is_impled = true,
        .icon = &sd_card_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 72,
    },
    {
        "Serial",
        .is_impled = true,
        .icon = &serial_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 128,
    },
    {
        "CMSIS-DAP",
        .is_impled = true,
        .icon = &jtag_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 92,
    },
    {
        "Settings",
        .is_impled = true,
        .icon = &settings_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 92,
    },
    {
        "SystemInfo",
        .is_impled = true,
        .icon = &info_icon,
        .icon_scale_a = 32,
        .icon_scale_b = 92,
    },
};

#define main_menu_screen_focus_buzz() (buzzer_set(50, 1, 0))

// /*******************************************************************************
//  * @brief 回调函数：按钮组
//  * @param None
//  * @return None
//  ******************************************************************************/
// static void btns_clicked_cb(lv_event_t *event)
// {
//     uint32_t index = (uint32_t)lv_event_get_user_data(event);

//     // printf("[main_screen] btns_clicked_cb index:%d\n", index);
//     ESP_LOGI(TAG, "btns_clicked_cb index:%" PRId32, index);

//     switch (index)
//     {
//     case MAIN_MENU_SCREEN_SERIAL:
//         break;
//     case MAIN_MENU_SCREEN_SETTINGS:
//         screen_switch(&settings_screen);
//         break;
//     case MAIN_MENU_SCREEN_SYS_INFO:
//         screen_switch(&sys_info_screen);
//         break;
//     default:
//         return;
//     }
// }

void item_switch(uint32_t index)
{
    switch (index)
    {
    case MAIN_MENU_SCREEN_SERIAL:
        break;
    case MAIN_MENU_SCREEN_SETTINGS:
        screen_set_load_anim(LV_SCR_LOAD_ANIM_OVER_LEFT);
        screen_switch(&settings_screen);
        break;
    case MAIN_MENU_SCREEN_SYS_INFO:
        screen_set_load_anim(LV_SCR_LOAD_ANIM_OVER_LEFT);
        screen_switch(&sys_info_screen);
        break;
    default:
        return;
    }
}

void lv_anim_completed_cb(lv_anim_t *anim)
{
    uint32_t index = (uint32_t)lv_anim_get_user_data(anim);

    ESP_LOGI(TAG, "lv_anim_completed_cb index:%" PRId32, index);
}

static void ui_set_item_focus(uint32_t index, bool anim_on)
{
    lv_group_t *g = lv_group_get_default();

    if (g == NULL)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: group is NULL");
        return;
    }

    if (index >= MAIN_MENU_SCREEN_ENUM_COUNT)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: index out of range");
        return;
    }

    if (prev_focused_index >= 0)
    {

        lv_obj_t *prev_item = lv_group_get_obj_by_index(g, prev_focused_index);
        main_menu_screen_item_handle_t *prev_item_handle = &item_handle_list[prev_focused_index];

        // 为当前项创建增加高度的动画
        lv_anim_t focus_height_anim;
        lv_anim_init(&focus_height_anim);
        lv_anim_set_var(&focus_height_anim, prev_item);
        lv_anim_set_exec_cb(&focus_height_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_values(&focus_height_anim, lv_obj_get_height(prev_item), 30);
        lv_anim_set_path_cb(&focus_height_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&focus_height_anim, 500);
        lv_anim_start(&focus_height_anim);

        lv_anim_t focus_width_anim;
        lv_anim_init(&focus_width_anim);
        lv_anim_set_var(&focus_width_anim, prev_item);
        lv_anim_set_exec_cb(&focus_width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        lv_anim_set_values(&focus_width_anim, lv_obj_get_width(prev_item), 200);
        lv_anim_set_path_cb(&focus_width_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&focus_width_anim, 500);
        lv_anim_start(&focus_width_anim);

        if (prev_item_handle->icon_img != NULL)
        {
            lv_anim_t focus_icon_anim;
            lv_anim_init(&focus_icon_anim);
            lv_anim_set_var(&focus_icon_anim, prev_item_handle->icon_img);
            lv_anim_set_exec_cb(&focus_icon_anim, (lv_anim_exec_xcb_t)lv_image_set_scale);
            lv_anim_set_values(&focus_icon_anim, lv_image_get_scale(prev_item_handle->icon_img), prev_item_handle->cfg->icon_scale_a);
            lv_anim_set_path_cb(&focus_icon_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&focus_icon_anim, 500);
            lv_anim_start(&focus_icon_anim);
        }

        // lv_obj_set_style_pad_all(prev_item_handle->btn, 0, LV_PART_MAIN);
    }

    {
        lv_obj_t *now_item = lv_group_get_obj_by_index(g, index);
        main_menu_screen_item_handle_t *now_item_handle = &item_handle_list[index];

        // 为当前项创建增加高度的动画
        lv_anim_t defocus_height_anim;
        lv_anim_init(&defocus_height_anim);
        lv_anim_set_var(&defocus_height_anim, now_item);
        lv_anim_set_exec_cb(&defocus_height_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_values(&defocus_height_anim, lv_obj_get_height(now_item), 100);
        lv_anim_set_path_cb(&defocus_height_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&defocus_height_anim, 500);
        lv_anim_start(&defocus_height_anim);

        lv_anim_t defocus_width_anim;
        lv_anim_init(&defocus_width_anim);
        lv_anim_set_var(&defocus_width_anim, now_item);
        lv_anim_set_exec_cb(&defocus_width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        lv_anim_set_values(&defocus_width_anim, lv_obj_get_width(now_item), 270);
        lv_anim_set_path_cb(&defocus_width_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&defocus_width_anim, 500);
        lv_anim_start(&defocus_width_anim);

        if (now_item_handle->icon_img != NULL)
        {
            lv_anim_t defocus_icon_anim;
            lv_anim_init(&defocus_icon_anim);
            lv_anim_set_var(&defocus_icon_anim, now_item_handle->icon_img);
            lv_anim_set_exec_cb(&defocus_icon_anim, (lv_anim_exec_xcb_t)lv_image_set_scale);
            lv_anim_set_values(&defocus_icon_anim, lv_image_get_scale(now_item_handle->icon_img), now_item_handle->cfg->icon_scale_b);
            lv_anim_set_path_cb(&defocus_icon_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&defocus_icon_anim, 500);
            lv_anim_start(&defocus_icon_anim);
        }

        // lv_obj_set_style_pad_all(now_item_handle->btn, 5, LV_PART_MAIN);
    }

    prev_focused_index = index;
}

static void main_menu_screen_screen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    uint32_t index = (uint32_t)lv_event_get_user_data(e);
    main_menu_screen_item_handle_t *item_handle = &item_handle_list[index];

    if (code == LV_EVENT_FOCUSED)
    {
        // // 为当前项创建增加高度的动画
        // lv_anim_t a;
        // lv_anim_init(&a);
        // lv_anim_set_var(&a, target);
        // lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
        // lv_anim_set_values(&a, lv_obj_get_height(target), 100);
        // lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        // lv_anim_set_time(&a, 500);
        // lv_anim_start(&a);

        // lv_anim_t width_anim;
        // lv_anim_init(&width_anim);
        // lv_anim_set_var(&width_anim, target);
        // lv_anim_set_exec_cb(&width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        // lv_anim_set_values(&width_anim, lv_obj_get_width(target), 270);
        // lv_anim_set_path_cb(&width_anim, lv_anim_path_overshoot);
        // lv_anim_set_time(&width_anim, 500);
        // lv_anim_start(&width_anim);

        ui_set_item_focus(index, true);

        // if (is_first_focus)
        // {
        //     is_first_focus = false;
        //     return;
        // }

        main_menu_screen_focus_buzz();
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        // // 为当前项创建增加高度的动画
        // lv_anim_t a;
        // lv_anim_init(&a);
        // lv_anim_set_var(&a, target);
        // lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
        // lv_anim_set_values(&a, lv_obj_get_height(target), 30);
        // lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        // lv_anim_set_time(&a, 500);
        // lv_anim_start(&a);

        // lv_anim_t width_anim;
        // lv_anim_init(&width_anim);
        // lv_anim_set_var(&width_anim, target);
        // lv_anim_set_exec_cb(&width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        // lv_anim_set_values(&width_anim, lv_obj_get_width(target), 200);
        // lv_anim_set_path_cb(&width_anim, lv_anim_path_overshoot);
        // lv_anim_set_time(&width_anim, 500);
        // lv_anim_start(&width_anim);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        main_menu_screen_focus_buzz();

        // lv_obj_set_parent(target, lv_scr_act()); // 将当前对象移动到屏幕的顶层
        // lv_obj_set_align(target, LV_ALIGN_CENTER);

        // lv_anim_t a;
        // lv_anim_init(&a);
        // lv_anim_set_var(&a, target);
        // lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
        // lv_anim_set_values(&a, lv_obj_get_height(target), 120);
        // lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        // lv_anim_set_user_data(&a, lv_event_get_user_data(e));
        // lv_anim_set_completed_cb(&a, lv_anim_completed_cb);
        // lv_anim_set_time(&a, 500);
        // lv_anim_start(&a);

        // lv_anim_t width_anim;
        // lv_anim_init(&width_anim);
        // lv_anim_set_var(&width_anim, target);
        // lv_anim_set_exec_cb(&width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        // lv_anim_set_values(&width_anim, lv_obj_get_width(target), 300);
        // lv_anim_set_path_cb(&width_anim, lv_anim_path_overshoot);
        // lv_anim_set_time(&width_anim, 500);
        // lv_anim_set_user_data(&width_anim, lv_event_get_user_data(e));
        // lv_anim_set_completed_cb(&width_anim, lv_anim_completed_cb);
        // lv_anim_start(&width_anim);

        item_switch(index);
    }
}

static void indev_group_init(void)
{
    lv_group_t *group = lv_group_create();
    lv_group_set_editing(group, false);
    lv_group_set_wrap(group, false);
    lv_group_remove_all_objs(group);

    lv_group_set_default(group);
    lv_indev_set_group(screen_get_indev(), group);

    for (uint32_t i = 0; i < MAIN_MENU_SCREEN_ENUM_COUNT; i++)
    {
        main_menu_screen_item_handle_t *item_handle = &item_handle_list[i];

        lv_group_add_obj(group, item_handle->btn);
    }
}

static void indev_group_focus_init(void)
{
    screen_t *prev_screen = screen_get_prev();
    if (prev_screen == &settings_screen)
    {
        main_menu_screen_item_handle_t *item_handle = &item_handle_list[MAIN_MENU_SCREEN_SETTINGS];
        ESP_LOGI(TAG, "prev_screen: settings_screen");

        lv_group_focus_obj(item_handle->btn);
    }
    else if (prev_screen == &sys_info_screen)
    {
        main_menu_screen_item_handle_t *item_handle = &item_handle_list[MAIN_MENU_SCREEN_SYS_INFO];
        ESP_LOGI(TAG, "prev_screen: sys_info_screen");

        lv_group_focus_obj(item_handle->btn);
    }
    else
    {
        main_menu_screen_item_handle_t *item_handle = &item_handle_list[MAIN_MENU_SCREEN_FLASH];
        ESP_LOGI(TAG, "prev_screen: none");

        ui_set_item_focus(MAIN_MENU_SCREEN_FLASH, true);
        lv_group_focus_obj(item_handle->btn);
        //    lv_obj_scroll_to_y(cont_col, lv_obj_get_y(item_handle->btn), LV_ANIM_OFF);
    }
}

static void parent_loaded_event_handler(lv_event_t *e)
{

    indev_group_init();
    indev_group_focus_init();
}


/*******************************************************************************
 * @brief 回调函数：
 * @param None
 * @return None
 * @ref lv_anim_exec_xcb_t
 ******************************************************************************/
static void btn_anime_exec_cb(lv_obj_t *obj, int32_t val)
{
    // lv_obj_align(obj, LV_ALIGN_CENTER, val, 0);
    lv_obj_set_width(obj, val);
}
    
/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 ******************************************************************************/
static void main_menu_screen_init(lv_obj_t *parent)
{
    prev_focused_index = -1;
    is_first_focus = true;

    ESP_LOGI(TAG, "initnal begin");
    
    lv_obj_set_style_bg_color(parent, lv_color_white(), 0); // 背景色
    
    bool is_sub_screen_back = false;
    screen_t *prev_screen = screen_get_prev();
    if ((prev_screen == &settings_screen) || (prev_screen == &sys_info_screen))
    {
        ESP_LOGI(TAG, "is sub_screen_back");
        is_sub_screen_back = true;
    }

    if (is_sub_screen_back == false)
    {
        lv_obj_add_event_cb(parent, parent_loaded_event_handler, LV_EVENT_SCREEN_LOADED, NULL);
    }

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
    // lv_obj_set_style_pad_gap(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(cont, 0, LV_PART_MAIN);

    // lv_obj_t *title_btn = lv_btn_create(cont);
    // lv_obj_set_flex_grow(title_btn, 0);
    // lv_obj_set_size(title_btn, LV_PCT(100), LV_SIZE_CONTENT);
    // lv_obj_set_style_bg_color(title_btn, lv_color_make(0x00, 0x00, 0xFF), LV_PART_MAIN);
    // lv_obj_set_style_radius(title_btn, 0, LV_PART_MAIN);
    // lv_obj_set_style_shadow_width(title_btn, 0, LV_PART_MAIN);
    // lv_obj_set_style_border_width(title_btn, 0, LV_PART_MAIN);
    // lv_obj_clear_flag(title_btn, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_t *title_btn_label = lv_label_create(title_btn);
    // lv_label_set_text(title_btn_label, "main_menu");
    // lv_obj_center(title_btn_label);
    // // lv_obj_set_style_text_font(title_btn_label, &main_font, LV_PART_MAIN);

    lv_obj_t *cont_col = lv_obj_create(cont);
    lv_obj_set_flex_grow(cont_col, 1);
    lv_obj_set_size(cont_col, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);
    // lv_obj_set_style_pad_right(cont_col, 40, LV_PART_MAIN);
    // lv_obj_set_style_width(cont_col, 20, LV_PART_SCROLLBAR);
    lv_obj_set_scrollbar_mode(cont_col, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_y(cont_col, LV_SCROLL_SNAP_CENTER);
    lv_add_debug_border(cont_col);

    if (lv_group_get_default())
    {
        lv_group_delete(lv_group_get_default());
    }

    // lv_group_t *group = lv_group_create();
    // lv_group_set_editing(group, false);
    // lv_group_set_wrap(group, false);
    // lv_group_remove_all_objs(group);

    // lv_group_set_default(group);
    // lv_indev_set_group(screen_get_indev(), group);

    memset(item_handle_list, 0, sizeof(item_handle_list));

    for (uint32_t i = 0; i < MAIN_MENU_SCREEN_ENUM_COUNT; i++)
    {
        main_menu_screen_item_handle_t *item_handle = &item_handle_list[i];

        item_handle->cfg = &btn_cfg_list[i];

        lv_obj_t *btn = lv_btn_create(cont_col);
        item_handle->btn = btn;
        {
            lv_obj_set_size(btn, 200, 30);
            lv_obj_set_align(btn, LV_ALIGN_RIGHT_MID);
            // lv_obj_add_event_cb(btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);

            lv_obj_add_event_cb(btn, main_menu_screen_screen_event_handler, LV_EVENT_ALL, (void *)i);
            lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
            lv_obj_set_style_outline_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
            lv_add_debug_border(btn);
            // lv_group_add_obj(group, btn);

            lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
        }

        {
            // 为当前项创建增加高度的动画
            lv_anim_t anim;
            lv_anim_init(&anim);
            lv_anim_set_var(&anim, btn);
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
            lv_anim_set_values(&anim, 0, 200);
            lv_anim_set_path_cb(&anim, lv_anim_path_overshoot);
            lv_anim_set_time(&anim, 500);
            lv_anim_start(&anim);
        }

        lv_obj_t *item_cont = lv_obj_create(btn);
        lv_obj_set_size(item_cont, lv_pct(100), lv_pct(100));
        lv_obj_set_align(item_cont, LV_ALIGN_CENTER);
        lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
        lv_obj_set_style_bg_opa(item_cont, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(item_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(item_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(item_cont, 0, LV_PART_MAIN);
        // lv_obj_set_style_width(item_cont, 20, LV_PART_SCROLLBAR);
        lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
        lv_add_debug_border(item_cont);

        if (item_handle->cfg->icon != NULL)
        {
            lv_obj_t *icon = lv_img_create(item_cont);
            {
                lv_img_set_src(icon, item_handle->cfg->icon);
                lv_obj_set_size(icon, 80, 80);
                // lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);
                lv_image_set_scale(icon, item_handle->cfg->icon_scale_a);
                lv_obj_set_align(icon, LV_ALIGN_LEFT_MID);
                // lv_obj_set_style_pad_left(icon, 10, LV_PART_MAIN);
                lv_add_debug_border(icon);
            }
            item_handle->icon_img = icon;
        }

        lv_obj_t *label_cont = lv_obj_create(item_cont);
        lv_obj_set_size(label_cont, LV_SIZE_CONTENT, lv_pct(100));
        lv_obj_set_align(label_cont, LV_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(label_cont, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(label_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(label_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(label_cont, 0, LV_PART_MAIN);
        // lv_obj_set_style_width(label_cont, 20, LV_PART_SCROLLBAR);
        lv_obj_set_scrollbar_mode(label_cont, LV_SCROLLBAR_MODE_OFF);
        lv_add_debug_border(label_cont);

        lv_obj_t *label = lv_label_create(label_cont);
        {
            lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_fmt(label, "%s", item_handle->cfg->name);
            lv_obj_center(label);
            // lv_obj_set_style_text_font(label, &main_font, LV_PART_MAIN);
            lv_add_debug_border(label);
        }

        if (item_handle->cfg->icon != NULL)
        {
            lv_obj_set_flex_grow(item_handle->icon_img, 40);
            lv_obj_set_flex_grow(label_cont, 60);
        }

        if (item_handle->cfg->is_impled == false)
        {
            lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (is_sub_screen_back == true)
    {
        indev_group_init();
        indev_group_focus_init();
    }

    // lv_anim_t title_btn_anim;
    // lv_anim_init(&title_btn_anim);
    // lv_anim_set_var(&title_btn_anim, title_btn);
    // lv_anim_set_exec_cb(&title_btn_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
    // lv_anim_set_values(&title_btn_anim, 0, lv_obj_get_height(title_btn));
    // lv_anim_set_time(&title_btn_anim, 250);
    // lv_anim_start(&title_btn_anim);

   ESP_LOGI(TAG, "init success");
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void main_menu_screen_deinit(void)
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
static void main_menu_screen_loop(void)
{
}

/*******************************************************************************
 * @brief 界面描述结构体
 ******************************************************************************/
const screen_t main_menu_screen = {
    .name = "main_menu_screen",
    .init_cb = main_menu_screen_init,
    .deinit_cb = main_menu_screen_deinit,
    .loop_cb = main_menu_screen_loop,
};
