#include <string.h>
#include <esp_log.h>
#include "settings_screen.h"
#include "main_menu_screen.h"
#include "buzzer.h"
#include "assets.h"
#include "config.h"

static const char *TAG = "settings_screen";
static bool is_first_focus = true;
static bool is_editing_exit = false;
static int32_t prev_focused_index = -1;
static uint8_t brightness_level;
static lv_group_t *main_group = NULL;
// static lv_group_t *brightness_group = NULL;

// static void brightness_indev_group_init(void);
static void settings_screen_screen_event_handler(lv_event_t *e);

static const settings_screen_item_cfg_t btn_cfg_list[SETTINGS_SCREEN_ENUM_COUNT] = {
    {
        .name = "Language",
        .is_impled = true,
    },
    {
        .name = "Brightness",
        .is_impled = true,
    },
    {
        .name = "Buzzer",
        .is_impled = true,
    },
    {
        .name = "Info",
        .is_impled = true,
    },
    {
        .name = "About",
        .is_impled = true,
    },
};

static settings_screen_item_handle_t item_handle_list[SETTINGS_SCREEN_ENUM_COUNT];

static lv_obj_t *brightness_obj_list[SCREEN_BRIGHTNESS_COUNT];
static lv_obj_t *brightness_cursor_obj_list[2];

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

