#include <string.h>
#include <esp_log.h>
#include "lv_exmenu.h"
#include "buzzer.h"

static const char *TAG = "lv_exmenu";

/*******************************************************************************
 * @brief 回调函数：菜单项按钮图标尺寸动画执行回调
 * @param obj 图标对象
 * @param val 焦点索引
 * @return None
 * @ref lv_anim_exec_xcb_t
 ******************************************************************************/
static void lv_exmenu_item_btn_icon_size_anime_exec_cb(lv_obj_t *obj, int32_t val)
{
    lv_obj_set_width(obj, val);
    lv_obj_set_height(obj, val);
}

/*******************************************************************************
 * @brief 回调函数：菜单项按钮高度动画执行回调
 * @param obj 按钮对象
 * @param val 高度值
 * @return None
 * @ref lv_anim_exec_xcb_t
 ******************************************************************************/
static void lv_exmenu_item_btn_height_anime_exec_cb(lv_obj_t *obj, int32_t val)
{
    if (obj == NULL)
    {
        return;
    }

    lv_obj_set_height(obj, val);
}

static void lv_exmenu_parent_scroll_y_anime_exec_cb(lv_obj_t *obj, int32_t val)
{
    if (obj == NULL)
    {
        return;
    }

    lv_obj_scroll_to_y(obj, val, LV_ANIM_OFF);
}

static bool lv_exmenu_is_menu_item_obj(const lv_exmenu_handle_t *handle, const lv_obj_t *obj)
{
    if ((handle == NULL) || (obj == NULL))
    {
        return false;
    }

    for (uint32_t i = 0; i < handle->cfg.item_count; i++)
    {
        if (handle->item_handle_list[i].btn == obj)
        {
            return true;
        }
    }

    return false;
}

static void lv_exmenu_item_scroll_to_center_anim(lv_exmenu_handle_t *handle, lv_obj_t *now_item, bool anim_on)
{
    if ((handle == NULL) || (now_item == NULL))
    {
        return;
    }

    lv_obj_t *cont = lv_obj_get_parent(now_item);
    if (cont == NULL)
    {
        return;
    }

    lv_anim_delete(cont, (lv_anim_exec_xcb_t)lv_exmenu_parent_scroll_y_anime_exec_cb);

    int32_t scroll_from = lv_obj_get_scroll_y(cont);

    lv_obj_t *prev_item = NULL;
    int32_t prev_height_org = 0;
    if ((handle->prev_focused_index >= 0) && (handle->prev_focused_index < handle->cfg.item_count))
    {
        prev_item = handle->item_handle_list[handle->prev_focused_index].btn;
        if (prev_item != NULL)
        {
            prev_height_org = lv_obj_get_height(prev_item);
        }
    }

    int32_t now_height_org = lv_obj_get_height(now_item);

    if ((prev_item != NULL) && (prev_item != now_item))
    {
        lv_obj_set_height(prev_item, 45);
    }
    lv_obj_set_height(now_item, 100);
    lv_obj_update_layout(cont);

    lv_obj_scroll_to_view(now_item, LV_ANIM_OFF);
    int32_t scroll_to = lv_obj_get_scroll_y(cont);

    if ((prev_item != NULL) && (prev_item != now_item))
    {
        lv_obj_set_height(prev_item, prev_height_org);
    }
    lv_obj_set_height(now_item, now_height_org);
    lv_obj_update_layout(cont);
    lv_obj_scroll_to_y(cont, scroll_from, LV_ANIM_OFF);

    if ((anim_on == false) || (scroll_from == scroll_to))
    {
        lv_obj_scroll_to_y(cont, scroll_to, LV_ANIM_OFF);
        return;
    }

    lv_anim_t focus_scroll_anim;
    lv_anim_init(&focus_scroll_anim);
    lv_anim_set_var(&focus_scroll_anim, cont);
    lv_anim_set_exec_cb(&focus_scroll_anim, (lv_anim_exec_xcb_t)lv_exmenu_parent_scroll_y_anime_exec_cb);
    lv_anim_set_values(&focus_scroll_anim, scroll_from, scroll_to);
    lv_anim_set_path_cb(&focus_scroll_anim, lv_anim_path_ease_in_out);
    lv_anim_set_time(&focus_scroll_anim, 500);
    lv_anim_start(&focus_scroll_anim);
}

