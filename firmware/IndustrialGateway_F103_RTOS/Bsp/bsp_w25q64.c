#include "bsp_w25q64.h"
#include "spi.h"
#include "main.h"

#define W25Q64_CMD_READ_ID            0x9F
#define W25Q64_CMD_WRITE_ENABLE       0x06
#define W25Q64_CMD_READ_STATUS_REG1   0x05
#define W25Q64_CMD_SECTOR_ERASE       0x20
#define W25Q64_CMD_READ_DATA          0x03
#define W25Q64_CMD_PAGE_PROGRAM       0x02


#define PROTOCOL_LEN_GET_LOG   0x05
#define FRAME_LEN_GET_LOG      10

#define W25Q64_PAGE_SIZE              256
#define W25Q64_SECTOR_SIZE            4096

void BSP_W25Q64_CS_Low(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin,GPIO_PIN_RESET);
}
void BSP_W25Q64_CS_High (void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin,GPIO_PIN_SET);
}
void BSP_W25Q64_ReadID(uint8_t *id)
{
    uint8_t cmd = W25Q64_CMD_READ_ID;

    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);
    HAL_SPI_Receive(&hspi1,id,3,100);

    BSP_W25Q64_CS_High();   
}
void BSP_W25Q64_WriteEnable(void)
{
    uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;
    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);

    BSP_W25Q64_CS_High();
}
uint8_t BSP_W25Q64_ReadStatusReg1(void)
{
    uint8_t cmd =W25Q64_CMD_READ_STATUS_REG1;
    uint8_t status = 0;
    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);

    HAL_SPI_Receive(&hspi1 ,&status, 1 ,100);

    BSP_W25Q64_CS_High();

    return status;

}
void BSP_W25Q64_WaitBusy(void)
{
    while (BSP_W25Q64_ReadStatusReg1() &0X01  )
    {
        
    }
    
}
void BSP_W25Q64_SectorErase(uint32_t addr)
{
    uint8_t cmd = W25Q64_CMD_SECTOR_ERASE;
    uint8_t addr_buf[3];;

    addr_buf[0] = addr >> 16;
    addr_buf[1] = addr >> 8;
    addr_buf[2] = addr;
    BSP_W25Q64_WriteEnable();
    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);
    HAL_SPI_Transmit(&hspi1,addr_buf,3,100);

    BSP_W25Q64_CS_High();
    BSP_W25Q64_WaitBusy();


}
void BSP_W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd = W25Q64_CMD_READ_DATA;
    uint8_t addr_buf[3] = {0};
    addr_buf[0] = addr >> 16;
    addr_buf[1] = addr >> 8;
    addr_buf[2] = addr;
    
    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);
    HAL_SPI_Transmit(&hspi1,addr_buf,3,100);
    HAL_SPI_Receive(&hspi1,buf,len,100);

    BSP_W25Q64_CS_High();

}
void BSP_W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd = W25Q64_CMD_PAGE_PROGRAM;
    uint8_t addr_buf[3] = {0};
    addr_buf[0] = addr >> 16;
    addr_buf[1] = addr >> 8;
    addr_buf[2] = addr;

    BSP_W25Q64_WriteEnable();

    BSP_W25Q64_CS_Low();

    HAL_SPI_Transmit(&hspi1,&cmd,1,100);
    HAL_SPI_Transmit(&hspi1,addr_buf,3,100);
    HAL_SPI_Transmit(&hspi1,buf,len,100);

    BSP_W25Q64_CS_High();

    BSP_W25Q64_WaitBusy();
}