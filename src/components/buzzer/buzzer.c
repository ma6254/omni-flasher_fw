#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include "board.h"
#include "buzzer.h"

#define BUZZER_LEDC_FREQ (4000)            // 蜂鸣器频率，单位Hz
#define BUZZER_LEDC_TIMER LEDC_TIMER_2     // 蜂鸣器使用的LEDC定时器
#define BUZZER_LEDC_CHANNEL LEDC_CHANNEL_2 // 蜂鸣器使用的LEDC通道

static const char *TAG = "buzzer";

typedef enum
{

    BUZZER_STEP_IDLE = 0, // 空闲状态
    BUZZER_STEP_OFF,      // 关闭状态
    BUZZER_STEP_ON,       // 打开状态
} buzzer_step_t;

global_buzzer_t g_buzzer;

/*******************************************************************************
 * @brief 初始化蜂鸣器句柄
 * @param handle 蜂鸣器句柄
 * @param cfg    蜂鸣器配置
 * @return None
 ******************************************************************************/
esp_err_t buzzer_handle_init(buzzer_handle_t *handle, const buzzer_cfg_t *cfg)
{
    esp_err_t err;

    if (handle == NULL || cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    memset(handle, 0, sizeof(buzzer_handle_t));
    handle->cfg = *cfg;

    if (cfg->type == BUZZER_PASSIVE)
    {
        // 配置LEDC定时器
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = cfg->timer_num,
            .freq_hz = cfg->freq_hz,
            .clk_cfg = LEDC_AUTO_CLK,
        };
        err = ledc_timer_config(&ledc_timer);
        if (err != ESP_OK)
        {
            return err;
        }

        // 配置LEDC通道
        ledc_channel_config_t ledc_channel = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = cfg->channel,
            .timer_sel = cfg->timer_num,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = cfg->gpio_num,
            .duty = 0, // 初始占空比为0
            .hpoint = 0,
        };
        err = ledc_channel_config(&ledc_channel);
        if (err != ESP_OK)
        {
            return err;
        }
    }
    else if (cfg->type == BUZZER_ACTIVE)
    {
        // 有源蜂鸣器只需要配置GPIO为输出模式
        gpio_reset_pin(cfg->gpio_num);
        gpio_set_direction(cfg->gpio_num, GPIO_MODE_OUTPUT);
    }
    else
    {
        return ESP_ERR_INVALID_ARG; // 无效的蜂鸣器类型
    }

    return ESP_OK;
}

/*******************************************************************************
 * @brief 初始化蜂鸣器句柄
 * @param handle 蜂鸣器句柄
 * @param cfg    蜂鸣器配置
 * @return None
 ******************************************************************************/
esp_err_t buzzer_handle_io_write(buzzer_handle_t *handle, bool state)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->cfg.type == BUZZER_PASSIVE)
    {

        uint32_t duty;

        if (handle->cfg.inv_level == 0)
        {
            if (state)
            {
                duty = 512; // 输出PWM波
            }
            else
            {
                duty = 0; // 空闲低电平
            }
        }
        else
        {
            if (state)
            {
                duty = 512; // 输出PWM波
            }
            else
            {
                duty = 1024 - 1; // 空闲高电平
            }
        }

        // 设置占空比
        ledc_set_duty(LEDC_LOW_SPEED_MODE, handle->cfg.channel, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, handle->cfg.channel);
    }
    else if (handle->cfg.type == BUZZER_ACTIVE)
    {
        uint32_t level = state ? 1 : 0;

        if (handle->cfg.inv_level)
        {
            level = !level; // 如果配置了反相输出，则取反
        }

        gpio_set_level(handle->cfg.gpio_num, level);
    }
    else
    {
        return ESP_ERR_INVALID_ARG; // 无效的蜂鸣器类型
    }

    return ESP_OK;
}

/*******************************************************************************
 * @brief 设置蜂鸣器参数
 * @param handle 蜂鸣器句柄
 * @param param  蜂鸣器参数
 * @return None
 ******************************************************************************/
