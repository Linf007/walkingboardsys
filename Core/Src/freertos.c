/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "battery.h"
#include "angle.h"
#include "gnss.h"
#include "LoRa.h"
#include "pull.h"
#include "main.h"
#include "event_groups.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern uint8_t AngleBuffer_old[66];
extern uint8_t GnssBuffer_old[100];
extern uint8_t CmdData_Rx[MAX_Num_of_Relay*Size_of_Relay];
extern uint16_t PullBuffer_old[adc_avg][adc_channel];
extern uint8_t BatteryBuffer_old[32];

EventGroupHandle_t Upstream_Event_Handle;
/* USER CODE END Variables */
osThreadId Uart4_LoRa_TaskHandle;
osThreadId Battery_Task_1Handle;
osThreadId Angle_Task_2Handle;
osThreadId GNSS_Task_3Handle;
osThreadId Pull_Task_0Handle;
osThreadId Upstream_TaskHandle;
osSemaphoreId myBinarySem_CommunicationHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_LoRa_FNC(void const * argument);
void Start_Battery_FNC(void const * argument);
void Start_Angle_FNC(void const * argument);
void Start_GNSS_FNC(void const * argument);
void Start_Pull_FNC(void const * argument);
void Start_upstream_FNC(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of myBinarySem_Communication */
  osSemaphoreDef(myBinarySem_Communication);
  myBinarySem_CommunicationHandle = osSemaphoreCreate(osSemaphore(myBinarySem_Communication), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	Upstream_Event_Handle=xEventGroupCreate();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Uart4_LoRa_Task */
  osThreadDef(Uart4_LoRa_Task, Start_LoRa_FNC, osPriorityNormal, 0, 300);
  Uart4_LoRa_TaskHandle = osThreadCreate(osThread(Uart4_LoRa_Task), NULL);

  /* definition and creation of Battery_Task_1 */
  osThreadDef(Battery_Task_1, Start_Battery_FNC, osPriorityLow, 0, 128);
  Battery_Task_1Handle = osThreadCreate(osThread(Battery_Task_1), NULL);

  /* definition and creation of Angle_Task_2 */
  osThreadDef(Angle_Task_2, Start_Angle_FNC, osPriorityLow, 0, 128);
  Angle_Task_2Handle = osThreadCreate(osThread(Angle_Task_2), NULL);

  /* definition and creation of GNSS_Task_3 */
  osThreadDef(GNSS_Task_3, Start_GNSS_FNC, osPriorityLow, 0, 200);
  GNSS_Task_3Handle = osThreadCreate(osThread(GNSS_Task_3), NULL);

  /* definition and creation of Pull_Task_0 */
  osThreadDef(Pull_Task_0, Start_Pull_FNC, osPriorityLow, 0, 128);
  Pull_Task_0Handle = osThreadCreate(osThread(Pull_Task_0), NULL);

  /* definition and creation of Upstream_Task */
  osThreadDef(Upstream_Task, Start_upstream_FNC, osPriorityLow, 0, 128);
  Upstream_TaskHandle = osThreadCreate(osThread(Upstream_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Start_LoRa_FNC */
/**
  * @brief  Function implementing the uart4_LoRa_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_LoRa_FNC */
void Start_LoRa_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_LoRa_FNC */
  /* Infinite loop */
	for(;;)
  {	
		HAL_UARTEx_ReceiveToIdle_IT(&huart4,CmdData_Rx,MAX_Num_of_Relay*Size_of_Relay);
//		if(osMutexWait(myMutex_UART4Handle,0)==osOK)
//		{
			if(osSemaphoreWait(myBinarySem_CommunicationHandle,osWaitForever)==osOK)
			{
				LoRa_Handle();//对上位机发送的命令进行处理
			}
//			osMutexRelease(myMutex_UART4Handle);
//		}

    osDelay(2);
  }
  /* USER CODE END Start_LoRa_FNC */
}

/* USER CODE BEGIN Header_Start_Battery_FNC */
/**
* @brief Function implementing the Battery_Task_1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Battery_FNC */
void Start_Battery_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_Battery_FNC */
  /* Infinite loop */
	osEvent event_battery;
//	uint32_t notify_value1=0;	
	for(;;)
  {
		HAL_UART_Receive_IT(&huart1,BatteryBuffer_old,32);
		event_battery=osSignalWait(Signal_BIT,osWaitForever);
		if(event_battery.value.signals&Signal_BIT)
		{
			Battery_Data_Handle();
		}	
    osDelay(1);
  }
  /* USER CODE END Start_Battery_FNC */
}

/* USER CODE BEGIN Header_Start_Angle_FNC */
/**
* @brief Function implementing the Angle_Task_2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Angle_FNC */
void Start_Angle_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_Angle_FNC */
  /* Infinite loop */
//  uint32_t notify_value;
//	uint32_t notify_value2=0;
	osEvent event_angle;
	for(;;)
  {
		HAL_UART_Receive_IT(&huart2,AngleBuffer_old,66);
//		notify_value2=osThreadFlagsWait(Signal_BIT,osFlagsWaitAll,osWaitForever);
//		if(notify_value2&Signal_BIT)
		event_angle=osSignalWait(Signal_BIT,osWaitForever);
		if(event_angle.value.signals&Signal_BIT)		
		{
			Angle_Data_Handle();
		}
    osDelay(1);
  }
  /* USER CODE END Start_Angle_FNC */
}

/* USER CODE BEGIN Header_Start_GNSS_FNC */
/**
* @brief Function implementing the GNSS_Task_3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_GNSS_FNC */
void Start_GNSS_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_GNSS_FNC */
  /* Infinite loop */
//  uint32_t notify_value3=0;
	osEvent event_gnss;
	for(;;)
  {
		HAL_UARTEx_ReceiveToIdle_IT(&huart3,GnssBuffer_old,100);
//		notify_value3=osThreadFlagsWait(Signal_BIT,osFlagsWaitAll,osWaitForever);
//		if(notify_value3&Signal_BIT)
//		if(osSemaphoreAcquire(myBinarySem_GNSSHandle,osWaitForever)==osOK)
		event_gnss=osSignalWait(Signal_BIT,osWaitForever);
		if(event_gnss.value.signals&Signal_BIT)		
		{
			Gnss_Data_Handle();
		}
    osDelay(1);
  }
  /* USER CODE END Start_GNSS_FNC */
}