static void lv_exmenu_item_ui_set_focus_anim(lv_exmenu_handle_t *handle, int32_t index, bool anim_on)
{
    lv_group_t *g = lv_group_get_default();

    if (g == NULL)
    {
        ESP_LOGE(TAG, "ui_set_item_focus: group is NULL");
        return;
    }

    if ((index < -1) || (index >= (int32_t)handle->cfg.item_count) || (index >= LV_EXMENU_ITEM_COUNT_MAX))
    {
        ESP_LOGE(TAG, "ui_set_item_focus: index out of range index=%" PRId32 ", item_count=%" PRIu32, index,
                 handle->cfg.item_count);
        return;
    }

    // ESP_LOGE(TAG, "ui_set_item_focus: prev_index:%" PRId32 " > now_index:%" PRId32, handle->prev_focused_index, index);

    if ((handle->prev_focused_index >= 0) && (handle->prev_focused_index < handle->cfg.item_count))
    {
        lv_exmenu_item_handle_t *prev_item_handle = &handle->item_handle_list[handle->prev_focused_index];
        const lv_exmenu_item_cfg_t *prev_item_cfg = &handle->cfg.item_cfg_list[handle->prev_focused_index];
        lv_obj_t *prev_item = prev_item_handle->btn;

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
  
        
        if (prev_item_handle->icon_img != NULL)
        {
            lv_anim_t defocus_icon_size_anim;
            lv_anim_init(&defocus_icon_size_anim);
            lv_anim_set_var(&defocus_icon_size_anim, prev_item_handle->icon_img);
            lv_anim_set_exec_cb(&defocus_icon_size_anim, (lv_anim_exec_xcb_t)lv_exmenu_item_btn_icon_size_anime_exec_cb);
            lv_anim_set_values(&defocus_icon_size_anim, lv_obj_get_width(prev_item_handle->icon_img), 45);
            lv_anim_set_path_cb(&defocus_icon_size_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&defocus_icon_size_anim, 500);
            lv_anim_start(&defocus_icon_size_anim);

            lv_anim_t defocus_icon_anim;
            lv_anim_init(&defocus_icon_anim);
            lv_anim_set_var(&defocus_icon_anim, prev_item_handle->icon_img);
            lv_anim_set_exec_cb(&defocus_icon_anim, (lv_anim_exec_xcb_t)lv_image_set_scale);
            lv_anim_set_values(&defocus_icon_anim, lv_image_get_scale(prev_item_handle->icon_img),
                               prev_item_cfg->icon_scale_a);
            lv_anim_set_path_cb(&defocus_icon_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&defocus_icon_anim, 500);
            lv_anim_start(&defocus_icon_anim);
        }
    }

    if ((index >= 0) && (index < handle->cfg.item_count))
    {
        lv_exmenu_item_handle_t *now_item_handle = &handle->item_handle_list[index];
        const lv_exmenu_item_cfg_t *now_item_cfg = &handle->cfg.item_cfg_list[index];
        lv_obj_t *now_item = now_item_handle->btn;
        lv_exmenu_item_scroll_to_center_anim(handle, now_item, anim_on);

        // 为当前项创建增加高度的动画
        lv_anim_t focus_height_anim;
        lv_anim_init(&focus_height_anim);
        lv_anim_set_var(&focus_height_anim, now_item);
        lv_anim_set_exec_cb(&focus_height_anim, (lv_anim_exec_xcb_t)lv_exmenu_item_btn_height_anime_exec_cb);
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

        if (now_item_handle->icon_img != NULL)
        {
            lv_anim_t focus_icon_size_anim;
            lv_anim_init(&focus_icon_size_anim);
            lv_anim_set_var(&focus_icon_size_anim, now_item_handle->icon_img);
            lv_anim_set_exec_cb(&focus_icon_size_anim, (lv_anim_exec_xcb_t)lv_exmenu_item_btn_icon_size_anime_exec_cb);
            lv_anim_set_values(&focus_icon_size_anim, lv_obj_get_width(now_item_handle->icon_img), 84);
            lv_anim_set_path_cb(&focus_icon_size_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&focus_icon_size_anim, 500);
            lv_anim_start(&focus_icon_size_anim);

            lv_anim_t focus_icon_anim;
            lv_anim_init(&focus_icon_anim);
            lv_anim_set_var(&focus_icon_anim, now_item_handle->icon_img);
            lv_anim_set_exec_cb(&focus_icon_anim, (lv_anim_exec_xcb_t)lv_image_set_scale);
            lv_anim_set_values(&focus_icon_anim, lv_image_get_scale(now_item_handle->icon_img),
                               now_item_cfg->icon_scale_b);
            lv_anim_set_path_cb(&focus_icon_anim, lv_anim_path_overshoot);
            lv_anim_set_time(&focus_icon_anim, 500);
            lv_anim_start(&focus_icon_anim);
        }
    }

    handle->prev_focused_index = index;
}

