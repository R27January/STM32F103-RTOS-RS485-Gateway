#ifndef __LOG_APP_H
#define __LOG_APP_H

#include "stdint.h"

#define LOG_START_ADDR   0x001000
#define LOG_MAX_COUNT    128

typedef enum
{
    LOG_TYPE_QUERY_OK = 1,
    LOG_TYPE_PROTOCOL_ERR ,
    LOG_TYPE_MAX
} LogType_t;
typedef struct 
{
    uint32_t seq;
    LogType_t type;
    uint16_t cnt;
    uint16_t adc;
    uint8_t err_code;
}LogData_t;
uint32_t Log_CalcAddr(uint32_t seq);
void Log_SaveOne(LogData_t *log);  

typedef struct 
{
    uint32_t log_count;
    uint32_t latest_seq;
    uint32_t next_seq;
    uint32_t max_count;
    uint16_t record_size;
    uint8_t log_full;
    uint8_t reserved;
}LogRuntimeInfo_t;
void LogRuntimeInfo_Init(void);
uint32_t LogRuntimeInfo_GetNextSeq(void);
void LogRuntimeInfo_OnLogAccepted(uint32_t seq);
void LogRuntimeInfo_GetSnapshot(LogRuntimeInfo_t *info);


#endif

