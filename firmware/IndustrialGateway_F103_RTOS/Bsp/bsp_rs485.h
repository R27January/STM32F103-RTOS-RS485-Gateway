#ifndef __BSP_RS485_H
#define __BSP_RS485_H

#include "main.h"
#include "stdint.h"

void BSP_RS485_SetRxMode(void);
void BSP_RS485_SetTxMode(void);
void BSP_RS485_SendBytes(uint8_t *data,uint16_t len);



#endif