/*******************************************************************************
 * @brief 回调函数：父容器加载完成事件处理函数
 * @param e 事件对象 
 * @return None
 ******************************************************************************/
static void lv_exmenu_parent_loaded_event_handler(lv_event_t *e)
{
    lv_exmenu_handle_t *handle = (lv_exmenu_handle_t *)lv_event_get_user_data(e);
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "lv_exmenu_parent_loaded_event_handler user_data is NULL");
        return;
    }

    if (handle->cfg.item_count > 0)
    {
        ESP_LOGI(TAG, "initial focus first item");
        lv_group_focus_obj(handle->item_handle_list[0].btn);
    }
}

/*******************************************************************************
 * @brief handle 菜单项按钮事件处理函数
 * @param e 事件对象 
 * @return None
 ******************************************************************************/
static void lv_exmenu_item_btn_event_handler(lv_event_t *e)
{
    // ESP_LOGI(TAG, "lv_exmenu_item_btn_event_handler");

    uint32_t code = lv_event_get_code(e);
    lv_exmenu_item_event_t *item_event = lv_event_get_user_data(e);
    if (item_event == NULL)
    {
        ESP_LOGE(TAG, "lv_exmenu_item_btn_event_handler user_data is NULL");
        return;
    }

    if(code == LV_EVENT_CLICKED)
    {
        // ESP_LOGI(TAG, "lv_exmenu_item_btn_event_handler LV_EVENT_CLICKED index=%" PRIu32, item_event->index);
    }
    else if (code == LV_EVENT_FOCUSED)
    {
        // ESP_LOGI(TAG, "lv_exmenu_item_btn_event_handler LV_EVENT_FOCUSED index=%" PRIu32, item_event->index);
        buzzer_set(50, 1, 0);
        // item_event->handle->pending_focused_index = item_event->index;

        lv_exmenu_item_ui_set_focus_anim(item_event->handle, item_event->index, true);

        // item_event->handle->pending_focused_index = item_event->index;
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        lv_group_t *group = item_event->handle->group;
        lv_obj_t *focused_obj = NULL;
        if (group != NULL)
        {
            focused_obj = lv_group_get_focused(group);
        }

        if (lv_exmenu_is_menu_item_obj(item_event->handle, focused_obj) == false)
        {
            lv_exmenu_item_ui_set_focus_anim(item_event->handle, -1, true);
        }
    }
}

