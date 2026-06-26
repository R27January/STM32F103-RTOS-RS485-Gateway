#include "downstream_data.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

DownstreamData_t g_downstream_cache;

void DownstreamData_Init(void)
{
  memset(&g_downstream_cache, 0, sizeof(g_downstream_cache));

  g_downstream_cache.target_id = DOWNSTREAM_CWS91_ID;
  g_downstream_cache.valid = 0;
  g_downstream_cache.online = 0;
  g_downstream_cache.last_err = DOWN_ERR_NONE;
}

void DownstreamData_OnPollStart(uint8_t target_id)
{
  taskENTER_CRITICAL();

  g_downstream_cache.target_id = target_id;
  g_downstream_cache.poll_count++;

  taskEXIT_CRITICAL();
}

void DownstreamData_UpdateSuccess(uint8_t target_id, int16_t temp_x10, uint16_t hum_x10)
{
  taskENTER_CRITICAL();

  g_downstream_cache.target_id = target_id;
  g_downstream_cache.valid = 1;
  g_downstream_cache.online = 1;
  g_downstream_cache.last_err = DOWN_ERR_NONE;

  g_downstream_cache.temp_x10 = temp_x10;
  g_downstream_cache.hum_x10 = hum_x10;

  g_downstream_cache.update_count++;
  g_downstream_cache.success_count++;
  g_downstream_cache.fail_streak = 0;

  taskEXIT_CRITICAL();
}

void DownstreamData_UpdateFail(uint8_t target_id, uint8_t err_code)
{
  taskENTER_CRITICAL();

  g_downstream_cache.target_id = target_id;
  g_downstream_cache.last_err = err_code;

  if (err_code == DOWN_ERR_TIMEOUT)
  {
    g_downstream_cache.timeout_count++;
  }
  else if (err_code == DOWN_ERR_CRC)
  {
    g_downstream_cache.crc_err_count++;
  }
  else if (err_code == DOWN_ERR_PARSE)
  {
    g_downstream_cache.parse_err_count++;
  }

  if (g_downstream_cache.fail_streak < 255)
  {
    g_downstream_cache.fail_streak++;
  }

  if (g_downstream_cache.fail_streak >= DOWNSTREAM_OFFLINE_THRESHOLD)
  {
    g_downstream_cache.online = 0;
  }

  taskEXIT_CRITICAL();
}

void DownstreamData_Update(uint8_t target_id, int16_t temp_x10, uint16_t hum_x10)
{
  DownstreamData_UpdateSuccess(target_id, temp_x10, hum_x10);
}

void DownstreamData_GetSnapshot(DownstreamData_t *out)
{
  if (out == 0)
  {
    return;
  }

  taskENTER_CRITICAL();
  *out = g_downstream_cache;
  taskEXIT_CRITICAL();
}