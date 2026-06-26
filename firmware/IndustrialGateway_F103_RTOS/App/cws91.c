#include "cws91.h" 
#include "string.h"
#include "modbus_rtu.h"
uint8_t CWS91_ParseTempHum(uint8_t *rx_buf, int16_t *temp,uint16_t *hum_x10)
{
    if (rx_buf == NULL || temp == 0 || hum_x10 == 0)
    {
        return MODBUS_ERR_LEN;
    }
    uint16_t temp_raw;
    uint16_t hum_raw;

    temp_raw = ((uint16_t) rx_buf[3] << 8) | rx_buf[4];
    hum_raw = ((uint16_t)rx_buf[5]<<8) | rx_buf[6];
    
    *temp = (int16_t)temp_raw - 400;
    *hum_x10 = hum_raw ;

    return MODBUS_OK;

}