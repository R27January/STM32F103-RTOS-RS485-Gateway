#include "log_app.h"
#include "bsp_w25q64.h"
#include "string.h"
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

static LogRuntimeInfo_t s_log_runtime_info;

void LogRuntimeInfo_Init(void)
{
    s_log_runtime_info.latest_seq = 0;
    s_log_runtime_info.log_count = 0;
    s_log_runtime_info.log_full = 0;
    s_log_runtime_info.max_count = LOG_MAX_COUNT;
    s_log_runtime_info.next_seq = 0;
    s_log_runtime_info.record_size = sizeof(LogData_t);
    s_log_runtime_info.reserved = 0;
}

uint32_t LogRuntimeInfo_GetNextSeq(void)
{
    return s_log_runtime_info.next_seq;
}

void LogRuntimeInfo_OnLogAccepted(uint32_t seq)
{
    s_log_runtime_info.latest_seq  = seq;
    s_log_runtime_info.next_seq = seq + 1;

    if(s_log_runtime_info.log_count < s_log_runtime_info.max_count)
    {
        s_log_runtime_info.log_count++;
    }
    if (s_log_runtime_info.log_count >= s_log_runtime_info.max_count)
    {
        s_log_runtime_info.log_full = 1;
    }
}
void LogRuntimeInfo_GetSnapshot(LogRuntimeInfo_t *info)
{
    if (info == NULL)
    {
        return;
    }
    *info = s_log_runtime_info;
}