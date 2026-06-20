#include "startup_screen.h"
#include "misc/lv_color.h"
#include <esp_log.h>

static const char *TAG = "startup_screen";
static lv_timer_t *startup_timer;
static uint8_t color_index = 0;

/*******************************************************************************
 * @brief 回调函数：开机定时器
 * @param None
 * @return None
 ******************************************************************************/
static void startup_timer_cb(lv_timer_t *timer)
{
    color_index++;
    color_index %= 2;

    if (color_index == 0)
    {
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), 0); // 背景色
    }
    else
    {
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0); // 背景色
    }
}

/*******************************************************************************
 * @brief 界面初始化
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_init(void)
{
    // prev_gif_loaded = false;

    ESP_LOGI(TAG, "initial begin");

    lv_obj_clean(lv_scr_act());                                   // 清空屏幕
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), 0); // 背景色

    lv_obj_t *cont_col = lv_obj_create(lv_scr_act());
    {
        lv_obj_set_flex_grow(cont_col, 1);
        lv_obj_set_size(cont_col, lv_pct(100), lv_pct(100));
        lv_obj_set_align(cont_col, LV_ALIGN_CENTER);
        lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
        lv_obj_set_style_bg_opa(cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(cont_col, 0, LV_PART_MAIN);
        // lv_obj_set_style_pad_all(cont_col, 0, LV_PART_MAIN);
    }

    lv_obj_t *a_btn = lv_btn_create(cont_col);
    lv_obj_t *a_btn_label = lv_label_create(a_btn);
    {
        lv_obj_set_flex_grow(a_btn, 1);
        lv_obj_set_size(a_btn, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(a_btn, lv_color_make(255, 0, 0), LV_PART_MAIN);
        lv_label_set_text(a_btn_label, "AAA");
        lv_obj_align(a_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(a_btn_label, lv_color_black(), LV_PART_MAIN);
    }

    lv_obj_t *b_btn = lv_btn_create(cont_col);
    lv_obj_t *b_btn_label = lv_label_create(b_btn);
    {
        lv_obj_set_flex_grow(b_btn, 1);
        lv_obj_set_size(b_btn, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(b_btn, lv_color_make(0, 255, 0), LV_PART_MAIN);
        lv_label_set_text(b_btn_label, "BBB");
        lv_obj_align(b_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(b_btn_label, lv_color_black(), LV_PART_MAIN);
    }

    lv_obj_t *c_btn = lv_btn_create(cont_col);
    lv_obj_t *c_btn_label = lv_label_create(c_btn);
    {
        lv_obj_set_flex_grow(c_btn, 1);
        lv_obj_set_size(c_btn, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(c_btn, lv_color_make(0, 0, 255), LV_PART_MAIN);

        lv_label_set_text(c_btn_label, "CCC");
        lv_obj_align(c_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(c_btn_label, lv_color_black(), LV_PART_MAIN);
    }

    startup_timer = lv_timer_create(startup_timer_cb, 500, NULL);
    lv_timer_reset(startup_timer);
    lv_timer_resume(startup_timer);

    ESP_LOGI(TAG, "initialized");
}

/*******************************************************************************
 * @brief 界面反初始化
 * @param None
 * @return None
 ******************************************************************************/
static void startup_screen_deinit(void)
{

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
