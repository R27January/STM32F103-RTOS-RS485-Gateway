#include "modbus_rtu.h"


uint16_t Modbus_CalcCRC16(uint8_t *buf,uint16_t len)
{   
    uint16_t crc;
    crc = Protocol_CalcCRC16(buf,len);

    return crc;
}
uint16_t Modbus_BuildReadHoldingRegsReq(uint8_t slave_id,uint16_t start_addr,uint16_t reg_num,uint8_t *tx_buf)
{
    uint16_t crc;

    if (tx_buf == 0)
    {
        return 0;    
    }
    
    tx_buf[0] = slave_id;
    tx_buf[1] = 0x03;

    tx_buf[2] = (uint8_t)(start_addr >> 8);
    tx_buf[3] = (uint8_t)(start_addr);

    tx_buf[4] = (uint8_t)(reg_num >> 8);
    tx_buf[5] = (uint8_t)(reg_num);

    crc = Modbus_CalcCRC16(tx_buf,6);

    tx_buf[6] = (uint8_t)(crc & 0xFF);
    tx_buf[7] = (uint8_t)(crc >> 8);

    return 8;

}
uint8_t Modbus_CheckReadHoldingRegsResp(uint8_t* rx_buf , uint16_t len)
{
    uint16_t crc_recv;
    uint16_t crc_clac;

    if (rx_buf == 0)
    {
        return MODBUS_ERR_LEN;
    }
    
    if(len != 9)
    {
        return MODBUS_ERR_LEN;
    }
    if (rx_buf[0] != 0x01)
    {
        return MODBUS_ERR_ADDR;
    }
    if (rx_buf[1] != 0x03)
    {
        return MODBUS_ERR_FUNC;
    }
    if (rx_buf[2] != 0x04)
    {
        return MODBUS_ERR_BYTES;
    }
    
    crc_clac = Modbus_CalcCRC16(rx_buf,len - 2);
    crc_recv = (uint16_t)(rx_buf[8] << 8) | (uint16_t)(rx_buf[7]);
    if (crc_clac != crc_recv)
    {
        return MODBUS_ERR_CRC;
    }
       
    return MODBUS_OK;
}