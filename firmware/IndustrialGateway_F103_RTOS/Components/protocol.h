#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#define CMD_GET_DEVICE_INFO       0x03
#define CMD_RESP_DEVICE_INFO      0x83
#define GATEWAY_ID                0x0001
#define FW_VERSION                0x0901
#define PROTOCOL_VERSION          0x0100

#define FEATURE_ADC_QUERY         (1U << 0)
#define FEATURE_LOG_QUERY         (1U << 1)
#define FEATURE_DEVICE_INFO       (1U << 2)
#define FEATURE_RS485_UPSTREAM    (1U << 3)
#define FEATURE_UART_DMA_IDLE     (1U << 4)
#define FEATURE_OLED_STATUS       (1U << 5)

#define FEATURE_FLAGS             (FEATURE_ADC_QUERY      | \
                                   FEATURE_LOG_QUERY      | \
                                   FEATURE_DEVICE_INFO    | \
                                   FEATURE_RS485_UPSTREAM | \
                                   FEATURE_UART_DMA_IDLE  | \
                                   FEATURE_OLED_STATUS)

#define DEVICE_STATUS_OK          0x0001

#include "stdint.h"
#include "log_app.h"
uint16_t Protocol_CalcCRC16(uint8_t *data, uint16_t len);
uint8_t Protocol_SendDeviceInfo(void);

#define PROTOCOL_LEN_GET_LOG_INFO      0x01
#define PROTOCOL_LEN_RESP_LOG_INFO     0x10
#define FRAME_LEN_GET_LOG_INFO         6
#define FRAME_LEN_RESP_LOG_INFO        21
#define CMD_GET_LOG_INFO               0x04
#define CMD_RESP_LOG_INFO              0x84

uint8_t Protocol_SendLogInfo(LogRuntimeInfo_t info);

#define CMD_CLEAR_LOG        0x05
#define CMD_RESP_CLEAR_LOG   0x85
#define PROTOCOL_LEN_CLEAR_LOG  0x05
#define PROTOCOL_LEN_RESP_CLEAR_LOG  0x02
#define FRAME_LEN_RESP_CLEAR_LOG     0x07
#define CLEAR_LOG_OK                 0x00
#define CLEAR_LOG_CONFIRM_ERR        0x01

uint8_t Protocol_SendClearLogResponse(uint8_t status);

#define CMD_GET_DOWNSTREAM_DATA          0x06
#define CMD_RESP_DOWNSTREAM_DATA         0x86

#define PROTOCOL_LEN_GET_DOWNSTREAM_DATA 0x01
#define PROTOCOL_LEN_RESP_DOWNSTREAM_DATA 0x0B

#define FRAME_LEN_RESP_DOWNSTREAM_DATA   16
#endif

