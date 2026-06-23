#ifndef __BSP_W25Q64_H
#define __BSP_W25Q64_H

#include "stdint.h"
void BSP_W25Q64_CS_Low(void);
void BSP_W25Q64_CS_High (void);
void BSP_W25Q64_ReadID(uint8_t *id);
void BSP_W25Q64_WriteEnable(void);
uint8_t BSP_W25Q64_ReadStatusReg1(void);
void BSP_W25Q64_WaitBusy(void);
void BSP_W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len);
void BSP_W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len);
void BSP_W25Q64_SectorErase(uint32_t addr);
#endif

