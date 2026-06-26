#ifndef __DOWNSTREAM_DATA_H
#define __DOWNSTREAM_DATA_H

#include "stdint.h"

#define DOWNSTREAM_CWS91_ID           0x01

#define DOWNSTREAM_OFFLINE_THRESHOLD  3
#define DOWNSTREAM_RETRY_COUNT        2

#define DOWN_ERR_NONE                 0x00
#define DOWN_ERR_TIMEOUT              0x01
#define DOWN_ERR_CRC                  0x02
#define DOWN_ERR_PARSE                0x03

typedef struct
{
  uint8_t target_id;
  uint8_t valid;
  uint8_t online;
  uint8_t last_err;

  int16_t temp_x10;
  uint16_t hum_x10;

  uint32_t update_count;
  uint32_t poll_count;
  uint32_t success_count;
  uint32_t timeout_count;
  uint32_t crc_err_count;
  uint32_t parse_err_count;

  uint8_t fail_streak;
} DownstreamData_t;

extern DownstreamData_t g_downstream_cache;

void DownstreamData_Init(void);
void DownstreamData_OnPollStart(uint8_t target_id);
void DownstreamData_UpdateSuccess(uint8_t target_id, int16_t temp_x10, uint16_t hum_x10);
void DownstreamData_UpdateFail(uint8_t target_id, uint8_t err_code);
void DownstreamData_Update(uint8_t target_id, int16_t temp_x10, uint16_t hum_x10);
void DownstreamData_GetSnapshot(DownstreamData_t *out);

#endif