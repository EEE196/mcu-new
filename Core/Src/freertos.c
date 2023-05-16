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
	//CO_t CO_Data;
} CollatedData;

QueueHandle_t xQueueCollate;
QueueHandle_t xQueueSD;
//QueueHandle_t xQueueLoRa;
/* USER CODE END Variables */
osThreadId PMHandle;
osThreadId GPSHandle;
osThreadId COLLATEHandle;
osThreadId SDHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void PM_Task(void const * argument);
void GPS_Task(void const * argument);
void COLLATE_Task(void const * argument);
void SD_Task(void const * argument);

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
	//xQueueLORA = xQueueCreate( 10, sizeof( CollatedData ) );
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
  osThreadDef(SD, SD_Task, osPriorityNormal, 0, 128);
  SDHandle = osThreadCreate(osThread(SD), NULL);

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
		printf("SPS sensor probing failed\n");
		sensirion_sleep_usec(1000000); /* wait 1s */
	}
	printf("SPS sensor probing successful\n");

	uint8_t fw_major;
	uint8_t fw_minor;
	ret = sps30_read_firmware_version(&fw_major, &fw_minor);
	if (ret) {
		printf("error reading firmware version\n");
	} else {
		printf("FW: %u.%u\n", fw_major, fw_minor);
	}

	char serial_number[SPS30_MAX_SERIAL_LEN];
	ret = sps30_get_serial(serial_number);
	if (ret) {
		printf("error reading serial number\n");
	} else {
		printf("Serial Number: %s\n", serial_number);
	}

	ret = sps30_start_measurement();
	if (ret < 0)
		printf("error starting measurement\n");
	printf("measurements started\n");
	sensirion_sleep_usec(SPS30_MEASUREMENT_DURATION_USEC); /* wait 1s */

	for(;;)
	{
		ret = sps30_read_measurement(&m);
		if (ret < 0) {
			printf("error reading measurement\n");

		} else {
			printf("measured values:\n"
					"\t%0.2f pm1.0\n"
					"\t%0.2f pm2.5\n"
					"\t%0.2f pm4.0\n"
					"\t%0.2f pm10.0\n"
					"\t%0.2f nc0.5\n"
					"\t%0.2f nc1.0\n"
					"\t%0.2f nc2.5\n"
					"\t%0.2f nc4.5\n"
					"\t%0.2f nc10.0\n"
					"\t%0.2f typical particle size\n\n",
					m.mc_1p0, m.mc_2p5, m.mc_4p0, m.mc_10p0, m.nc_0p5, m.nc_1p0,
					m.nc_2p5, m.nc_4p0, m.nc_10p0, m.typical_particle_size);
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
			GPS_Init();
			ulTaskNotifyTake( pdTRUE,
					xMaxBlockTime );
		} else {
			GPS_print((char*)rx_buffer);
			if(GPS_validate((char*) rx_buffer))
			{
				GPS_parse((char*) rx_buffer);
				xQueueSend( xQueueCollate, ( void* ) &toQueue, ( TickType_t ) 10);
				vTaskSuspend( NULL );
			}
			rx_index = 0;
			memset(rx_buffer, 0, sizeof(rx_buffer));
			GPS_Init();
			ulTaskNotifyTake( pdTRUE,
					xMaxBlockTime );
		}
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
		/*case 2:
			collatedData.CO_Data = *(CO_t*)xReceivedEvent.pvData;
			break;
		case 3:
			collatedData.SO_Data = *(SO_t*)xReceivedEvent.pvData;
			break;*/
		}
		counter++;
		if (counter == 4)
		{
			xQueueSend( xQueueSD, ( void* ) &collatedData, ( TickType_t ) 10);
			//xQueueSend( xQueueLORA, ( void* ) &collatedData, ( TickType_t ) 10);
			counter = 0;
			osDelay(2000);
			vTaskResume( PMHandle );
			//vTaskResume( COHandle );
			vTaskResume( GPSHandle );
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

	FATFS       FatFs;                //FatFs handle
	FIL         fil;                  //File handle
	FRESULT     fres;                 //Result after operations

	fres = f_mount(&FatFs, "", 1);    //1=mount now
	if (fres != FR_OK)
	{
		printf("No SD Card found : (%i)\r\n", fres);
		vTaskSuspend( NULL );
	}
	printf("SD Card Mounted Successfully!!!\r\n");
	FATFS *pfs;
	DWORD fre_clust;
	uint32_t totalSpace, freeSpace;

	f_getfree("", &fre_clust, &pfs);
	totalSpace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
	freeSpace = (uint32_t)(fre_clust * pfs->csize * 0.5);

	printf("TotalSpace : %lu bytes, FreeSpace = %lu bytes\n", totalSpace, freeSpace);
	fres = f_open(&fil, "data.bin", FA_WRITE | FA_READ | FA_OPEN_APPEND);
	if(fres != FR_OK)
	{
		printf("SD CARD OK");
	}
	/* Infinite loop */
	for(;;)
	{
		xQueueReceive( xQueueSD, &xReceivedEvent, portMAX_DELAY );
		void* vptr_test = &xReceivedEvent;
		uint8_t buffer[sizeof(xReceivedEvent)];
		memcpy(buffer, vptr_test, sizeof(xReceivedEvent));
		f_write(&fil, buffer, sizeof(buffer), NULL);
	}
  /* USER CODE END SD_Task */
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
