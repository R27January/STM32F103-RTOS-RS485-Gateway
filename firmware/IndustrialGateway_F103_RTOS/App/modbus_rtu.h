#ifndef __MODBUS_RTU_H
#define __MODBUS_RUT_H
#include "stdint.h"
#include "protocol.h"


uint16_t Modbus_CalcCRC16(uint8_t *buf,uint16_t len);
uint16_t Modbus_BuildReadHoldingRegsReq(uint8_t slave_id,uint16_t start_addr,uint16_t reg_num,uint8_t *tx_buf);
uint8_t Modbus_CheckReadHoldingRegsResp(uint8_t* rx_buf , uint16_t len);
#define MODBUS_OK          0
#define MODBUS_ERR_LEN     1
#define MODBUS_ERR_ADDR    2
#define MODBUS_ERR_FUNC    3
#define MODBUS_ERR_BYTES   4
#define MODBUS_ERR_CRC     5


#endif
