
#include <string.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <driver/ledc.h>

#include <lvgl.h>
#include <esp_lvgl_port.h>
#include "lvgl_app.h"
#include "config.h"

#include "board.h"
#include "misc/lv_color.h"
#include "key.h"
#include "portmacro.h"
#include "startup_screen.h"
#include "settings_screen.h"
#include "proj_config.h"


static const char *TAG = "lvgl_app";
static lv_display_t *gLvDisplay = NULL;
static esp_lcd_panel_handle_t PanelHandle = NULL;
static lv_indev_t *gLvIndev = NULL;
static screen_t *current_screen = NULL;
static screen_t *wait_switch_screen = NULL;
static screen_t *prev_screen = NULL;
static screen_t *switch_screen_user_data = NULL;
static key_map_t current_key_map_mode = KEY_MAP_NAV;

static volatile uint32_t screen_switch_flag = 0;
static portMUX_TYPE screen_switch_mux = portMUX_INITIALIZER_UNLOCKED;

uint32_t screen_encoder_sleep_flag = 0;
uint32_t screen_encoder_activity_flag = 0;

static int32_t screen_load_anim = -1; // lv_screen_load_anim_t

void set_lcd_bl_pwm_duty(uint32_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CHANNEL,
    duty)); ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE,
    LCD_BL_LEDC_CHANNEL));
}


void set_lcd_bl_brightness(uint32_t brightness)
{
    uint32_t duty = (uint32_t)((brightness * 1024) / SCREEN_BRIGHTNESS_MAX);
    set_lcd_bl_pwm_duty(duty);
}

void set_lcd_bl_on(void)
{
    set_lcd_bl_brightness(g_config_data.brightness);
}

void set_key_map_mode(key_map_t mode)
{
    current_key_map_mode = mode;
}

// static void lv_tick_task(void *arg)
// {
//     lv_tick_inc(1); // 通常以1ms为周期调用
// }

lv_indev_t *screen_get_indev(void)
{
    return gLvIndev;
}

void *screen_get_switch_user_data(void)
{
    return switch_screen_user_data;
}

/*******************************************************************************
 * @brief 回调，LVGL输入设备，读取编码器数据
 * @param None
 * @return none
 * @ref lv_indev_read_cb_t
 ******************************************************************************/
static void lvgl_indev_keypad_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    key_event_t key_event;
    static uint32_t last_key = LV_KEY_ENTER;
    static lv_indev_state_t last_state = LV_INDEV_STATE_RELEASED;

    // ESP_LOGI(TAG, "keypad read cb");
    // 这里应该读取实际的按键状态并填充data结构体
    // 例如：
    // data->state = (gpio_get_level(KEY_GPIO) == 0) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    // data->key = ...; // 设置按键值，例如LV_KEY_ENTER, LV_KEY_UP等

    memset(&key_event, 0, sizeof(key_event_t));

    /* 必须每次都返回当前状态，否则长按计时无法累计 */
    data->key = last_key;
    data->state = last_state;
    data->continue_reading = false;

    BaseType_t xReturned = xQueueReceive(keys_event_queue, &key_event, 0);
    if(xReturned == pdFAIL)
    {
        return;
    }

    if (key_event.id >= KEY_ENUM_COUNT)
    {
        ESP_LOGE(TAG, "Invalid key id: %d", key_event.id);
        return;
    }

    if (key_event.state == KEY_STATE_PRESSED)
    {
        last_state = LV_INDEV_STATE_PRESSED;
    }
    else if (key_event.state == KEY_STATE_RELEASED)
    {
        last_state = LV_INDEV_STATE_RELEASED;
    }
    else
    {
        last_state = LV_INDEV_STATE_RELEASED; // 默认状态
    }

    if (current_key_map_mode == KEY_MAP_NAV)
    {
        if (key_event.id == KEY_1)
            last_key = LV_KEY_PREV;
        else if (key_event.id == KEY_2)
            last_key = LV_KEY_ENTER;
        else if (key_event.id == KEY_3)
            last_key = LV_KEY_NEXT;
    }
    else if (current_key_map_mode == KEY_MAP_DIR_UD)
    {
        if (key_event.id == KEY_1)
            last_key = LV_KEY_UP;
        else if (key_event.id == KEY_2)
            last_key = LV_KEY_ENTER;
        else if (key_event.id == KEY_3)
            last_key = LV_KEY_DOWN;
    }
    else if (current_key_map_mode == KEY_MAP_DIR_LR)
    {
        if (key_event.id == KEY_1)
            last_key = LV_KEY_LEFT;
        else if (key_event.id == KEY_2)
            last_key = LV_KEY_ENTER;
        else if (key_event.id == KEY_3)
            last_key = LV_KEY_RIGHT;
    }

    // ESP_LOGI(TAG, "Key %d state changed to %s", key_event.id,
    //          (key_event.state == KEY_STATE_PRESSED) ? "PRESSED" : "RELEASED");

    data->key = last_key;
    data->state = last_state;
    data->continue_reading = true;
}

