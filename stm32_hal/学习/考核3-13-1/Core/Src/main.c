#include "main.h"
#include "led\led.h"
#include "usart\uart.h"
#include "string.h"
#include "stdio.h"

void SystemClock_Config(void);


void LED_Proc(void);
void USART1_Proc(void);
	

uint32_t				uwTick_LED_Set_Point = 0;
uint32_t				uwTick_USART1_Set_Point = 0;


char						str[21] = {0};
uint32_t				i = 0;


char						TxDate1[100] = {0};
char						TxDate2[100] = {0};
char						TxDate3[100] = {0};
unsigned char		RxDate[2];


int main(void)
{
	
	//系统初始化
  HAL_Init();
  SystemClock_Config();
	
	//外设初始化
  LED_Init();
	USART1_Init();
	
	HAL_UART_Receive_IT(&huart1, RxDate, 2);
  while (1)
  {
		LED_Proc();
		USART1_Proc();
  }
}



void LED_Proc(void)
{
	if((uwTick - uwTick_LED_Set_Point) < 300)		return;
	uwTick_LED_Set_Point = uwTick;

	if(RxDate[1] == 0x58)
	{
		LED_DispOne_ON(RxDate[0] - 48);
	}
	if(RxDate[1] == 0x59)
	{
		LED_DispOne_OFF(RxDate[0] - 48);
	}
}



void USART1_Proc(void)
{
	if((uwTick - uwTick_USART1_Set_Point) < 5000)		return;
	uwTick_USART1_Set_Point = uwTick;
	
	sprintf(TxDate1, "\r\n\r\n tX打开一个led灯 tY灭一个led灯\r\n"); 
	sprintf(TxDate2, " 请输入指令！\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t * )TxDate1, strlen(TxDate1), 50);
	HAL_UART_Transmit(&huart1, (uint8_t * )TxDate2, strlen(TxDate2), 50);	

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	HAL_UART_Receive_IT(&huart1, RxDate, 2);
	
	if(RxDate[1] == 0x58)
	{
		sprintf(TxDate3, "LED%d开了", RxDate[0] - 48);
		HAL_UART_Transmit(&huart1, (uint8_t * )TxDate3, strlen(TxDate3), 50);
	}
	if(RxDate[1] == 0x59)
	{
		sprintf(TxDate3, "LED%d关了", RxDate[0] - 48);
		HAL_UART_Transmit(&huart1, (uint8_t * )TxDate3, strlen(TxDate3), 50);
	}
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
