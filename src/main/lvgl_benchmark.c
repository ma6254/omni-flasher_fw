#include "lvgl_benchmark.h"
#include <esp_log.h>
#include "lvgl_app.h"

const static char *TAG = "lvgl_benchmark";

/*******************************************************************************
 * @brief 回调函数，LVGL基准测试结束时调用
 * @param summary  benchmark结果的总结
 * @return none
 * @ref lv_demo_benchmark_on_end_cb_t
 ******************************************************************************/
static void lv_demo_benchmark_on_end_cb(const lv_demo_benchmark_summary_t * summary)
{
    ESP_LOGI(TAG, "========== Benchmark Results ==========");
    ESP_LOGI(TAG, "%-32s %6s %6s %10s %10s", "Scene", "FPS", "CPU%", "Render(ms)", "Flush(ms)");
    ESP_LOGI(TAG, "-----------------------------------------------------------------------");

    lv_demo_benchmark_scene_dsc_t *curr_scene = summary->scenes;
    while (1)
    {
        if (curr_scene->create_cb == NULL)
        {
            break;
        }

        if (curr_scene->measurement_cnt == 0) {
            ESP_LOGI(TAG, "%-32s  (skipped)", curr_scene->name);
        } else {
            int32_t cnt = curr_scene->measurement_cnt;
            ESP_LOGI(TAG, "%-32s %6" PRIu32 " %6" PRIu32 " %10" PRIu32 " %10" PRIu32,
                     curr_scene->name,
                     curr_scene->fps_avg / cnt,
                     curr_scene->cpu_avg_usage / cnt,
                     curr_scene->render_avg_time / cnt,
                     curr_scene->flush_avg_time / cnt);
        }

        curr_scene++;
    }

    int32_t valid_cnt = summary->valid_scene_cnt > 0 ? summary->valid_scene_cnt : 1;
    ESP_LOGI(TAG, "-----------------------------------------------------------------------");
    ESP_LOGI(TAG, "Valid scenes : %" PRId32, summary->valid_scene_cnt);
    ESP_LOGI(TAG, "Avg FPS      : %" PRId32, summary->total_avg_fps / valid_cnt);
    ESP_LOGI(TAG, "Avg CPU%%     : %" PRId32, summary->total_avg_cpu / valid_cnt);
    ESP_LOGI(TAG, "Avg Render   : %" PRId32 " ms", summary->total_avg_render_time / valid_cnt);
    ESP_LOGI(TAG, "Avg Flush    : %" PRId32 " ms", summary->total_avg_flush_time / valid_cnt);
    ESP_LOGI(TAG, "=======================================");

    lv_demo_benchmark_summary_display(summary);
}

/*******************************************************************************
 * @brief LVGL基准测试
 * @param None
 * @return none
 ******************************************************************************/
void lv_demo_benchmark_init(void)
{
    lv_demo_benchmark_set_end_cb(lv_demo_benchmark_on_end_cb);

    ESP_ERROR_CHECK(lvgl_port_lock(portMAX_DELAY) ? ESP_OK : ESP_FAIL);
    lv_demo_benchmark();
    lvgl_port_unlock();
}
