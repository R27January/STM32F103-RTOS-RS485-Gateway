#include "protocol.h"
#include"string.h"
#include "stdint.h"
#include "bsp_rs485.h"
#include "log_app.h"
uint16_t Protocol_CalcCRC16(uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFF;
  uint16_t i;
  uint8_t j;
  for(i = 0; i < len ; i++)
  {
    crc ^= data[i];
    for(j = 0; j < 8 ; j++)
    {
      if(crc & 0x0001)
      {
        crc = (crc>>1) ^ 0xA001;
      }
      else
      {
        crc = (crc>>1);
      }
    }
  }
  return crc;
}

uint8_t Protocol_SendDeviceInfo(void)
{
    uint8_t tx_buf[16] = {0};
    uint16_t crc;

    tx_buf[0] = 0xAA;
    tx_buf[1] = 0X55;
    tx_buf[2] = 0x0B;
    tx_buf[3] = CMD_RESP_DEVICE_INFO  ;

    tx_buf[4] = (uint8_t)(GATEWAY_ID >> 8);
    tx_buf[5] = (uint8_t)(GATEWAY_ID);

    tx_buf[6] = (uint8_t)(FW_VERSION >> 8);
    tx_buf[7] = (uint8_t)(FW_VERSION);

    tx_buf[8] = (uint8_t)(PROTOCOL_VERSION >> 8);
    tx_buf[9] = (uint8_t)(PROTOCOL_VERSION);

    tx_buf[10] = (uint8_t)(FEATURE_FLAGS >> 8);
    tx_buf[11] = (uint8_t)(FEATURE_FLAGS);

    tx_buf[12] = (uint8_t)(DEVICE_STATUS_OK >> 8);
    tx_buf[13] = (uint8_t)(DEVICE_STATUS_OK);

    crc = Protocol_CalcCRC16(&tx_buf[2],tx_buf[2] + 1);

    tx_buf[14] = crc & 0xFF;
    tx_buf[15] = crc >> 8;

    BSP_RS485_SendBytes(tx_buf, 16);
    
    return 1;
}

uint8_t Protocol_SendLogInfo(LogRuntimeInfo_t info)
{
  uint8_t tx_buf[FRAME_LEN_RESP_LOG_INFO] = {0};
  uint16_t crc;

  tx_buf[0] = 0xAA;
  tx_buf[1] = 0x55;
  tx_buf[2] = 0x10;
  tx_buf[3] = CMD_RESP_LOG_INFO;

  tx_buf[4] = (uint8_t)(info.log_count >> 24);
  tx_buf[5] = (uint8_t)(info.log_count >> 16);
  tx_buf[6] = (uint8_t)(info.log_count >> 8);
  tx_buf[7] = (uint8_t)(info.log_count );

  tx_buf[8] = (uint8_t)(info.latest_seq >> 24);
  tx_buf[9] = (uint8_t)(info.latest_seq >> 16);
  tx_buf[10] = (uint8_t)(info.latest_seq >> 8);
  tx_buf[11] = (uint8_t)(info.latest_seq);

  tx_buf[12] = (uint8_t)(info.max_count >> 24);
  tx_buf[13] = (uint8_t)(info.max_count >> 16);
  tx_buf[14] = (uint8_t)(info.max_count >> 8);
  tx_buf[15] = (uint8_t)(info.max_count );

  tx_buf[16] = (uint8_t)(info.record_size >> 8);
  tx_buf[17] = (uint8_t)(info.record_size);

  tx_buf[18] = info.log_full;

  crc = Protocol_CalcCRC16(&tx_buf[2],tx_buf[2] + 1);

  tx_buf[19] = (uint8_t)(crc & 0xFF);
  tx_buf[20] = (uint8_t)(crc >> 8);
  
  BSP_RS485_SendBytes(tx_buf,FRAME_LEN_RESP_LOG_INFO);

  return 1;
}
uint8_t Protocol_SendClearLogResponse(uint8_t status)
{
  uint8_t tx_buf[FRAME_LEN_RESP_CLEAR_LOG] = {0};
  uint16_t crc;

  tx_buf[0] = 0xAA;
  tx_buf[1] = 0x55;
  tx_buf[2] = PROTOCOL_LEN_RESP_CLEAR_LOG;
  tx_buf[3] = CMD_RESP_CLEAR_LOG;

  tx_buf[4] = status;

  crc = Protocol_CalcCRC16(&tx_buf[2], tx_buf[2] + 1);

  tx_buf[5] = (uint8_t)(crc &0xFF);
  tx_buf[6] = (uint8_t)(crc >> 8);

  BSP_RS485_SendBytes(tx_buf,FRAME_LEN_RESP_CLEAR_LOG);

  return 1;
}