/*******************************************************************************
 * @brief LVGL初始化
 * @param None
 * @return none
 ******************************************************************************/
void lvgl_app_init(void)
{

    // 显示缓冲区像素总数
    const uint32_t display_buffer_pixels =
        (uint32_t)LCD_HOR_PIXEL * LCD_DRAW_BUFFER_HEIGHT;

    // 背光PWM初始化
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LCD_BL_LEDC_TIMER,
        .freq_hz = LCD_BL_LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_BL_LEDC_CHANNEL,
        .timer_sel = LCD_BL_LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LCD_BL_GPIO,
        .duty = 0, // Set duty to 50%
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    set_lcd_bl_off();

    // SPI初始化
    const spi_bus_config_t SpiBusConfig = {
        .mosi_io_num = LCD_MOSI_GPIO,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = LCD_SCLK_GPIO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz =
            (uint32_t)LCD_VER_PIXEL * LCD_HOR_PIXEL * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(
        spi_bus_initialize(LCD_SPI_NUM, &SpiBusConfig, SPI_DMA_CH_AUTO));

    const esp_lcd_panel_io_spi_config_t IoConfig = {
        .dc_gpio_num = LCD_DC_GPIO,
        .cs_gpio_num = LCD_CS_GPIO,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 20,
    };
    esp_lcd_panel_io_handle_t IoHandle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &IoConfig, &IoHandle));

    const esp_lcd_panel_dev_config_t PanelConfig = {
        .reset_gpio_num = LCD_RST_GPIO,
        .color_space = LCD_COLOR_SPACE,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        .flags.reset_active_high = 0,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(IoHandle, &PanelConfig, &PanelHandle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(PanelHandle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(PanelHandle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(PanelHandle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(PanelHandle, true));

    // LVGL参数配置
    const lvgl_port_cfg_t lvgl_port_cfg = {
        .task_priority = 4,      // LVGL task priority
        .task_stack = 8192,      // LVGL task stack size
        .task_affinity = 1,      // LVGL task pinned to core 1 (main is on core 0)
        .task_max_sleep_ms = 10, // Maximum sleep in LVGL task
        .timer_period_ms = 10    // LVGL timer tick period in ms
    };
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_port_cfg));

    ESP_ERROR_CHECK(lvgl_port_lock(portMAX_DELAY) ? ESP_OK : ESP_FAIL);

    const lvgl_port_display_cfg_t DisplayConfig = {
        .io_handle = IoHandle,
        .panel_handle = PanelHandle,
        .buffer_size = display_buffer_pixels,
        .double_buffer = true, // Use double buffering
        .hres = LCD_HOR_PIXEL,
        .vres = LCD_VER_PIXEL,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565_SWAPPED,
        .rotation =
            {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
        .flags =
            {
                .buff_dma = true,
                .buff_spiram = false,
                .swap_bytes = false,
            },
    };
    gLvDisplay = lvgl_port_add_disp(&DisplayConfig);
    ESP_ERROR_CHECK(gLvDisplay ? ESP_OK : ESP_FAIL);

    // 设置旋转参数
    lv_display_set_rotation(gLvDisplay, LV_DISPLAY_ROTATION_90);
    
    lv_obj_t *screen = lv_screen_active();
    ESP_ERROR_CHECK(screen ? ESP_OK : ESP_FAIL);
    
    ESP_LOGI(TAG, "lcd panel init done");
    
    lv_group_t *group = lv_group_get_default();
    if (!group)
    {
        group = lv_group_create();
        lv_group_set_default(group);
        lv_group_remove_all_objs(group);
    }

    gLvIndev = lv_indev_create();
    lv_indev_set_type(gLvIndev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(gLvIndev, lvgl_indev_keypad_cb);
    lv_indev_set_display(gLvIndev, gLvDisplay);
    lv_indev_set_group(gLvIndev, group);

    lvgl_port_unlock();
    ESP_LOGI(TAG, "lvgl_app_init");
}

/*******************************************************************************
 * @brief 打开屏幕
 * @param None
 * @return None
 ******************************************************************************/
void set_lcd_on(void)
{
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(PanelHandle, true));
    set_lcd_bl_on();
}

/*******************************************************************************
 * @brief 切换到初始的显示界面
 * @param None
 * @return None
 ******************************************************************************/
void screen_init(void)
{
    esp_err_t esp_err;

    // esp_err = screen_switch(&startup_screen);
    // ESP_ERROR_CHECK(esp_err);

    // DEBUG
    set_lcd_on();


    ESP_ERROR_CHECK(lvgl_port_lock(portMAX_DELAY) ? ESP_OK : ESP_FAIL);
    lv_obj_clean(lv_scr_act());                                              // 清空屏幕
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN); // 背景色
    lvgl_port_unlock();

    // esp_err = screen_switch(&startup_screen);
    // ESP_ERROR_CHECK(esp_err);

    // esp_err = screen_switch(&setting_screen);
    esp_err = screen_switch(&settings_screen);
    ESP_ERROR_CHECK(esp_err);
}

/*******************************************************************************
 * @brief 切换显示界面
 * @param None
 * @return None
 ******************************************************************************/
esp_err_t screen_switch(const screen_t *screen)
{
    return screen_switch_arg(screen, NULL);
}

esp_err_t screen_switch_arg(const screen_t *screen, void *user_data)
{
    if (screen == NULL)
        return ESP_FAIL;

    if (screen->init_cb == NULL)
        return ESP_FAIL;

    if (screen->loop_cb == NULL)
        return ESP_FAIL;

    portENTER_CRITICAL(&screen_switch_mux);
    switch_screen_user_data = user_data;
    wait_switch_screen = (screen_t *)screen;
    screen_switch_flag = 1;
    portEXIT_CRITICAL(&screen_switch_mux);

    // ESP_LOGI(TAG, "waiting switch_screen to %s", wait_switch_screen->name);

    return ESP_OK;
}

/*******************************************************************************
 * @brief 获取上一个界面
 * @param None
 * @return none
 ******************************************************************************/
screen_t *screen_get_prev(void)
{
    return prev_screen;
}

/*******************************************************************************
 * @brief 清除屏幕
 * @param None
 * @return none
 ******************************************************************************/
void screen_clear(void)
{
    lv_obj_t *parent = lv_scr_act();

    lv_obj_clean(parent); // 清空屏幕
    lv_obj_set_style_bg_color(parent, lv_color_make(0x2D, 0x43, 0x56),
                              LV_PART_MAIN); // 背景色

    lv_obj_set_style_pad_left(parent, 0, LV_PART_MAIN); // 内边距

    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE); // 禁止滚动
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);
}

