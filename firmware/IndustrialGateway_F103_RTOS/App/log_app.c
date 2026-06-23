#include "log_app.h"
#include "bsp_w25q64.h"

uint32_t Log_CalcAddr(uint32_t seq)
{
    uint32_t addr = LOG_START_ADDR + seq * sizeof(LogData_t);
    return addr;
}

void Log_SaveOne(LogData_t *log)
{
    uint32_t addr = Log_CalcAddr(log->seq);
    BSP_W25Q64_PageProgram(addr, (uint8_t *)log, sizeof(LogData_t));
}