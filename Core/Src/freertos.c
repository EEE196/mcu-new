/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include <stdio.h>
#include "../../pm2.5/sps30.h"
#include "../../gps/gps.h"
#include "fatfs.h"
#include "../../lora/rfm95.h"
#include <spi.h>
#include "../../co2/scd30.h"
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
typedef struct IP_TASK_COMMANDS
{
	uint8_t from_Task; /*0 - GPS; 1 - PM; 2 - CO; 3 - SO */
	void *pvData; /* Holds or points to any data associated with the event. */

} xIPStackEvent_t;

typedef struct DATA
{
	GPS_t GPS_Data;
	//SO_t SO_Data;
	struct sps30_measurement PM_Data;
	CO_t CO_Data;
} CollatedData;

QueueHandle_t xQueueCollate;
QueueHandle_t xQueueSD;
QueueHandle_t xQueueLoRa;
/* USER CODE END Variables */
osThreadId PMHandle;
osThreadId GPSHandle;
osThreadId COLLATEHandle;
osThreadId SDHandle;
osThreadId LORAHandle;
osThreadId COHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void PM_Task(void const * argument);
void GPS_Task(void const * argument);
void COLLATE_Task(void const * argument);
void SD_Task(void const * argument);
void LORA_Task(void const * argument);
void CO_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	printf("STACK OVERFLOW\n");
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	xQueueCollate = xQueueCreate( 4, sizeof( xIPStackEvent_t ) );
	xQueueSD = xQueueCreate( 10, sizeof( CollatedData ) );
	xQueueLoRa = xQueueCreate( 10, sizeof( CollatedData ) );
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of PM */
  osThreadDef(PM, PM_Task, osPriorityNormal, 0, 128);
  PMHandle = osThreadCreate(osThread(PM), NULL);

  /* definition and creation of GPS */
  osThreadDef(GPS, GPS_Task, osPriorityNormal, 0, 128);
  GPSHandle = osThreadCreate(osThread(GPS), NULL);

  /* definition and creation of COLLATE */
  osThreadDef(COLLATE, COLLATE_Task, osPriorityBelowNormal, 0, 128);
  COLLATEHandle = osThreadCreate(osThread(COLLATE), NULL);

  /* definition and creation of SD */
  osThreadDef(SD, SD_Task, osPriorityNormal, 0, 2056);
  SDHandle = osThreadCreate(osThread(SD), NULL);

  /* definition and creation of LORA */
  osThreadDef(LORA, LORA_Task, osPriorityNormal, 0, 128);
  LORAHandle = osThreadCreate(osThread(LORA), NULL);

  /* definition and creation of CO */
  osThreadDef(CO, CO_Task, osPriorityNormal, 0, 128);
  COHandle = osThreadCreate(osThread(CO), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_PM_Task */
/**
  * @brief  Function implementing the PM thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_PM_Task */
void PM_Task(void const * argument)
{
  /* USER CODE BEGIN PM_Task */
	struct sps30_measurement m;
	int16_t ret;

	void* pointer = &m;
	xIPStackEvent_t toQueue = { 1, pointer };


	/* Busy loop for initialization, because the main loop does not work without
	 * a sensor.
	 */
	while (sps30_probe() != 0) {
		osDelay(1000000); /* wait 1s */
	}


	ret = sps30_start_measurement();
	if (ret < 0)
	{

	}
	sensirion_sleep_usec(SPS30_MEASUREMENT_DURATION_USEC); /* wait 1s */

	for(;;)
	{
		ret = sps30_read_measurement(&m);
		if (ret < 0) {

		} else {
			xQueueSend( xQueueCollate, ( void* ) &toQueue, ( TickType_t ) 10);
			vTaskSuspend( NULL );
		}
	}
  /* USER CODE END PM_Task */
}

/* USER CODE BEGIN Header_GPS_Task */
/**
* @brief Function implementing the GPS thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_GPS_Task */
void GPS_Task(void const * argument)
{
  /* USER CODE BEGIN GPS_Task */

	/* USER CODE BEGIN GPS_Task */
	void* pointer = &GPS;
	xIPStackEvent_t toQueue = { 0, pointer };
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
	GPS_Init();
	ulTaskNotifyTake( pdTRUE,
			xMaxBlockTime );
	/* Infinite loop */
	for(;;)
	{
		if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
			rx_buffer[rx_index++] = rx_data;

		} else {
			if(GPS_validate((char*) rx_buffer))
			{
				GPS_parse((char*) rx_buffer);
				xQueueSend( xQueueCollate, ( void* ) &toQueue, ( TickType_t ) 10);
				vTaskSuspend( NULL );
			}
			rx_index = 0;
			memset(rx_buffer, 0, sizeof(rx_buffer));
		}
		GPS_Init();
		ulTaskNotifyTake( pdTRUE,
				xMaxBlockTime );
	}
  /* USER CODE END GPS_Task */
}

