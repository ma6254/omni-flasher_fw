#ifndef SERPROG_H
#define SERPROG_H

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SERPROG_ACK 0x06
#define SERPROG_NACK 0x15

typedef enum
{
    SERPROG_CMD_NOP = 0,     // 0x00 No operation
    SERPROG_CMD_Q_IFACE,     // 0x01 Query interface version
    SERPROG_CMD_Q_CMDMAP,    // 0x02 Query supported commands bitmap
    SERPROG_CMD_Q_PGMNAME,   // 0x03 Query programmer name
    SERPROG_CMD_Q_SERBUF,    // 0x04 Query Serial Buffer Size
    SERPROG_CMD_Q_BUSTYPE,   // 0x05 Query supported bustypes
    SERPROG_CMD_Q_CHIPSIZE,  // 0x06 Query supported chipsize (2^n format)
    SERPROG_CMD_Q_OPBUF,     // 0x07 Query operation buffer size
    SERPROG_CMD_Q_WRNMAXLEN, // 0x08 Query Write to opbuf: Write-N maximum length
    SERPROG_CMD_R_BYTE,      // 0x09 Read a single byte
    SERPROG_CMD_R_NBYTES,    // 0x0A Read n bytes
    SERPROG_CMD_O_INIT,      // 0x0B Initialize operation buffer
    SERPROG_CMD_O_WRITEB,    // 0x0C Write opbuf: Write byte with address
    SERPROG_CMD_O_WRITEN,    // 0x0D Write to opbuf: Write-N
    SERPROG_CMD_O_DELAY,     // 0x0E Write opbuf: udelay
    SERPROG_CMD_O_EXEC,      // 0x0F Execute operation buffer
    SERPROG_CMD_SYNCNOP,     // 0x10 Special no-operation that returns NAK+ACK
    SERPROG_CMD_Q_RDNMAXLEN, // 0x11 Query read-n maximum length
    SERPROG_CMD_S_BUSTYPE,   // 0x12 Set used bustype(s).
    SERPROG_CMD_O_SPIOP,     // 0x13 Perform SPI operation.
    SERPROG_CMD_S_SPI_FREQ,  // 0x14 Set SPI clock frequency
    SERPROG_CMD_S_PIN_STATE, // 0x15  Enable/disable output drivers
} serprog_cmd_t;

typedef enum
{
    SERPROG_EVENT_NONE = 0,
    SERPROG_EVENT_WORK_START,
    SERPROG_EVENT_WORK_END,
    SERPROG_EVENT_SPEED_NOTIFY, // 实时通讯速度
} serprog_event_code_t;

extern QueueHandle_t serprog_event_queue;

esp_err_t serprog_init(void);
void serprog_deinit(void);

#endif // SERPROG_H
