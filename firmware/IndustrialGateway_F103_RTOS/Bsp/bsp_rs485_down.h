#ifndef __BSP_RS485_DOWN_H
#define __BSP_RS485_DOWN_h
#include "stdint.h"

void BSP_RS485_Down_SetRxMode(void);
void BSP_RS485_Down_SetTxMode(void);
void BSP_RS485_Down_SendBytes(uint8_t *data,uint16_t len);


#endif