/* USER CODE BEGIN Header_Start_Pull_FNC */
/**
* @brief Function implementing the Pull_Task_0 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Pull_FNC */
void Start_Pull_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_Pull_FNC */
  /* Infinite loop */
//	uint32_t notify_value0=0;
	osEvent event_pull;
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)PullBuffer_old,adc_avg*adc_channel);	
	for(;;)
  {
//		notify_value0=osThreadFlagsWait(Signal_BIT,osFlagsWaitAll,osWaitForever);
//		if(notify_value0&Signal_BIT)
		event_pull=osSignalWait(Signal_BIT,osWaitForever);
		if(event_pull.value.signals&Signal_BIT)	
		{
			Pull_Data_Handle();
		}
    osDelay(1);
  }
  /* USER CODE END Start_Pull_FNC */
}

/* USER CODE BEGIN Header_Start_upstream_FNC */
/**
* @brief Function implementing the Upstream_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_upstream_FNC */
void Start_upstream_FNC(void const * argument)
{
  /* USER CODE BEGIN Start_upstream_FNC */
  /* Infinite loop */
	for(;;)
  {
		if(xEventGroupWaitBits(Upstream_Event_Handle,Handshake_Event|Received_Relay_Event,pdFALSE,pdTRUE,portMAX_DELAY))
		{
			LoRa_Operation();	
		}
    osDelay(600);
  }
  /* USER CODE END Start_upstream_FNC */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
