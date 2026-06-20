#include <esp_log.h>
#include <string.h>
#include "key.h"

static const char *TAG = "key";

global_key_t g_key;
QueueHandle_t keys_event_queue = NULL; // item: key_event_t

void key_init(void)
{
    memset(&g_key, 0, sizeof(global_key_t));

    keys_event_queue = xQueueCreate(10, sizeof(key_event_t));
}

static void key_process(key_id_t id, key_filt_t *filt, key_state_t state)
{
    if (filt == NULL)
        return;

    if (state == KEY_STATE_NONE)
        return;

    // 是否是第一次运行，则初始化状态
    if(!filt->is_raw_inited)
    {
        filt->last_raw_state = state;
        filt->is_raw_inited = 1;
        return;
    }

    filt->filt_cnt <<= 1;
    if (state == KEY_STATE_PRESSED)
        filt->filt_cnt |= 1;

    key_state_t now_state = KEY_STATE_NONE;

    if ((filt->filt_cnt & KEY_FILT_MASK) == 0x00)
    {
        now_state = KEY_STATE_RELEASED;
    }
    else if ((filt->filt_cnt & KEY_FILT_MASK) == KEY_FILT_MASK)
    {
        now_state = KEY_STATE_PRESSED;
    }

    if (filt->is_inited == 0)
    {
        filt->is_inited = 1;
    }
    else if ((now_state != KEY_STATE_NONE) && (now_state != filt->last_state))
    {
        filt->is_inited = 1;

        key_event_t event = {
            .id = id,
            .state = now_state,
        };
        BaseType_t xReturned = xQueueSend(keys_event_queue, &event, 0);
        if(xReturned != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to send key event to queue");
        }

        // ESP_LOGI(TAG, "Key %d state changed to %s", id, (now_state == KEY_STATE_PRESSED) ? "PRESSED" : "RELEASED");
    }

    filt->last_state = now_state;
    filt->last_raw_state = state;
}

void key_task_handler(void)
{
    uint32_t key_raw_val = 0;

    if(read_key1_io())
        key_raw_val |= (1 << KEY_1);

    if (read_key2_io())
        key_raw_val |= (1 << KEY_2);

    if (read_key3_io())
        key_raw_val |= (1 << KEY_3);

    for(uint8_t i = 0; i < KEY_ENUM_COUNT; i++)
    {
        key_filt_t *curr_filt = &g_key.filt[i];
        key_state_t curr_state = (key_raw_val & (1 << i)) ? KEY_STATE_PRESSED : KEY_STATE_RELEASED;

        key_process(i, curr_filt, curr_state);
    }
}