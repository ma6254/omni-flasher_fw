#ifndef KEYS_H
#define KEYS_H

#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define KEY_FILT_MASK 0x03

typedef enum
{
    KEY_1 = 0,
    KEY_2,
    KEY_3,
    KEY_ENUM_COUNT,
} key_id_t;

typedef enum
{
    KEY_STATE_NONE = 0,
    KEY_STATE_RELEASED = 1,
    KEY_STATE_PRESSED = 2,
} key_state_t;

typedef struct
{
    uint8_t id;    // key_id_t
    uint8_t state; // key_state_t
} key_event_t;

typedef struct
{
    uint8_t filt_cnt;
    key_state_t last_raw_state;
    key_state_t last_state;
    uint8_t is_raw_inited;
    uint8_t is_inited;
} key_filt_t;

typedef struct
{
    key_filt_t filt[KEY_ENUM_COUNT];
} global_key_t;

extern global_key_t g_key;
extern QueueHandle_t keys_event_queue;

void key_init(void);
void key_task_handler(void);

#endif // KEYS_H