/*******************************************************************************
 * @brief 循环处理业务逻辑
 * @param None
 * @return none
 ******************************************************************************/
void screen_loop_handler(void)
{
    screen_t *next_screen = NULL;

    portENTER_CRITICAL(&screen_switch_mux);
    if (screen_switch_flag)
    {
        screen_switch_flag = 0;
        next_screen = wait_switch_screen;
    }
    portEXIT_CRITICAL(&screen_switch_mux);

    if (next_screen)
    {
        lvgl_port_lock(portMAX_DELAY);

        if (current_screen != NULL)
        {
            if (current_screen->deinit_cb)
                current_screen->deinit_cb();
        }

        prev_screen = current_screen;
        lv_obj_t *parent = lv_obj_create(NULL);

        next_screen->init_cb(parent);

        if (screen_load_anim >= 0)
        {
            lv_screen_load_anim(parent, screen_load_anim, 500, 0, true);
            screen_load_anim = -1;
        }
        else
        {
            lv_screen_load(parent);
        }

        lvgl_port_unlock();

        // ESP_LOGI(TAG, "switch_screen to %s", wait_switch_screen->name);

        current_screen = next_screen;
    }

    if (current_screen == NULL)
        return;

    if (current_screen->loop_cb)
    {
        bool locked = lvgl_port_lock(0);

        if (locked)
        {
            current_screen->loop_cb();
            lvgl_port_unlock();
        }
    }
}

void lv_add_debug_border(lv_obj_t *obj)
{
#if CFG_USE_DEBUG_BORDER
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_make(0xFF, 0x00, 0x00), LV_PART_MAIN);
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
#endif // CFG_USE_DEBUG_BORDER
}

void screen_set_load_anim(lv_screen_load_anim_t load_anim)
{
    screen_load_anim = load_anim;
}
