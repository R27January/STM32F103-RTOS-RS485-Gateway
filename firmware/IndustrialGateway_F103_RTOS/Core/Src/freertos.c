/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_oled.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "adc.h"
#include "queue.h"
#include "semphr.h"
#include "bsp_rs485.h"
#include "log_app.h"
#include "bsp_w25q64.h"
#include "event_groups.h"
#include "timers.h" 
#include "protocol.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct 
{
  uint32_t adc_value;
  uint32_t sample_count;
}SensorData_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_DMA_RX_BUF_SIZE    64


/* Protocol frame head */
#define PROTOCOL_HEAD1              0xAA  /* 帧头第1字节，固定为0xAA */
#define PROTOCOL_HEAD2              0x55  /* 帧头第2字节，固定为0x55 */

/* Protocol command */
#define CMD_GET_SENSOR              0x01  /* 主机查询传感器数据的请求命令 */
#define CMD_RESP_SENSOR             0x81  /* 设备返回传感器数据的响应命令 */
#define CMD_RESP_ERROR              0xE0  /* 设备返回协议错误的响应命令 */

#define CMD_GET_LOG                 0x02
#define CMD_RESP_LOG                0x82
/* Protocol LEN field: length of CMD + DATA */
#define PROTOCOL_LEN_GET_SENSOR     0x01  /* 请求帧LEN字段：CMD+DATA共1字节，只有CMD */
#define PROTOCOL_LEN_RESP_SENSOR    0x05  /* 正常响应LEN字段：CMD+DATA共5字节 */
#define PROTOCOL_LEN_RESP_ERROR     0x02  /* 错误响应LEN字段：CMD+DATA共2字节 */
#define PROTOCOL_LEN_RESP_LOG       0x0B
#define PROTOCOL_CRC_LEN            2     /* CRC16字段长度：CRC_L和CRC_H共2字节 */
#define PROTOCOL_LEN_GET_LOG        0x05
#define PROTOCOL_LEN_GET_DEVICE_INFO    0x01

/* Whole frame length */
#define FRAME_LEN_GET_SENSOR        6   /* 请求整帧长度：帧头2+LEN1+CMD1+CRC2 */
#define FRAME_LEN_RESP_SENSOR       10    /* 正常响应整帧长度：帧头2+LEN1+CMD/DATA5+CRC2 */
#define FRAME_LEN_RESP_ERROR        7     /* 错误响应整帧长度：帧头2+LEN1+CMD/DATA2+CRC2 */
#define FRAME_LEN_GET_LOG           10
#define FRAME_LEN_RESP_LOG          16

/* Error code */
#define ERR_CODE_HEAD               0x01  /* 错误码：帧头错误 */
#define ERR_CODE_LEN                0x02  /* 错误码：LEN字段错误 */
#define ERR_CODE_CMD                0x03  /* 错误码：CMD命令错误 */
#define ERR_CODE_CHECK              0x04  /* 错误码：CRC16校验错误 */

/* Protocol check result */
#define PROTOCOL_OK                 0x00  /* 协议检查通过，无错误 */

/*Event */
#define EVENT_COMM_OK               (1 << 0)
#define EVENT_PROTOCOL_ERR          (1 << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t DisplayDataQueue;
QueueHandle_t CommDataQueue;
QueueHandle_t LogQueue;

TaskHandle_t CommTaskHandle_RTOS = NULL;

SemaphoreHandle_t OLEDMutex;
SemaphoreHandle_t UARTMutex;

EventGroupHandle_t SystemEventGroup;

TimerHandle_t CommTimeoutTimer;

uint32_t sensor_count = 0;
uint32_t display_count = 0;
uint32_t comm_count = 0;
uint32_t sensor_value = 0;
uint32_t test_count = 0;

uint8_t uart_dma_rx_buf[UART_DMA_RX_BUF_SIZE];
volatile uint16_t uart_rx_len = 0;