esp_err_t buzzer_handle_set_param(buzzer_handle_t *handle, const buzzer_param_t *param)
{
    if (handle == NULL || param == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if ((param->dur_ms == 0) || (param->count == 0))
    {
        return ESP_ERR_INVALID_ARG;
    }

    handle->pending_param = *param;
    handle->pending_valid = 0;

    return ESP_OK;
}

/*******************************************************************************
 * @brief 处理蜂鸣器任务
 * @param None
 * @return None
 ******************************************************************************/
void buzzer_handle_process(buzzer_handle_t *handle)
{
    if (handle == NULL)
        return;

    // 如果有立即执行的请求，优先处理
    if (handle->pending_param.immed)
    {
        handle->pending_param.immed = 0;

        handle->time_cnt = handle->pending_param.dur_ms / BUZZER_TASK_TIME_MS;
        handle->buzz_cnt = handle->pending_param.count;
        handle->now_param = handle->pending_param;
        handle->pending_valid = 1;
        handle->step = BUZZER_STEP_ON;
    }

    if (handle->time_cnt > 0)
        handle->time_cnt--;

    if (handle->time_cnt)
        return;

    switch (handle->step)
    {
    case BUZZER_STEP_IDLE:
    {
        buzzer_handle_io_write(handle, false); // 关蜂鸣器

        // 如果有新的参数更新，立即应用
        // 用于不立即执行的请求，等待前面周期结束后再应用
        if (handle->pending_valid == 0)
        {
            handle->pending_valid = 1;

            handle->time_cnt = handle->pending_param.dur_ms / BUZZER_TASK_TIME_MS;
            handle->buzz_cnt = handle->pending_param.count;
            handle->now_param = handle->pending_param;
            handle->step = BUZZER_STEP_ON;
            break;
        }

        if (handle->buzz_cnt > 0)
            handle->step = BUZZER_STEP_ON;
    }
    break;
    case BUZZER_STEP_OFF:
    {
        buzzer_handle_io_write(handle, false);
        handle->time_cnt = handle->now_param.dur_ms / BUZZER_TASK_TIME_MS; // 设置停叫的时间
        handle->step = BUZZER_STEP_IDLE;
    }
    break;
    case BUZZER_STEP_ON:
    {
        buzzer_handle_io_write(handle, true);
        if (handle->buzz_cnt > 0)
            handle->buzz_cnt--;                                            // 蜂鸣器叫声往-1
        handle->time_cnt = handle->now_param.dur_ms / BUZZER_TASK_TIME_MS; // 设置叫的时间
        handle->step = BUZZER_STEP_OFF;
    }
    break;
    default:
        break;
    }
}

/*******************************************************************************
 * @brief 初始化蜂鸣器
 * @param None
 * @return None
 ******************************************************************************/
void buzzer_init(void)
{
    esp_err_t err;

    memset(&g_buzzer, 0, sizeof(g_buzzer));

    buzzer_cfg_t cfg = {
        .type = BUZZER_PASSIVE,
        .gpio_num = BUZZER_GPIO,
        .timer_num = BUZZER_LEDC_TIMER,
        .channel = BUZZER_LEDC_CHANNEL,
        .freq_hz = BUZZER_LEDC_FREQ,
        .inv_level = 0, // 不反相输出
    };

    err = buzzer_handle_init(&g_buzzer.handle, &cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize buzzer handle: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "initialized successfully");
}

/*******************************************************************************
 * @brief 蜂鸣器任务处理函数
 * @param None
 * @return None
 ******************************************************************************/
void buzzer_task_process(void)
{
    buzzer_handle_process(&g_buzzer.handle);
}

void buzzer_set(uint32_t dur_ms, uint32_t count, bool immed)
{
    esp_err_t err;

    buzzer_param_t param = {
        .dur_ms = dur_ms,
        .count = count,
        .immed = immed ? 1 : 0,
    };

    err = buzzer_handle_set_param(&g_buzzer.handle, &param);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set buzzer parameters: %s", esp_err_to_name(err));
    }
}
