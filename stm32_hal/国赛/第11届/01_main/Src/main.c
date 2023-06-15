#include "main.h"
#include "string.h"
#include "stdio.h"
#include "tim\tim.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "adc\adc.h"


void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);


uint32_t												uwTick_Key_Set_Point;
uint32_t												uwTick_Lcd_Set_Point;

//Key
uint8_t													Key_Val;
uint8_t													Key_Down;
uint8_t													Key_Up;
uint8_t													Key_Old;

//LCD
char														LCD_ShowStr[21];

//main
uint32_t												F1;
uint32_t												F2;
uint32_t												X1;
uint32_t												X2;
float														V1;
float														V2;
uint8_t													Mode;
uint8_t													PA7_Mode;
uint8_t													VLED_Disp = 0;
uint8_t													FLED_Disp = 1;
uint8_t													VLED = 0;
uint8_t													FLED = 1;
uint8_t													uwLED;

int main(void)
{

  HAL_Init();
  SystemClock_Config();
	
	MX_GPIO_Init();
	LCD_Init();
	ADC2_Init();
	TIM2_IC_Init();
	TIM15_IC_Init();
	TIM3_OC_Init();
	
	//LCD
	LCD_Clear(Black);
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	
	//TIM
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim15, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);
	
  while (1)
  {
		Key_Proc();
		Lcd_Proc();
  }
  
}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)		return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);	
	Key_Old = Key_Val;
	
	if(Key_Down == 1)
	{
		Mode++;
		Mode %= 2;
		LCD_Clear(Black);
		if(Mode == 0)
		{
			VLED = VLED_Disp;
			FLED = FLED_Disp;
			uwLED = 0;
		}
	}
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			VLED_Disp++;
			VLED_Disp %= 8;
			if(VLED_Disp == FLED_Disp)
				VLED_Disp++;
			VLED_Disp %= 8;
		}
		if(Key_Down == 3)
		{
			FLED_Disp++;
			FLED_Disp %= 8;
			if(VLED_Disp == FLED_Disp)
				FLED_Disp++;
			FLED_Disp %= 8;
		}
	}

	if(Key_Down == 4)
	{
		PA7_Mode++;
		PA7_Mode %= 2;
		
	}	
}
void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	V1 = Get_ADC_PR5() * 3.3 / 4095;
	V2 = Get_ADC_PR6() * 3.3 / 4095;
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "    DATA         ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "    V1:%.1fV     ", V1);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "    V2:%.1fV     ", V2);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);	
		sprintf(LCD_ShowStr, "    F1:%dHz      ", F1);
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);	
		sprintf(LCD_ShowStr, "    F2:%dHz      ", F2);
		LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);	
	}
	else if(Mode == 1)
	{
		
		sprintf(LCD_ShowStr, "    PARA         ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "    VD:LD%1d     ", VLED_Disp + 1);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "    FD:LD%1d     ", FLED_Disp + 1);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);	

	}
	
	if(V1 > V2)
		uwLED |= 0x01 << VLED;
	else
		uwLED &= ~(0x01 << VLED);
	if(F1 > F2)
		uwLED |= 0x01 << FLED;
	else
		uwLED &= ~(0x01 << FLED);
	LED_Disp(uwLED);
}

//CallBack
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			X1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
			F1 = 1000000 / X1;
		}
	}
	
	if(htim->Instance == TIM15)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			X2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
			F2 = 1000000 / X2;
		}
	}
	
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			if(PA7_Mode == 0)
				__HAL_TIM_SetCompare(htim, TIM_CHANNEL_2, __HAL_TIM_GetCounter(htim) + X1 / 2);
			else if(PA7_Mode == 1)
				__HAL_TIM_SetCompare(htim, TIM_CHANNEL_2, __HAL_TIM_GetCounter(htim) + X2 / 2);
		}
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
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