SensorData_t sensor_data = {0,0};
SensorData_t display_data = {0,0};
SensorData_t comm_data = {0,0};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CommTask */
osThreadId_t CommTaskHandle;
const osThreadAttr_t CommTask_attributes = {
  .name = "CommTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LogTask */
osThreadId_t LogTaskHandle;
const osThreadAttr_t LogTask_attributes = {
  .name = "LogTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Protocol_SendError(uint8_t err_code);
void Protocol_SendSensorResponse(SensorData_t sensor_data);
uint8_t Protocol_CheckRequest(uint8_t *rx_buf);
uint16_t Protocol_CalcCRC16(uint8_t *data, uint16_t len);
static void CommTask_SendQueryOkLog(SensorData_t sensor);
void Protocol_SendLogResponse(LogData_t read_log);
void Comm_RxStartDMA(void);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
static const char* SystemStatus_GetText(EventBits_t bits);
void CommTimeoutTimerCallback(TimerHandle_t xTimer);
//uint8_t Protocol_CalcXor(uint8_t* data,uint8_t len);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartDisplayTask(void *argument);
void StartCommTask(void *argument);
void StartLogTask(void *argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  OLEDMutex = xSemaphoreCreateMutex();
  UARTMutex = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  CommTimeoutTimer = xTimerCreate("CommTimeout",pdMS_TO_TICKS(3000),pdFALSE,NULL,CommTimeoutTimerCallback);
  if(CommTimeoutTimer == NULL)
  {
    
  }
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  DisplayDataQueue = xQueueCreate(5,sizeof(SensorData_t));
  CommDataQueue = xQueueCreate(5,sizeof(SensorData_t));
  LogQueue = xQueueCreate(5,sizeof(LogData_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* creation of CommTask */
  CommTaskHandle = osThreadNew(StartCommTask, NULL, &CommTask_attributes);
  /* creation of LogTask */
  LogTaskHandle = osThreadNew(StartLogTask, NULL, &LogTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    CommTaskHandle_RTOS = (TaskHandle_t)CommTaskHandle;
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  SystemEventGroup = xEventGroupCreate();
  if (SystemEventGroup == NULL)
  {
    
  }
  
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    test_count++;
    if(xSemaphoreTake(OLEDMutex,pdMS_TO_TICKS(100)) == pdPASS)
    {
      //BSP_OLED_ShowNum(0,6,test_count,5);
      xSemaphoreGive(OLEDMutex);
    }
    if (xSemaphoreTake(UARTMutex,pdMS_TO_TICKS(100)) ==pdPASS)
    {
      //HAL_UART_Transmit(&huart1,(uint8_t*)"TEST\r\n",6,100);
      xSemaphoreGive(UARTMutex);
    }
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  for(;;)
  {   
    sensor_count++;
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1,100) == HAL_OK)
    {
      sensor_data.sample_count = sensor_count;
      sensor_value = HAL_ADC_GetValue(&hadc1);
      sensor_data.adc_value = sensor_value;
      xQueueSend(DisplayDataQueue,&sensor_data,0);
      xQueueSend(CommDataQueue,&sensor_data,0);
    }
    osDelay(500);
  }
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
  if(xSemaphoreTake(OLEDMutex,pdMS_TO_TICKS(100)) == pdPASS)
  {
    BSP_OLED_ShowString(0,0,"V1 RUN");  
    BSP_OLED_ShowString(0, 2, "CNT:");    
    BSP_OLED_ShowString(0, 4, "ADC:");
    BSP_OLED_ShowString(0, 6, "STAT:");
    xSemaphoreGive(OLEDMutex);

  }
  for(;;)
  {
    EventBits_t bits;
    const char* status_text;

    bits = xEventGroupGetBits(SystemEventGroup);
    status_text = SystemStatus_GetText(bits);

    if(xQueueReceive(DisplayDataQueue,&display_data,portMAX_DELAY) == pdPASS)
    {
      display_count++;
      if(xSemaphoreTake(OLEDMutex,pdMS_TO_TICKS(100)) == pdPASS)
      {

        BSP_OLED_ShowNum(6,2,display_data.sample_count,5);    
        BSP_OLED_ShowNum(6,4,display_data.adc_value,5);
        BSP_OLED_ShowString(6,6,status_text);
        xSemaphoreGive(OLEDMutex);
      }
    }
    osDelay(500);
  }
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartCommTask */
/**
* @brief Function implementing the CommTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCommTask */
void StartCommTask(void *argument)
{
  /* USER CODE BEGIN StartCommTask */
  /* Infinite loop */
  uint8_t rx_buf[FRAME_LEN_GET_LOG]= {0};
  SensorData_t latest_comm_data = {0,0};
  Comm_RxStartDMA();
  for(;;)
  {
    comm_count++;
    
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

    while (xQueueReceive(CommDataQueue,&comm_data,0) == pdPASS)
    {
      latest_comm_data = comm_data;
    }
    
    if (uart_rx_len < 6 || uart_rx_len > FRAME_LEN_GET_LOG)
    {
      Comm_RxStartDMA();
      continue;
    }
    memset(rx_buf, 0, sizeof(rx_buf));
    memcpy(rx_buf,uart_dma_rx_buf,uart_rx_len);

    uint8_t len = rx_buf[2];
    uint16_t frame_len = len + 5;

    if(uart_rx_len != frame_len)
    {
      Comm_RxStartDMA();
      continue;
    }        
          if(xSemaphoreTake(UARTMutex,pdMS_TO_TICKS(100)) == pdPASS)
          {
            uint8_t protocol_ret = Protocol_CheckRequest(rx_buf);

            if(protocol_ret == PROTOCOL_OK)
            {
              xEventGroupSetBits(SystemEventGroup,EVENT_COMM_OK);
              xEventGroupClearBits(SystemEventGroup,EVENT_PROTOCOL_ERR);

              xTimerReset(CommTimeoutTimer,0);

              if(rx_buf[3] == CMD_GET_SENSOR)
              {
                Protocol_SendSensorResponse(latest_comm_data);
                CommTask_SendQueryOkLog(latest_comm_data);
              }
              else if (rx_buf[3] == CMD_GET_LOG)
              {
                uint32_t query_seq = 0;
                uint32_t log_addr;
                query_seq = ((uint32_t)rx_buf[4] << 24) |
                            ((uint32_t)rx_buf[5] << 16) |
                            ((uint32_t)rx_buf[6] << 8)  |
                            ((uint32_t)rx_buf[7]);
                log_addr = Log_CalcAddr(query_seq);

                LogData_t read_log;

                BSP_W25Q64_ReadData(log_addr,(uint8_t*)&read_log,sizeof(LogData_t));

                Protocol_SendLogResponse(read_log);

              }
              else if (rx_buf[3] == CMD_GET_DEVICE_INFO)
              {
                Protocol_SendDeviceInfo();
              }
              
            }
            else
            {
              xEventGroupSetBits(SystemEventGroup,EVENT_PROTOCOL_ERR);
              xEventGroupClearBits(SystemEventGroup,EVENT_COMM_OK);


              Protocol_SendError(protocol_ret);
            }
            xSemaphoreGive(UARTMutex);
          }
            Comm_RxStartDMA();

    osDelay(10);
  }
  /* USER CODE END StartCommTask */
}

/* USER CODE BEGIN Header_StartLogTask */
/**
* @brief Function implementing the LogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLogTask */
void StartLogTask(void *argument)
{
  /* USER CODE BEGIN StartLogTask */
  /* Infinite loop */
  LogData_t log;
  for(;;)
  {
    while (xQueueReceive(LogQueue,&log,portMAX_DELAY) == pdPASS)
    {
      Log_SaveOne(&log);
    }
    
    osDelay(1);
  }
  /* USER CODE END StartLogTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
uint8_t Protocol_CheckRequest(uint8_t *rx_buf)
{ 
  uint16_t crc_calc;
  uint16_t crc_recv;

  if(rx_buf[0] != PROTOCOL_HEAD1 || rx_buf[1] != PROTOCOL_HEAD2)
  {
    return ERR_CODE_HEAD;
  }
  if (rx_buf[3] == CMD_GET_SENSOR)
  {
    if (rx_buf[2] != PROTOCOL_LEN_GET_SENSOR)
    {
      return ERR_CODE_LEN;
    }
  }
  else if (rx_buf[3] == CMD_GET_LOG)
  {
    if (rx_buf[2] != PROTOCOL_LEN_GET_LOG)
    {
      return ERR_CODE_LEN;
    }
  }
  else if (rx_buf[3] == CMD_GET_DEVICE_INFO)
  {
    if (rx_buf[2] != PROTOCOL_LEN_GET_DEVICE_INFO)
    {
      return ERR_CODE_LEN;
    }
    
  }
  
  else
  {
    return ERR_CODE_CMD;
  }
  crc_calc = Protocol_CalcCRC16(&rx_buf[2], rx_buf[2] + 1);
  crc_recv = (uint16_t)rx_buf[3 + rx_buf[2]] | ((uint16_t)rx_buf[4+ rx_buf[2]] << 8);

  if (crc_calc != crc_recv)
  {
    return ERR_CODE_CHECK;
  }
  
  return PROTOCOL_OK;  
}
void Protocol_SendSensorResponse(SensorData_t sensor_data)
{
  uint8_t tx_frame[FRAME_LEN_RESP_SENSOR];
  uint16_t cnt;
  uint16_t adc;
  uint16_t crc;
  cnt = sensor_data.sample_count;
  adc = sensor_data.adc_value;
  
  tx_frame[0] = PROTOCOL_HEAD1;
  tx_frame[1] = PROTOCOL_HEAD2;
  tx_frame[2] = PROTOCOL_LEN_RESP_SENSOR;
  tx_frame[3] = CMD_RESP_SENSOR;

  tx_frame[4] = cnt>>8;
  tx_frame[5] = cnt & 0xFF;
  tx_frame[6] = adc>>8;
  tx_frame[7] = adc & 0xFF;
  crc = Protocol_CalcCRC16(&tx_frame[2],PROTOCOL_LEN_RESP_SENSOR + 1);    
  tx_frame[8] = crc & 0xFF;
  tx_frame[9] = crc>>8;
  BSP_RS485_SendBytes(tx_frame,FRAME_LEN_RESP_SENSOR);  
}
void Protocol_SendError(uint8_t err_code)
{
  uint8_t err_frame[FRAME_LEN_RESP_ERROR];
  uint16_t crc;

  err_frame[0] = PROTOCOL_HEAD1;
  err_frame[1] = PROTOCOL_HEAD2;
  err_frame[2] = PROTOCOL_LEN_RESP_ERROR;
  err_frame[3] = CMD_RESP_ERROR;
  err_frame[4] = err_code;
  crc = Protocol_CalcCRC16(&err_frame[2],PROTOCOL_LEN_RESP_ERROR + 1);
  err_frame[5] = crc & 0xFF;
  err_frame[6] = crc>>8;

  BSP_RS485_SendBytes(err_frame,FRAME_LEN_RESP_ERROR);

}
/*
uint8_t Protocol_CalcXor(uint8_t*data,uint8_t len)
{
  uint8_t check = 0;
  uint8_t i;
  for(i = 0; i < len ; i++)
  {
    check ^= data[i];
  }
  return check;
}
*/

/*
uint16_t Protocol_CalcCRC16(uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFF;
  uint16_t i;
  uint8_t j;
  for(i = 0; i < len ; i++)
  {
    crc ^= data[i];
    for(j = 0; j < 8 ; j++)
    {
      if(crc & 0x0001)
      {
        crc = (crc>>1) ^ 0xA001;
      }
      else
      {
        crc = (crc>>1);
      }
    }
  }
  return crc;
}
*/

static void CommTask_SendQueryOkLog(SensorData_t sensor)
{
  LogData_t log;
  static uint32_t log_seq;
  log.seq = log_seq;
  log.type = LOG_TYPE_QUERY_OK;
  log.cnt = sensor.sample_count;
  log.adc = sensor.adc_value;
  log.err_code = 0;

  if (xQueueSend(LogQueue, &log, 0) == pdPASS)
  {
    log_seq++;
  }
}
void Protocol_SendLogResponse(LogData_t read_log)
{
  uint8_t tx_buf[FRAME_LEN_RESP_LOG];
  uint16_t crc;

  tx_buf[0] = PROTOCOL_HEAD1;
  tx_buf[1] = PROTOCOL_HEAD2;
  tx_buf[2] = PROTOCOL_LEN_RESP_LOG;
  tx_buf[3] = CMD_RESP_LOG;

  tx_buf[4] = (uint8_t)(read_log.seq >> 24);
  tx_buf[5] = (uint8_t)(read_log.seq >> 16);
  tx_buf[6] = (uint8_t)(read_log.seq >> 8);
  tx_buf[7] = (uint8_t)(read_log.seq);

  tx_buf[8] = (uint8_t)read_log.type;

  tx_buf[9]  = (uint8_t)(read_log.cnt >> 8);
  tx_buf[10] = (uint8_t)(read_log.cnt);

  tx_buf[11] = (uint8_t)(read_log.adc >> 8);
  tx_buf[12] = (uint8_t)(read_log.adc);

  tx_buf[13] = (uint8_t)read_log.err_code;

  crc = Protocol_CalcCRC16(&tx_buf[2], tx_buf[2] + 1);

  tx_buf[14] = (uint8_t)crc;
  tx_buf[15] = (uint8_t)(crc >> 8);

  BSP_RS485_SendBytes(tx_buf,FRAME_LEN_RESP_LOG);
}

void Comm_RxStartDMA(void)
{
  BSP_RS485_SetRxMode();
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_dma_rx_buf,UART_DMA_RX_BUF_SIZE);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx,DMA_IT_HT);
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if(huart->Instance == USART1)
  {
    uart_rx_len = Size;

    if (CommTaskHandle_RTOS != NULL)
    {
      vTaskNotifyGiveFromISR(CommTaskHandle_RTOS,&xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
  }
}
static const char* SystemStatus_GetText(EventBits_t bits)
{
  if (bits & EVENT_COMM_OK)
  {
    return "COMM OK  ";
  }
  if (bits & EVENT_PROTOCOL_ERR)
  {
    return "PROTO ERR ";
  }
  
  return "IDLE     ";
}
void CommTimeoutTimerCallback(TimerHandle_t xTimer)
{
  xEventGroupClearBits(SystemEventGroup,EVENT_COMM_OK);
}
/* USER CODE END Application */