static void ui_set_brightness_level(uint8_t level)
{
    if (level > SCREEN_BRIGHTNESS_MAX)
    {
        ESP_LOGE(TAG, "ui_set_brightness_level: level out of range");
        return;
    }

    ESP_LOGI(TAG, "ui_set_brightness_level: level:%" PRIu8, level);

    for (uint32_t i = 0; i < SCREEN_BRIGHTNESS_COUNT; i++)
    {
        lv_obj_t *curr_obj = brightness_obj_list[i];

        if (i < level)
        {
            lv_obj_set_style_opa(curr_obj, LV_OPA_COVER, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_opa(curr_obj, LV_OPA_50, LV_PART_MAIN);
        }
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

static void ui_set_item_focus(int32_t index, bool anim_on)
{
    lv_group_t *g = lv_group_get_default();

    if (g == NULL)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: group is NULL");
        return;
    }

    if (index >= SETTINGS_SCREEN_ENUM_COUNT)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: index out of range");
        return;
    }

    if(prev_focused_index == index)
    {
        return;
    }

    if ((prev_focused_index >= 0) && (prev_focused_index < SETTINGS_SCREEN_ENUM_COUNT))
    {
        lv_obj_t *prev_item = item_handle_list[prev_focused_index].btn;

        if (prev_item == NULL)
        {
            ESP_LOGW(TAG, "ui_set_item_focus: prev item is NULL");
            prev_focused_index = -1;
        }
        else
        {

            // 为当前项创建增加高度的动画
            lv_anim_t defocus_height_anim;
            lv_anim_init(&defocus_height_anim);
            lv_anim_set_var(&defocus_height_anim, prev_item);
            lv_anim_set_exec_cb(&defocus_height_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
            lv_anim_set_values(&defocus_height_anim, lv_obj_get_height(prev_item), 45);
            lv_anim_set_path_cb(&defocus_height_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&defocus_height_anim, 500);
            lv_anim_start(&defocus_height_anim);

            lv_anim_t defocus_width_anim;
            lv_anim_init(&defocus_width_anim);
            lv_anim_set_var(&defocus_width_anim, prev_item);
            lv_anim_set_exec_cb(&defocus_width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
            lv_anim_set_values(&defocus_width_anim, lv_obj_get_width(prev_item), 200);
            lv_anim_set_path_cb(&defocus_width_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&defocus_width_anim, 500);
            lv_anim_start(&defocus_width_anim);

            // lv_obj_set_style_pad_all(prev_item_handle->btn, 0, LV_PART_MAIN);
        }
    }

    if (index < 0)
    {
        prev_focused_index = -1;
        return;
    }

    lv_obj_t *now_item = item_handle_list[index].btn;
    if (now_item == NULL)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: current item is NULL");
        return;
    }

    if (index >= 0)
    {
        // 为当前项创建增加高度的动画
        lv_anim_t focus_height_anim;
        lv_anim_init(&focus_height_anim);
        lv_anim_set_var(&focus_height_anim, now_item);
        lv_anim_set_exec_cb(&focus_height_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_values(&focus_height_anim, lv_obj_get_height(now_item), 100);
        lv_anim_set_path_cb(&focus_height_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&focus_height_anim, 500);
        lv_anim_start(&focus_height_anim);

        lv_anim_t focus_width_anim;
        lv_anim_init(&focus_width_anim);
        lv_anim_set_var(&focus_width_anim, now_item);
        lv_anim_set_exec_cb(&focus_width_anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
        lv_anim_set_values(&focus_width_anim, lv_obj_get_width(now_item), 270);
        lv_anim_set_path_cb(&focus_width_anim, lv_anim_path_overshoot);
        lv_anim_set_time(&focus_width_anim, 500);
        lv_anim_start(&focus_width_anim);

        // lv_obj_set_style_pad_all(now_item_handle->btn, 5, LV_PART_MAIN);
    }

    prev_focused_index = index;
}

/*******************************************************************************
 * @brief 回调函数：关闭按钮事件处理
 * @param None
 * @return None
 ******************************************************************************/
static void close_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_FOCUSED)
    {
        ui_set_item_focus(-1, true);
    }
}

/*******************************************************************************
 * @brief 回调函数：亮度设置按钮事件处理
 * @param None
 * @return None
 ******************************************************************************/
static void brightness_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t index = (uint32_t)lv_event_get_user_data(e);

    settings_screen_item_handle_t *item_handle = &item_handle_list[index];

    if (code == LV_EVENT_KEY)
    {
        uint32_t key = lv_indev_get_key(lv_indev_get_act());

        ESP_LOGI(TAG, "key event key:%" PRIu32, key);

        if (key == LV_KEY_UP)
        {
            if (brightness_level < SCREEN_BRIGHTNESS_MAX)
            {
                brightness_level++;
                ui_set_brightness_level(brightness_level);
                set_lcd_bl_brightness(brightness_level);
            }
        }
        else if (key == LV_KEY_DOWN)
        {
            if (brightness_level > SCREEN_BRIGHTNESS_MIN)
            {
                brightness_level--;
                ui_set_brightness_level(brightness_level);
                set_lcd_bl_brightness(brightness_level);
            }
        }
        else if (key == LV_KEY_ENTER)
        {
            lv_obj_add_flag(brightness_cursor_obj_list[0], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(brightness_cursor_obj_list[1], LV_OBJ_FLAG_HIDDEN);

            lv_group_set_editing(main_group, true);
            set_key_map_mode(KEY_MAP_NAV);

            lv_obj_remove_event_cb(item_handle->btn, brightness_btn_event_handler);
            lv_obj_add_event_cb(item_handle->btn, settings_screen_screen_event_handler, LV_EVENT_ALL, (void *)index);

            g_config_data.brightness = brightness_level;
            config_save();

            is_editing_exit = true;
        }
    }
}

static void settings_screen_screen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t index = (uint32_t)lv_event_get_user_data(e);

    lv_group_t *group = lv_group_get_default();

    if (code == LV_EVENT_CLICKED)
    {
        if (is_editing_exit)
        {
            is_editing_exit = false;
            return;
        }

        buzzer_set(100, 1, 0);

        ESP_LOGI(TAG, "LV_EVENT_CLICKED");

        if (index == SETTINGS_SCREEN_BRIGHTNESS)
        {
            settings_screen_item_handle_t *item_handle = &item_handle_list[index];

            ESP_LOGI(TAG, "enter brightness edit mode");

                lv_obj_clear_flag(brightness_cursor_obj_list[0], LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(brightness_cursor_obj_list[1], LV_OBJ_FLAG_HIDDEN);

                lv_obj_remove_event_cb(item_handle->btn, settings_screen_screen_event_handler);
                lv_obj_add_event_cb(item_handle->btn, brightness_btn_event_handler, LV_EVENT_ALL, (void *)index);

                // lv_group_set_default(brightness_group);
                // lv_indev_set_group(screen_get_indev(), brightness_group);
                // lv_group_set_editing(brightness_group, true);
                // lv_group_focus_obj(item_handle->btn);
                lv_group_set_editing(main_group, true);
                set_key_map_mode(KEY_MAP_DIR_UD);
        }
    }
    else if (code == LV_EVENT_PRESSED)
    {
        ESP_LOGI(TAG, "pressed");
    }
    else if (code == LV_EVENT_LONG_PRESSED)
    {
        ESP_LOGI(TAG, "long pressed");
    }
    else if (code == LV_EVENT_FOCUSED)
    {
        if (index >= SETTINGS_SCREEN_ENUM_COUNT)
        {
            ESP_LOGW(TAG, "LV_EVENT_FOCUSED invalid index: %" PRIu32, index);
            return;
        }

        // if (is_first_focus)
        // {
        //     is_first_focus = false;
        //     return;
        // }

        ui_set_item_focus(index, true);
        buzzer_set(100, 1, 0);
    }
}

// static void brightness_indev_group_init(void)
// {

//     brightness_group = lv_group_create();
//     lv_group_set_editing(brightness_group, false);
//     lv_group_set_wrap(brightness_group, false);
//     lv_group_remove_all_objs(brightness_group);

//     // lv_group_set_default(brightness_group);
//     // lv_indev_set_group(screen_get_indev(), brightness_group);

//     settings_screen_item_handle_t *item_handle = &item_handle_list[SETTINGS_SCREEN_BRIGHTNESS];

//     lv_group_add_obj(brightness_group, item_handle->btn);
// }

static void indev_group_focus_init(void)
{
    lv_group_focus_obj(item_handle_list[0].btn);
    ui_set_item_focus(SETTINGS_SCREEN_LANGUAGE, true);
}

static void parent_loaded_event_handler(lv_event_t *e)
{
    indev_group_focus_init();
}

/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 ******************************************************************************/
static void settings_screen_init(lv_obj_t *parent)
{
    is_first_focus = true;
    prev_focused_index = -1;
    brightness_level = g_config_data.brightness;
    main_group = NULL;
    // brightness_group = NULL;
    is_editing_exit = false;

    memset(item_handle_list, 0, sizeof(item_handle_list));
    memset(brightness_obj_list, 0, sizeof(brightness_obj_list));
    memset(brightness_cursor_obj_list, 0, sizeof(brightness_cursor_obj_list));

    if (lv_group_get_default())
    {
        lv_group_delete(lv_group_get_default());
    }
    lv_group_set_default(NULL);

    main_group = lv_group_create();
    lv_group_set_editing(main_group, false);
    lv_group_set_wrap(main_group, false);
    lv_group_remove_all_objs(main_group);
    lv_indev_set_group(screen_get_indev(), main_group);
    set_key_map_mode(KEY_MAP_NAV);

    // lv_obj_clean(parent); // 清空屏幕

    // lv_obj_set_style_bg_color(parent, lv_color_white(), 0); // 背景色
    lv_obj_set_style_bg_color(parent, lv_color_make(0x2D, 0x43, 0x56), 0); // 背景色

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
    lv_label_set_text(title_btn_label, "Settings");
    lv_obj_center(title_btn_label);
    lv_add_debug_border(title_btn_label);
    lv_obj_set_style_text_font(title_btn_label, &lv_font_montserrat_20, LV_PART_MAIN);

    // 关闭按钮
    lv_obj_t *close_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(close_btn, close_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(close_btn, close_btn_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(close_btn, lv_color_make(0xFF, 0x00, 000), LV_PART_MAIN);
    lv_obj_set_style_radius(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(close_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(close_btn, LV_ALIGN_TOP_LEFT);
    lv_obj_set_size(close_btn, 32, 32);
    lv_obj_t *close_btn_label = lv_label_create(close_btn);
    lv_label_set_text(close_btn_label, "<");
    lv_obj_center(close_btn_label);
    // lv_obj_set_style_text_font(close_btn_label, &main_font, LV_PART_MAIN);
    lv_add_debug_border(close_btn);
    lv_group_add_obj(main_group, close_btn);

    // 窗口内容器
    lv_obj_t *cont_col = lv_obj_create(cont);
    lv_obj_set_flex_grow(cont_col, 1);
    lv_obj_set_size(cont_col, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_SPACE_AROUND);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cont_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont_col, 10, LV_PART_MAIN);
    lv_obj_set_scroll_snap_y(cont_col, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont_col, LV_SCROLLBAR_MODE_OFF);
    lv_add_debug_border(cont_col);

    lv_obj_set_style_width(cont_col, 20, LV_PART_SCROLLBAR);

    for (uint32_t i = 0; i < SETTINGS_SCREEN_ENUM_COUNT; i++)
    {
        settings_screen_item_handle_t *item_handle = &item_handle_list[i];
        item_handle->cfg = &btn_cfg_list[i];

        lv_obj_t *btn = lv_btn_create(cont_col);
        item_handle->btn = btn;
        {
            lv_obj_set_size(btn, 200, 30);
            lv_obj_set_align(btn, LV_ALIGN_CENTER);
            // lv_obj_add_event_cb(btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
            lv_obj_add_event_cb(btn, settings_screen_screen_event_handler, LV_EVENT_ALL, (void *)i);
            lv_add_debug_border(btn);
            lv_group_add_obj(main_group, btn);
        }

        lv_obj_t *item_cont = lv_obj_create(btn);
        item_handle->cont = item_cont;
        {
            lv_obj_set_size(item_cont, lv_pct(100), lv_pct(100));
            lv_obj_set_align(item_cont, LV_ALIGN_CENTER);
            lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
            lv_obj_set_style_bg_opa(item_cont, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_width(item_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(item_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(item_cont, 0, LV_PART_MAIN);
            // lv_obj_set_style_width(item_cont, 20, LV_PART_SCROLLBAR);
            lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
            lv_add_debug_border(item_cont);
        } 

        lv_obj_t *title_label_cont = lv_obj_create(item_cont);
        {
            lv_obj_set_flex_grow(title_label_cont, 0);
            lv_obj_set_size(title_label_cont, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_align(title_label_cont, LV_ALIGN_CENTER);
            lv_obj_set_style_bg_opa(title_label_cont, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_width(title_label_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(title_label_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(title_label_cont, 0, LV_PART_MAIN);
            // lv_obj_set_style_width(title_label_cont, 20, LV_PART_SCROLLBAR);
            lv_obj_set_scrollbar_mode(title_label_cont, LV_SCROLLBAR_MODE_OFF);
            lv_add_debug_border(title_label_cont);
        }

        lv_obj_t *label = lv_label_create(title_label_cont);
        item_handle->title_label = label;
        {
            lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_fmt(label, "%s", item_handle->cfg->name);
            // lv_obj_center(label);
            lv_obj_set_align(label, LV_ALIGN_LEFT_MID);
            lv_obj_set_style_text_color(label, lv_color_make(255, 255, 255), LV_PART_MAIN);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
            lv_add_debug_border(label);
        }

        // if (item_handle->cfg->is_impled == false)
        // {
        //     lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        // }
    }

    // 语言设置
    {
    }

    // 屏幕亮度设置
    {
        settings_screen_item_handle_t *item_handle = &item_handle_list[SETTINGS_SCREEN_BRIGHTNESS];
        ESP_ERROR_CHECK(item_handle != NULL ? ESP_OK : ESP_FAIL);
        
        lv_obj_t *brightness_parent_obj = item_handle->cont;
        ESP_ERROR_CHECK(brightness_parent_obj != NULL ? ESP_OK : ESP_FAIL);

        lv_obj_t *item_cont = lv_obj_create(brightness_parent_obj);
        {
            lv_obj_set_flex_grow(item_cont, 1);
            lv_obj_set_size(item_cont, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_align(item_cont, LV_ALIGN_CENTER);
            lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
            lv_obj_set_style_bg_opa(item_cont, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_width(item_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(item_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(item_cont, 0, LV_PART_MAIN);
            // lv_obj_set_style_width(item_cont, 20, LV_PART_SCROLLBAR);
            lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
            lv_add_debug_border(item_cont);
        }

        {
            lv_obj_t *left_label_btn = lv_btn_create(item_cont);
            {
                lv_obj_set_size(left_label_btn, LV_SIZE_CONTENT, lv_pct(100));
                lv_obj_set_align(left_label_btn, LV_ALIGN_CENTER);
                // lv_obj_add_event_cb(left_label_btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
                // lv_obj_add_event_cb(left_label_btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
                lv_obj_set_style_pad_all(left_label_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_radius(left_label_btn, 0, LV_PART_MAIN);
                lv_add_debug_border(left_label_btn);

                lv_obj_t *label = lv_label_create(left_label_btn);
                {
                    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_fmt(label, "<");
                    lv_obj_center(label);
                    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
                    lv_add_debug_border(label);
                }

                brightness_cursor_obj_list[0] = left_label_btn;
            }
        }

        for(uint32_t i = 0; i < SCREEN_BRIGHTNESS_COUNT; i++)
        {
            lv_obj_t *label_btn = lv_btn_create(item_cont);
            {
                lv_obj_set_size(label_btn, LV_SIZE_CONTENT, lv_pct(100));
                lv_obj_set_align(label_btn, LV_ALIGN_CENTER);
                // lv_obj_add_event_cb(label_btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
                // lv_obj_add_event_cb(label_btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
                lv_obj_set_style_pad_all(label_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_radius(label_btn, 0, LV_PART_MAIN);
                lv_add_debug_border(label_btn);
            }

            // lv_obj_t *label = lv_label_create(label_btn);
            // {
            //     lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            //     lv_label_set_text_fmt(label, "%" PRIu32, i + 1);
            //     lv_obj_center(label);
            //     lv_add_debug_border(label);
            // }
     
            lv_obj_t *icon = lv_image_create(label_btn);
            {
                lv_obj_set_size(icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_img_set_src(icon, &sun_icon);
                // lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);
                // lv_image_set_scale(icon, item_handle->cfg->icon_scale_a);
                lv_obj_set_align(icon, LV_ALIGN_LEFT_MID);
                // lv_obj_set_style_pad_left(icon, 10, LV_PART_MAIN);
                lv_add_debug_border(icon);
            }

            brightness_obj_list[i] = label_btn;
        }

        {
            lv_obj_t *left_label_btn = lv_btn_create(item_cont);
            {
                lv_obj_set_size(left_label_btn, LV_SIZE_CONTENT, lv_pct(100));
                lv_obj_set_align(left_label_btn, LV_ALIGN_CENTER);
                // lv_obj_add_event_cb(left_label_btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
                // lv_obj_add_event_cb(left_label_btn, settings_screen_screen_event_handler, LV_EVENT_ALL, NULL);
                lv_obj_set_style_pad_all(left_label_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_radius(left_label_btn, 0, LV_PART_MAIN);
                lv_add_debug_border(left_label_btn);
    
                lv_obj_t *label = lv_label_create(left_label_btn);
                {
                    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_fmt(label, ">");
                    lv_obj_center(label);
                    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
                    lv_add_debug_border(label);
                }

                brightness_cursor_obj_list[1] = left_label_btn;
            }
        }

        ui_set_brightness_level(brightness_level);
    }

    lv_obj_add_flag(brightness_cursor_obj_list[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag( brightness_cursor_obj_list[1], LV_OBJ_FLAG_HIDDEN);

    lv_group_set_default(main_group);
    lv_group_focus_obj(item_handle_list[0].btn);

    // brightness_indev_group_init();

    lv_obj_add_event_cb(parent, parent_loaded_event_handler, LV_EVENT_SCREEN_LOADED, NULL);

    ESP_LOGI(TAG, "initnal success");
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
