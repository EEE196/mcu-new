/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../test/test.h"
#include <stdio.h>
#include "../../so/so.h"
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

/* USER CODE BEGIN PV */
typedef struct DATA
{
	struct sps30_measurement PM_Data;
	CO_t CO_Data;
	GPS_t GPS_Data;
} CollatedData;
CollatedData collatedData;
void* vptr_test = &collatedData;
uint8_t buffer[sizeof(collatedData)];
FRESULT     fres;                 //Result after operations
uint16_t data_ready = 0;
uint16_t ret;
uint16_t err;
uint32_t vgas;
uint32_t vgas0;
uint32_t vtemp;
uint16_t vrefint_cal;                        // VREFINT calibration value
uint32_t vref;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI2_Init();
	MX_USART2_UART_Init();
	MX_FATFS_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_SPI3_Init();
	MX_TIM11_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */
	//test_sd();
	//test_pm();
	//test_gps();
	//test_lora();


	init_co();

	init_pm();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	init_sd();


	init_lora();

	printf("sleeping\n");
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	MX_USART1_UART_Init();
	init_gps();
	HAL_PWR_EnableSleepOnExit ();
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
	int length = snprintf(NULL, 0, "%d-%f.bin",collatedData.GPS_Data.date, collatedData.GPS_Data.utc_time);
	char comb_str[length+1];
	sprintf(comb_str, "%d-%f.bin",collatedData.GPS_Data.date, collatedData.GPS_Data.utc_time);
	init_gps();
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		HAL_PWR_EnableSleepOnExit ();
		HAL_SuspendTick();
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		HAL_ResumeTick();
		//collect SO
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		vgas = HAL_ADC_GetValue(&hadc1);

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		vgas0 = HAL_ADC_GetValue(&hadc1);

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		vtemp = HAL_ADC_GetValue(&hadc1);

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		HAL_ADC_Stop(&hadc1);


		vrefint_cal= *((uint16_t*)VREFINT_CAL_ADDR); // read VREFINT_CAL_ADDR memory location
		vref = 3.3*vrefint_cal/HAL_ADC_GetValue(&hadc1);
		//printf("%f \n", vref);

		collatedData.PM_Data.SO_ppm = so_convert(vgas, vgas0, vtemp, vref);
		printf("SO2 SENSOR COLLECTION:\n\t%d ppm SO2\n", collatedData.PM_Data.SO_ppm);
		//collect CO
		data_ready = 0;
		err = scd30_get_data_ready(&data_ready)	;
		if (data_ready)
			err =
					scd30_read_measurement(&collatedData.CO_Data.co2_ppm, &collatedData.CO_Data.temperature, &collatedData.CO_Data.relative_humidity);
		if (err == NO_ERROR) {
			printf("CO2 SENSOR COLLECTION:\n\t%f ppm CO2\n\t%f °C\n\t%f %%RH\n", collatedData.CO_Data.co2_ppm, collatedData.CO_Data.temperature, collatedData.CO_Data.relative_humidity);
		} else {
			printf("co data collect failed\n");
		}
		//collect PM
		ret = sps30_read_measurement(&collatedData.PM_Data);
		if (ret < 0) {
			printf("pm data collect failed\n");
		} else {
			printf("PM SENSOR COLLECTION:\n"
					"\t%f pm2.5\n"
					"\t%f pm10.0\n"
					"\t%f nc2.5\n"
					"\t%f nc10.0\n",
					collatedData.PM_Data.mc_2p5, collatedData.PM_Data.mc_10p0,
					collatedData.PM_Data.nc_2p5, collatedData.PM_Data.nc_10p0);
		}

		//squash structs
		memcpy(buffer, vptr_test, sizeof(collatedData));
		//send to lora
		if (!rfm95_send_data(&rfm95_handle, buffer, sizeof(buffer))) {
			printf("lora send failed\n\r");
		}
		else{
			printf("LORA SEND SUCCESSFUL\n");
		}
		//save to sd
		fres = f_open(&file, comb_str, FA_WRITE | FA_READ | FA_OPEN_APPEND);
		if(fres != FR_OK)
		{
			printf("File creation/open Error : (%i)\r\n", fres);
		} else {
			f_write(&file, buffer, sizeof(buffer), NULL);
			f_close(&file);
			printf("SD WRITE SUCCESSFUL\n");
		}
		//reset timer, 2seconds
		TIM11->CNT = 0;
		HAL_TIM_Base_Start_IT(&htim11);
		HAL_SuspendTick();
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		HAL_ResumeTick();
		HAL_TIM_Base_Stop_IT(&htim11);
		//call gps, restart
		GPS_Init();
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle The Output (LED) Pin

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
#include <stdio.h>
#include <string.h>
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the UART2 and Loop until the end of transmission */
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_13) // If The INT Source Is EXTI Line9 (A9 Pin)
	{
	}
}
void GPS_UART_CallBack(){
	if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = rx_data;
		HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
	} else {

		if(GPS_validate((char*) rx_buffer))
		{
			if (GPS_parse((char*) rx_buffer)) {
				collatedData.GPS_Data = GPS;
				HAL_PWR_DisableSleepOnExit ();
			} else {
				HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
			}
		} else {
			HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
		}
		rx_index = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
