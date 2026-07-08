#ifndef BUZZER_H
#define BUZZER_H

#include <driver/gpio.h>
#include <driver/ledc.h>


/*******************************************************************************
 *
 * 参数定义
 *
 ******************************************************************************/

#define BUZZER_TASK_TIME_MS (10) // 蜂鸣器任务处理周期，单位ms

/*******************************************************************************
 *
 * 数据结构定义
 *
 ******************************************************************************/

typedef enum
{
    // BUZZER_NONE = 0,
    BUZZER_PASSIVE = 1, // 无源蜂鸣器
    BUZZER_ACTIVE,      // 有源蜂鸣器
} buzzer_type_t;

// 蜂鸣器配置结构体
typedef struct BUZZER_CFG_T
{
    // 公共配置
    buzzer_type_t type;     // 蜂鸣器类型
    gpio_num_t gpio_num;    // 蜂鸣器GPIO引脚号
    struct
    {
        uint32_t inv_level : 1;   // 是否反相电平输出
        uint32_t resv_flags : 30; // 保留标志位
    };

    // 无源蜂鸣器专用配置
    struct
    {
        ledc_timer_t timer_num; // LEDC定时器编号
        ledc_channel_t channel; // LEDC通道编号
        uint32_t freq_hz;       // 蜂鸣器频率，单位Hz
    };

} buzzer_cfg_t;

// 蜂鸣器参数结构体
typedef struct
{
    uint32_t dur_ms; // 蜂鸣器响起的时间，单位毫秒
    uint32_t count;  // 蜂鸣器响起的次数
    struct
    {
        uint32_t immed : 1;       // 是否立刻更新参数
        uint32_t resv_flags : 31; // 保留标志位
    };
} buzzer_param_t;

// 蜂鸣器句柄结构体
typedef struct BUZZER_HANDLE_T
{
    buzzer_cfg_t cfg;
    bool is_enabled;
    uint32_t time_cnt;
    uint32_t buzz_cnt;
    uint8_t step;
    buzzer_param_t pending_param;
    buzzer_param_t now_param;
    struct
    {
        uint32_t pending_valid : 1; // 参数是否有效
        uint32_t resv_flags : 31;   // 保留标志位
    };
} buzzer_handle_t;

// 全局蜂鸣器结构体
typedef struct GLOBAL_BUZZER_T
{
    buzzer_handle_t handle;
} global_buzzer_t;

extern global_buzzer_t g_buzzer;

esp_err_t buzzer_handle_init(buzzer_handle_t *handle, const buzzer_cfg_t *cfg);
esp_err_t buzzer_handle_io_write(buzzer_handle_t *handle, bool state);
esp_err_t buzzer_handle_set_param(buzzer_handle_t *handle, const buzzer_param_t *param);
void buzzer_handle_process(buzzer_handle_t *handle);


void buzzer_init(void);
void buzzer_task_process(void);
void buzzer_set(uint32_t dur_ms, uint32_t count, bool immed);
void buzzer_set_enable(bool enable);

#define set_buzzer_io_on() (buzzer_handle_io_write(&g_buzzer.handle, true))
#define set_buzzer_io_off() (buzzer_handle_io_write(&g_buzzer.handle, false))

#endif // BUZZER_H
