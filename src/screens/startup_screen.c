#include <esp_log.h>
#include "startup_screen.h"
#include "main_menu_screen.h"
#include "assets.h"
#include "proj_config.h"

static const char *TAG = "startup_screen";
static lv_timer_t *startup_timer = NULL;
static lv_timer_t *delay_open_timer = NULL;
// static uint8_t color_index = 0;
static lv_obj_t *startup_gif_img = NULL;

/*******************************************************************************
 * @brief 回调函数：开机定时器
 * @param None
 * @return None
 ******************************************************************************/
static void startup_timer_cb(lv_timer_t *timer)
{
    lv_timer_pause(startup_timer);

    screen_set_load_anim(LV_SCREEN_LOAD_ANIM_FADE_OUT);
    screen_switch(&main_menu_screen);
}

/*******************************************************************************
 * @brief 回调函数：延时开屏定时器
 * @param None
 * @return None
 ******************************************************************************/
static void delay_open_timer_cb(lv_timer_t *timer)
{
    lv_timer_pause(delay_open_timer);

    set_lcd_bl_on();
}


/*******************************************************************************
 * @brief 回调函数：GIF播放完成事件处理
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_gif_event_handler(lv_event_t *e)
{
    lv_obj_t *gif_img = lv_event_get_target(e);

    
    ESP_LOGI(TAG, "gif play finished, restart");
    lv_gif_restart(gif_img);
}

/*******************************************************************************
 * @brief 回调函数：顶部遮罩动画执行回调
 * @param None
 * @return None
 * @ref lv_anim_exec_xcb_t
 ******************************************************************************/
static void top_mask_anime_exec_cb(lv_obj_t *obj, int32_t val)
{
    lv_obj_set_style_bg_opa(obj, val, LV_PART_MAIN);
}

/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_init(lv_obj_t *parent)
{
    // prev_gif_loaded = false;

    ESP_LOGI(TAG, "initial begin");

    lv_obj_set_style_bg_color(parent, lv_color_white(), 0); // 背景色
    // lv_obj_set_style_bg_color(parent, lv_color_black(), 0); // 背景色

    lv_obj_t *cont_col = lv_obj_create(parent);
    {
        lv_obj_set_flex_grow(cont_col, 1);
        lv_obj_set_size(cont_col, lv_pct(100), lv_pct(100));
        lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
        lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
        lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);
        lv_obj_clear_flag(cont_col, LV_OBJ_FLAG_SCROLLABLE);
        // lv_obj_set_style_pad_all(cont_col, 0, LV_PART_MAIN);
        lv_add_debug_border(cont_col);
    }

    startup_gif_img = lv_gif_create(cont_col);
    {
        lv_obj_align(startup_gif_img, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(startup_gif_img, startup_screen_gif_event_handler, LV_EVENT_READY, NULL);
        // lv_gif_set_src(startup_gif, &startup_gif_img_dsc);

#if CFG_USE_STARTUP_GIF
        lv_gif_set_src(startup_gif_img, &startup_gif);
#endif // CFG_USE_STARTUP_GIF

        // lv_gif_set_loop_count(startup_gif_img, 0);
        // lv_gif_pause(startup_gif_img);
        // lv_gif_restart(startup_gif_img);
        // lv_image_set_scale(startup_gif_img, 305);
        lv_gif_set_loop_count(startup_gif_img, 1);
        lv_add_debug_border(startup_gif_img);

        if(lv_gif_is_loaded(startup_gif_img)) {
            ESP_LOGI(TAG, "gif loaded successfully");
        }
        else {
            ESP_LOGW(TAG, "gif loaded failed");
        }
    }
    
    lv_obj_t *top_mask = lv_obj_create(parent);
    {
        lv_obj_set_flex_grow(top_mask, 1);
        lv_obj_set_size(top_mask, lv_pct(100), lv_pct(100));
        lv_obj_set_align(top_mask, LV_ALIGN_CENTER);
        lv_obj_set_flex_flow(top_mask, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(top_mask, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
        lv_obj_set_style_bg_opa(top_mask, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(top_mask, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(top_mask, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(top_mask, 0, LV_PART_MAIN);
        lv_obj_clear_flag(top_mask, LV_OBJ_FLAG_SCROLLABLE);
        // lv_obj_set_style_pad_all(top_mask, 0, LV_PART_MAIN);
        lv_add_debug_border(top_mask);
    }

    lv_anim_t top_mask_anim;
    {
        lv_anim_init(&top_mask_anim);
        lv_anim_set_var(&top_mask_anim, top_mask);
        lv_anim_set_exec_cb(&top_mask_anim, (lv_anim_exec_xcb_t)top_mask_anime_exec_cb);
        lv_anim_set_values(&top_mask_anim, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_path_cb(&top_mask_anim, lv_anim_path_ease_out);
        lv_anim_set_user_data(&top_mask_anim, NULL);
        lv_anim_set_time(&top_mask_anim, 3000);
        lv_anim_start(&top_mask_anim);
    }

    startup_timer = lv_timer_create(startup_timer_cb, 10000, NULL);
    lv_timer_pause(startup_timer);
    lv_timer_reset(startup_timer);
    lv_timer_resume(startup_timer);


    delay_open_timer = lv_timer_create(delay_open_timer_cb, 100, NULL);
    lv_timer_pause(delay_open_timer);
    lv_timer_reset(delay_open_timer);
    lv_timer_resume(delay_open_timer);

    ESP_LOGI(TAG, "initial successfully");
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_deinit(void)
{
    if (startup_timer)
    {
        lv_timer_pause(startup_timer);
        lv_timer_del(startup_timer);
        startup_timer = NULL;
    }

    ESP_LOGI(TAG, "deinitialized");
}

/*******************************************************************************
 * @brief 界面循环
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_loop(void)
{
}

/*******************************************************************************
 * @brief 界面描述结构体
 ******************************************************************************/
const screen_t startup_screen = {
    .name = "startup_screen",
    .init_cb = startup_screen_init,
    .deinit_cb = startup_screen_deinit,
    .loop_cb = startup_screen_loop,
};
