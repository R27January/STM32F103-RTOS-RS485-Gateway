#ifndef __CWS91_H
#define __CWS91_H
#include "stdint.h"

uint8_t CWS91_ParseTempHum(uint8_t *rx_buf, int16_t *temp,uint16_t *hum_x10);

#endif 