/* USER CODE BEGIN Header_COLLATE_Task */
/**
* @brief Function implementing the COLLATE thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_COLLATE_Task */
void COLLATE_Task(void const * argument)
{
  /* USER CODE BEGIN COLLATE_Task */
	uint8_t counter = 0;
	CollatedData collatedData;
	xIPStackEvent_t xReceivedEvent;

	/* Infinite loop */
	for(;;)
	{
		xQueueReceive( xQueueCollate, &xReceivedEvent, portMAX_DELAY );
		switch( xReceivedEvent.from_Task )
		{
		case 0:
			collatedData.GPS_Data = *(GPS_t*)xReceivedEvent.pvData;
			break;
		case 1:
			collatedData.PM_Data = *(struct sps30_measurement*)xReceivedEvent.pvData;
			break;
		case 2:
			collatedData.CO_Data = *(CO_t*)xReceivedEvent.pvData;
			break;
		/*case 3:
			collatedData.SO_Data = *(SO_t*)xReceivedEvent.pvData;
			break;*/
		}
		counter++;
		if (counter == 3)
		{
			xQueueSend( xQueueSD, ( void* ) &collatedData, ( TickType_t ) 10);
			xQueueSend( xQueueLoRa, ( void* ) &collatedData, ( TickType_t ) 10);
			counter = 0;
			osDelay(2000);
			//vTaskResume( PMHandle );
			//vTaskResume( COHandle );
			//vTaskResume( GPSHandle );
			//vTaskResume( SOHandle );
		}
	}
  /* USER CODE END COLLATE_Task */
}

/* USER CODE BEGIN Header_SD_Task */
/**
* @brief Function implementing the SD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SD_Task */
void SD_Task(void const * argument)
{
  /* USER CODE BEGIN SD_Task */
	CollatedData xReceivedEvent;
	void* vptr_test = &xReceivedEvent;
	uint8_t buffer[sizeof(xReceivedEvent)];

	FATFS       FatFs;                //FatFs handle
	FIL         fil;                  //File handle
	FRESULT     fres;                 //Result after operations

	fres = f_mount(&FatFs, "", 1);    //1=mount now
	if (fres != FR_OK)
	{
		vTaskSuspend( NULL );
	}
	/* Infinite loop */
	for(;;)
	{
		xQueueReceive( xQueueSD, &xReceivedEvent, portMAX_DELAY );
		fres = f_open(&fil, "data.bin", FA_WRITE | FA_READ | FA_OPEN_APPEND);
		vptr_test = &xReceivedEvent;
		memcpy(buffer, vptr_test, sizeof(xReceivedEvent));
		f_write(&fil, buffer, sizeof(buffer), NULL);
		f_close(&fil);
	}
  /* USER CODE END SD_Task */
}

/* USER CODE BEGIN Header_LORA_Task */
/**
* @brief Function implementing the LORA thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LORA_Task */
void LORA_Task(void const * argument)
{
  /* USER CODE BEGIN LORA_Task */
	CollatedData xReceivedEvent;
	void* vptr_test1 = &xReceivedEvent;
	uint8_t buffer1[sizeof(xReceivedEvent)];
	rfm95_handle_t rfm95_handle = {
			.spi_handle = &hspi3,
			.nss_port = RFM95_NSS_GPIO_Port,
			.nss_pin = RFM95_NSS_Pin,
			.nrst_port = RFM95_NRST_GPIO_Port,
			.nrst_pin = RFM95_NRST_Pin,
			.irq_port = RFM95_DIO0_GPIO_Port,
			.irq_pin = RFM95_DIO0_Pin,
			.dio5_port = RFM95_DIO5_GPIO_Port,
			.dio5_pin = RFM95_DIO5_Pin,
			.device_address = {0xDE, 0xAD, 0xBE, 0xEF},
			.application_session_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			.network_session_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			.reload_frame_counter = NULL,
			.save_frame_counter = NULL
	};
	if (!rfm95_init(&rfm95_handle)) {
		printf("RFM95 init failed\n\r");
	}
	/* Infinite loop */
	for(;;)
	{
		xQueueReceive( xQueueLoRa, &xReceivedEvent, portMAX_DELAY );
		vptr_test1 = &xReceivedEvent;
		memcpy(buffer1, vptr_test1, sizeof(xReceivedEvent));
		if (!rfm95_send_data(&rfm95_handle, buffer1, sizeof(buffer1))) {
			printf("RFM95 send failed\n\r");
		}
	}
  /* USER CODE END LORA_Task */
}

/* USER CODE BEGIN Header_CO_Task */
/**
* @brief Function implementing the CO thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CO_Task */
void CO_Task(void const * argument)
{
  /* USER CODE BEGIN CO_Task */
	CO_t data;
	int16_t err;
	uint16_t interval_in_seconds = 2;
	uint16_t data_ready = 0;

	void* pointer = &data;
	xIPStackEvent_t toQueue = { 2, pointer };


	/* Busy loop for initialization, because the main loop does not work without
	 * a sensor.
	 */
	while (scd30_probe() != NO_ERROR) {
		sensirion_sleep_usec(1000000u);
	}

	scd30_set_measurement_interval(interval_in_seconds);
	sensirion_sleep_usec(20000u);
	scd30_start_periodic_measurement(0);


	/* Measure co2, temperature and relative humidity and store into
	 * variables.
	 */

	/* Infinite loop */
	for(;;)
	{
		data_ready = 0;
		err = scd30_get_data_ready(&data_ready);
		if (data_ready)
			err =
					scd30_read_measurement(&data.co2_ppm, &data.temperature, &data.relative_humidity);
		if (err == NO_ERROR) {
			xQueueSend( xQueueCollate, ( void* ) &toQueue, ( TickType_t ) 10);
			vTaskSuspend( NULL );
		}
	}
  /* USER CODE END CO_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void GPS_UART_CallBack()
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR( GPSHandle,
			&xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/* USER CODE END Application */
