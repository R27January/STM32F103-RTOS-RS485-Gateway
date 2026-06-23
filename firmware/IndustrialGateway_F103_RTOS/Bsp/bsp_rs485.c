#include "bsp_rs485.h"
#include "usart.h"

void BSP_RS485_SetRxMode(void)
{
    HAL_GPIO_WritePin(RS485_DIR_GPIO_Port,RS485_DIR_Pin,GPIO_PIN_RESET);
}
void BSP_RS485_SetTxMode(void)
{
    HAL_GPIO_WritePin(RS485_DIR_GPIO_Port,RS485_DIR_Pin,GPIO_PIN_SET);
}
void BSP_RS485_SendBytes(uint8_t *data,uint16_t len)
{
    BSP_RS485_SetTxMode();
    HAL_UART_Transmit(&huart1,data,len,100);
    //while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) == RESET ){}
    
    BSP_RS485_SetRxMode();
}

