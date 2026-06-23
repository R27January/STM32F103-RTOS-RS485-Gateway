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

uint16_t Protocol_CalcCRC16(uint8_t *data, uint16_t len);
uint8_t Protocol_SendDeviceInfo(void);

#endif

