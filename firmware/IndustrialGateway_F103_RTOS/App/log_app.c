#include "log_app.h"
#include "bsp_w25q64.h"
#include "string.h"
#include "protocol.h"
#include "stddef.h"
uint32_t Log_CalcAddr(uint32_t seq)
{
    uint32_t addr = LOG_START_ADDR + seq * sizeof(LogData_t);
    return addr;
}
static uint16_t Log_CalcRecordCRC(const LogData_t *log)//算record_crc
{
    return Protocol_CalcCRC16((uint8_t*)log,offsetof(LogData_t,record_crc)); //offsetof(A,B)   算出B到A开头有多少字节
}
uint8_t Log_IsValid(const LogData_t *log) //判断日志是否有效函数
{
    uint16_t crc_calc;
    if (log == NULL)
    {
        return 0;
    }
    if (log->magic != LOG_MAGIC)
    {
        return 0;
    }
    
    crc_calc = Log_CalcRecordCRC(log);

    if (crc_calc != log->record_crc)
    {
        return 0;
    }

    return 1;
    
}
void Log_SaveOne(LogData_t *log) 
{

    LogData_t record;
    
    if (log == 0)
    {
        return;
    }

    if (log->seq >= LOG_MAX_COUNT)
    {
        return;
    }
    uint32_t addr = Log_CalcAddr(log->seq);
    memset(&record, 0, sizeof(LogData_t));

    record.magic = LOG_MAGIC;
    record.seq = log->seq;
    record.source = log->source;
    record.target_id = log->target_id;
    record.type = log->type;
    record.err_code = log->err_code;
    record.cnt = log->cnt;
    record.adc = log->adc;   

    record.record_crc = Log_CalcRecordCRC(&record);

    BSP_W25Q64_PageProgram(addr, (uint8_t *)&record, sizeof(LogData_t));
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
void Log_RebuildRuntimeInfo(void)
{
    uint32_t i;
    uint32_t addr;
    LogData_t read_log;

    LogRuntimeInfo_Init();

    for (i = 0; i < LOG_MAX_COUNT; i++)
    {
        addr = Log_CalcAddr(i);
        BSP_W25Q64_ReadData(addr,(uint8_t*)&read_log,sizeof(LogData_t));

        if (Log_IsValid(&read_log) != 1)
        {
            break;
        }
        if (read_log.seq != i)
        {
            break;
        }
        
        LogRuntimeInfo_OnLogAccepted(read_log.seq);
    }
    
}
