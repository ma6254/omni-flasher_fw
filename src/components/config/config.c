
#include <string.h>
#include <stdio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "config.h"
#include "lvgl_app.h"
#include "lvgl_app.h"

static const char *TAG = "config";

static nvs_handle_t config_data_nvs_handle;
config_data_t g_config_data;

const char *config_buzzer_enable_names[] = {"OFF", "ON", NULL};

// 语言
const char *config_language_names[CONFIG_LANGUAGE_ENUM_COUNT + 1] = {
    "ZH-CN",
    "EN-US",
    NULL,
};

// 背光超时
const char *config_bl_off_timeout_setting_names[CONFIG_BL_OFF_TIMEOUT_ENUM_COUNT + 1] = {
    "关闭",
    "10S",
    "30S",
    "60S",
    "120S",
    NULL,
};

// 屏幕超时
const char *config_screen_lock_timeout_setting_names[CONFIG_SCREEN_LOCK_TIMEOUT_ENUM_COUNT + 1] = {
    "关闭",
    "10S",
    "30S",
    "60S",
    NULL,
};

// 其他屏幕超时
const char *config_other_screen_timeout_setting_names[] = {
    "关闭",
    "10S",
    "30S",
    NULL,
};

// 其他屏幕超时
const char *config_in_clk_freq_setting_names[] = {
    "10MHz",
    "11.2896MHz",
    "12.288MHz",
    NULL,
};

static void config_data_init(config_data_t *config_data)
{
    memset(config_data, 0, sizeof(config_data_t));

    config_data->language = CONFIG_LANGUAGE_CN_ZH;                       //
    config_data->brightness = SCREEN_BRIGHTNESS_MAX;                     //
    config_data->buzzer_enable = 1;                                      //
    config_data->bl_off_timeout = CONFIG_BL_OFF_TIMEOUT_120S;            //
    config_data->screen_lock_timeout = CONFIG_SCREEN_LOCK_TIMEOUT_60S;   //
    config_data->other_screen_timeout = CONFIG_OTHER_SCREEN_TIMEOUT_30S; //
}

/*******************************************************************************
 * @brief 配置数据加载
 * @param None
 * @return none
 ******************************************************************************/
static esp_err_t config_data_load(config_data_t *config_data)
{
    esp_err_t err;
    uint32_t tmp_u32;

    ESP_LOGI(TAG, "load config data");

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_HEADER, &tmp_u32);
    if (err == ESP_ERR_NVS_NOT_FOUND)
        return ESP_ERR_NVS_NOT_FOUND;

    if (tmp_u32 != CONFIG_HEADER_VALUE)
        return ESP_ERR_NVS_NOT_FOUND;

    ESP_ERROR_CHECK(err);

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_START_COUNT, &config_data->start_count);
    ESP_ERROR_CHECK(err);

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_LANGUAGE, &config_data->language);
    ESP_ERROR_CHECK(err);

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_BRIGHTNESS, &config_data->brightness);
    ESP_ERROR_CHECK(err);

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_BUZZER_ENABLE, &config_data->buzzer_enable);
    ESP_ERROR_CHECK(err);

    err = nvs_get_u32(config_data_nvs_handle, CONFIG_KEY_BL_OFF_TIMEOUT, &config_data->bl_off_timeout);
    ESP_ERROR_CHECK(err);

    return ESP_OK;
}

/*******************************************************************************
 * @brief 配置数据保存
 * @param None
 * @return none
 ******************************************************************************/
static void config_data_save(const config_data_t *config_data)
{
    esp_err_t err;

    ESP_LOGI(TAG, "save config data");

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_HEADER, CONFIG_HEADER_VALUE);
    ESP_ERROR_CHECK(err);

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_START_COUNT, config_data->start_count);
    ESP_ERROR_CHECK(err);

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_LANGUAGE, config_data->language);
    ESP_ERROR_CHECK(err);

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_BRIGHTNESS, config_data->brightness);
    ESP_ERROR_CHECK(err);

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_BUZZER_ENABLE, config_data->buzzer_enable);
    ESP_ERROR_CHECK(err);

    err = nvs_set_u32(config_data_nvs_handle, CONFIG_KEY_BL_OFF_TIMEOUT, config_data->bl_off_timeout);
    ESP_ERROR_CHECK(err);
}

static esp_err_t config_data_print(const config_data_t *config_data)
{
    ESP_LOGI(TAG, "=============================================================");
    ESP_LOGI(TAG, "config data:");
    ESP_LOGI(TAG, "           start_count: %2" PRIu32, config_data->start_count);
    ESP_LOGI(TAG, "              language: %2" PRIu32 " %s", config_data->language,config_language_names[config_data->language]);
    ESP_LOGI(TAG, "            brightness: %2" PRIu32, config_data->brightness);
    ESP_LOGI(TAG, "         buzzer_enable: %2" PRIu32 " %s", config_data->buzzer_enable, config_buzzer_enable_names[config_data->buzzer_enable]);
    ESP_LOGI(TAG, "        bl_off_timeout: %2" PRIu32 " %s", config_data->bl_off_timeout, config_bl_off_timeout_setting_names[config_data->bl_off_timeout]);
    ESP_LOGI(TAG, "   screen_lock_timeout: %2" PRIu32 " %s", config_data->screen_lock_timeout, config_screen_lock_timeout_setting_names[config_data->screen_lock_timeout]);
    ESP_LOGI(TAG, "  other_screen_timeout: %2" PRIu32 " %s", config_data->other_screen_timeout, config_other_screen_timeout_setting_names[config_data->other_screen_timeout]);
    ESP_LOGI(TAG, "=============================================================");

    return ESP_OK;
}

/*******************************************************************************
 * @brief 配置加载
 * @param None
 * @return none
 ******************************************************************************/
void config_load(void)
{
    config_data_load(&g_config_data);
}

/*******************************************************************************
 * @brief 配置保存
 * @param None
 * @return none
 ******************************************************************************/
void config_save(void)
{
    config_data_save(&g_config_data);
    // config_data_print(&g_config_data);
}

/*******************************************************************************
 * @brief 配置初始化
 * @param None
 * @return none
 ******************************************************************************/
void config_init(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS handle`
    err = nvs_open(CONFIG_NANMESPACE_NAME, NVS_READWRITE, &config_data_nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }
    ESP_ERROR_CHECK(err);

    err = config_data_load(&g_config_data);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "config data not found, initializing to defaults");
        config_data_init(&g_config_data);
        config_data_save(&g_config_data);
    }
    else
    {
        ESP_ERROR_CHECK(err);
    }

    g_config_data.start_count++;
    config_data_save(&g_config_data);

    config_data_print(&g_config_data);

    ESP_LOGI(TAG, "initialized");
}

/*******************************************************************************
 * @brief 配置恢复出厂设置
 * @param None
 * @return none
 ******************************************************************************/
void config_factory_reset(void)
{
    config_data_init(&g_config_data);
    config_data_save(&g_config_data);
}
