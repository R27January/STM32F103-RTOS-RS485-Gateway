#include "bsp_rs485_down.h"
#include "usart.h"
void BSP_RS485_Down_SetRxMode(void)
{
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
}
void BSP_RS485_Down_SetTxMode(void)
{
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
}

void BSP_RS485_Down_SendBytes(uint8_t *data,uint16_t len)
{
    BSP_RS485_Down_SetTxMode();

    HAL_UART_Transmit(&huart2,data,len,100);

    BSP_RS485_Down_SetRxMode();
}
