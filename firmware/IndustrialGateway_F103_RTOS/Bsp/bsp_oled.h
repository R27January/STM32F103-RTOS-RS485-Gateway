#ifndef __BSP_OLED_H
#define __BSP_OLED_H

#include "main.h"
#include "i2c.h"
#include <stdint.h>

/*
 * File: Bsp/bsp_oled.h
 * Target: SSD1306 I2C OLED, 128x64, common address 0x3C
 *
 * Coordinate rule:
 *   x: 0~20  character column, one character is 6 pixels wide
 *   y: 0~7   page row, one page is 8 pixels high
 *
 * Example:
 *   BSP_OLED_Init();
 *   BSP_OLED_Clear();
 *   BSP_OLED_ShowString(0, 0, "V1 RUN");
 */

#define BSP_OLED_WIDTH          128
#define BSP_OLED_HEIGHT         64
#define BSP_OLED_PAGE_NUM       8
#define BSP_OLED_I2C_ADDR       (0x3C << 1)
#define BSP_OLED_TIMEOUT        100

void BSP_OLED_Init(void);
void BSP_OLED_Clear(void);
void BSP_OLED_ClearLine(uint8_t y);

void BSP_OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void BSP_OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void BSP_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);

#endif
