#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_NANMESPACE_NAME "config"

#define CONFIG_ENABLE_WATERMARK 1

#define CONFIG_HEADER_VALUE 0xA5A55A5A

#define CONFIG_KEY_HEADER "header"
#define CONFIG_KEY_START_COUNT "start_count"
#define CONFIG_KEY_LANGUAGE "language"
#define CONFIG_KEY_BRIGHTNESS "brightness"
#define CONFIG_KEY_BUZZER_ENABLE "buzzer_enable"
#define CONFIG_KEY_BL_OFF_TIMEOUT "bl_off_time"

typedef struct
{
    uint32_t start_count;          // 上电次数
    uint32_t language;             // 语言
    uint32_t brightness;           // 背光亮度
    uint32_t buzzer_enable;        // 蜂鸣器使能
    uint32_t bl_off_timeout;       // 自动熄屏
    uint32_t screen_lock_timeout;  // 锁屏超时
    uint32_t other_screen_timeout; // 其他屏幕超时
} config_data_t;

// 自动熄屏
typedef enum
{
    CONFIG_BL_OFF_TIMEOUT_OFF = 0,
    CONFIG_BL_OFF_TIMEOUT_10S,
    CONFIG_BL_OFF_TIMEOUT_30S,
    CONFIG_BL_OFF_TIMEOUT_60S,
    CONFIG_BL_OFF_TIMEOUT_120S,
    CONFIG_BL_OFF_TIMEOUT_ENUM_COUNT,
} config_bl_off_timeout_t;

// 锁屏超时
typedef enum
{
    CONFIG_SCREEN_LOCK_TIMEOUT_OFF = 0,
    CONFIG_SCREEN_LOCK_TIMEOUT_10S,
    CONFIG_SCREEN_LOCK_TIMEOUT_30S,
    CONFIG_SCREEN_LOCK_TIMEOUT_60S,
    CONFIG_SCREEN_LOCK_TIMEOUT_ENUM_COUNT,
} config_screen_lock_timeout_t;

// 其他屏幕超时
typedef enum
{
    CONFIG_OTHER_SCREEN_TIMEOUT_OFF = 0,
    CONFIG_OTHER_SCREEN_TIMEOUT_10S,
    CONFIG_OTHER_SCREEN_TIMEOUT_30S,
    CONFIG_OTHER_SCREEN_TIMEOUT_ENUM_COUNT,
} config_other_screen_timeout_t;

// 语言
typedef enum
{
    CONFIG_LANGUAGE_EN_US = 0, // 英文
    CONFIG_LANGUAGE_ZH_CN,     // 中文，简体
    CONFIG_LANGUAGE_ENUM_COUNT // 枚举数量
} config_language_t;

extern const char *config_language_names[CONFIG_LANGUAGE_ENUM_COUNT + 1];
extern const char *config_language_locale_list[CONFIG_LANGUAGE_ENUM_COUNT + 1];
extern const char *config_bl_off_timeout_setting_names[CONFIG_BL_OFF_TIMEOUT_ENUM_COUNT + 1];
extern const char *config_screen_lock_timeout_setting_names[CONFIG_SCREEN_LOCK_TIMEOUT_ENUM_COUNT + 1];
extern const char *config_other_screen_timeout_setting_names[CONFIG_OTHER_SCREEN_TIMEOUT_ENUM_COUNT + 1];

void config_init(void);

void config_load(void);
void config_save(void);
void config_factory_reset(void);

extern config_data_t g_config_data;

#endif // CONFIG_H