/*******************************************************************************
 * @brief 初始化菜单
 * @param handle 菜单句柄
 * @param cfg 菜单配置
 * @return ESP_OK 成功，其他值失败
 ******************************************************************************/
esp_err_t lv_exmenu_init(lv_exmenu_handle_t *handle, const lv_exmenu_cfg_t *cfg)
{
    if (handle == NULL || cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (cfg->parent == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (cfg->item_count > LV_EXMENU_ITEM_COUNT_MAX)
    {
        ESP_LOGE(TAG, "item_count too large: %" PRIu32 " > %d", cfg->item_count, LV_EXMENU_ITEM_COUNT_MAX);
        return ESP_ERR_INVALID_SIZE;
    }

    memset(handle, 0, sizeof(lv_exmenu_handle_t));
    handle->cfg = *cfg;
    handle->prev_focused_index = -1;
    handle->pending_focused_index = -1;
    // char title_buf[128];

    lv_obj_t *parent = handle->cfg.parent;

    if (handle->cfg.group == NULL)
    {
        ESP_LOGW(TAG, "group is NULL, creating a new group");

        lv_group_t *g = lv_group_create();
        handle->group = g;
        lv_group_set_editing(g, false);
        lv_group_set_wrap(g, false);
        lv_group_remove_all_objs(g);
    }
    else
    {
        ESP_LOGI(TAG, "group is not NULL, using the provided group");
        handle->group = handle->cfg.group;
    }

    lv_group_t *g = handle->group;

    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_CENTER);
    lv_obj_add_event_cb(parent, lv_exmenu_parent_loaded_event_handler, LV_EVENT_SCREEN_LOADED, (void *)handle);

    for (uint32_t i = 0; i < handle->cfg.item_count; i++)
    {
        const lv_exmenu_item_cfg_t *item_cfg = &handle->cfg.item_cfg_list[i];
        lv_exmenu_item_handle_t *item_handle = &handle->item_handle_list[i];
        lv_exmenu_item_event_t *item_btn_event = &handle->item_btn_event_list[i];
        
        // 按钮
        lv_obj_t *btn = lv_btn_create(parent);
        lv_group_add_obj(g, btn);
        item_handle->btn = btn;
        {
            memset(item_btn_event, 0, sizeof(lv_exmenu_item_event_t));
            item_btn_event->handle = handle;
            item_btn_event->index = i;

            lv_obj_set_size(btn, 200, 45);
            lv_obj_set_align(btn, LV_ALIGN_CENTER);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            // lv_obj_add_event_cb(btn, btns_clicked_cb, LV_EVENT_CLICKED, (void *)i);
            lv_obj_add_event_cb(btn, lv_exmenu_item_btn_event_handler, LV_EVENT_ALL, (void *)item_btn_event);
            lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
            lv_obj_set_style_outline_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
            // lv_group_add_obj(main_group, item_handle_list[i].btn);
            lv_obj_set_style_bg_color(btn, lv_color_make(255, 102, 0), LV_PART_MAIN);
            lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
            lv_obj_set_style_shadow_color(btn, lv_color_make(0, 0, 0), LV_PART_MAIN);
            lv_add_debug_border(btn);
            lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
        }

        // 按钮内容器
        lv_obj_t *item_cont = lv_obj_create(btn);
        {
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
            // lv_obj_set_scroll_snap_y(cont_col, LV_SCROLL_SNAP_CENTER);
            lv_add_debug_border(item_cont);
        }

        if (item_cfg->icon != NULL)
        {
            lv_obj_t *icon_image = lv_img_create(item_cont);
            item_handle->icon_img = icon_image;
            {
                lv_obj_set_flex_grow(icon_image, 0);
                lv_img_set_src(icon_image, item_cfg->icon);
                lv_obj_set_size(icon_image, 45, 45);
                // lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);
                lv_image_set_scale(icon_image, item_cfg->icon_scale_a);
                lv_obj_set_align(icon_image, LV_ALIGN_LEFT_MID);
                // lv_obj_set_style_pad_left(icon, 10, LV_PART_MAIN);
                lv_add_debug_border(icon_image);
            }
        }

        // 按钮标题容器
        lv_obj_t *title_label_cont = lv_obj_create(item_cont);
        {
            lv_obj_set_flex_grow(title_label_cont, 1);
            lv_obj_set_size(title_label_cont, LV_SIZE_CONTENT, lv_pct(100));
            lv_obj_set_align(title_label_cont, LV_ALIGN_CENTER);
            lv_obj_set_style_bg_opa(title_label_cont, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_width(title_label_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(title_label_cont, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(title_label_cont, 0, LV_PART_MAIN);
            // lv_obj_set_style_width(title_label_cont, 20, LV_PART_SCROLLBAR);
            lv_obj_set_scrollbar_mode(title_label_cont, LV_SCROLLBAR_MODE_OFF);
            lv_add_debug_border(title_label_cont);
        }

        // 按钮标题标签
        lv_obj_t *label = lv_label_create(title_label_cont);
        {
            lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            // memset(title_buf, 0, sizeof(title_buf));

            if(handle->cfg.get_name_cb)
            {
                char name_buf[128];
                memset(name_buf, 0, sizeof(name_buf));
                esp_err_t err = handle->cfg.get_name_cb(handle, i, name_buf, sizeof(name_buf));
                if (err == ESP_OK)
                {
                    lv_label_set_text_fmt(label, "%s", name_buf);
                }
                else
                {
                    lv_label_set_text_fmt(label, "%s", item_cfg->name);
                }
            }
            else
            {
                lv_label_set_text_fmt(label, "%s", item_cfg->name);
            }

            // lv_obj_center(label);
            lv_obj_set_align(label, LV_ALIGN_CENTER);
            lv_obj_set_style_text_color(label, lv_color_make(255, 255, 255), LV_PART_MAIN);
            lv_obj_set_style_text_font(label, handle->cfg.item_font, LV_PART_MAIN);
            lv_add_debug_border(label);
        }
    }

    lv_obj_update_layout(parent);
    int32_t edge_pad = lv_obj_get_height(parent) / 2 - 100 / 2;
    if (edge_pad < 0)
    {
        edge_pad = 0;
    }
    lv_obj_set_style_pad_top(parent, edge_pad, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(parent, edge_pad, LV_PART_MAIN);
    lv_obj_update_layout(parent);

    ESP_LOGI(TAG, "initial success");
    return ESP_OK;
}

/*******************************************************************************
 * @brief 设置菜单焦点
 * @param handle 菜单句柄
 * @param index 焦点索引
 * @return ESP_OK 成功，其他值失败
 ******************************************************************************/
esp_err_t lv_exmenu_set_focus(lv_exmenu_handle_t *handle, int32_t index, bool anim_on)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (index >= (int32_t)(handle->cfg.item_count))
    {
        ESP_LOGE(TAG, "lv_exmenu_set_focus: index out of range index=%" PRId32 ", item_count=%" PRIu32, index, handle->cfg.item_count);
        return ESP_ERR_INVALID_ARG;
    }

    lv_group_t *g = handle->group;
    if (g == NULL)
    {
        ESP_LOGE(TAG, "lv_exmenu_set_focus: group is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= 0)
    {
        lv_obj_t *obj = handle->item_handle_list[index].btn;
        if (obj == NULL)
        {
            ESP_LOGE(TAG, "lv_exmenu_set_focus: obj is NULL");
            return ESP_ERR_INVALID_STATE;
        }
        lv_group_focus_obj(obj);
    }
    else
    {
        lv_exmenu_item_ui_set_focus_anim(handle, -1, anim_on);
    }

    handle->prev_focused_index = index;
    return ESP_OK;
}
