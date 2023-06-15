#include "main.h"
#include "stdio.h"
#include "string.h"
#include "tim\tim.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "rtc\rtc.h"

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);
void State_Proc(void);

//Point
uint32_t								uwTick_Key_Set_Point;
uint32_t								uwTick_Lcd_Set_Point;
uint32_t								uwTick_State_Set_Point;
uint32_t								uwTick_Led_Set_Point;
uint32_t								uwTick_Key_Tim_Point;
uint32_t								uwTick_Lift_Tim_Point;


//Key
uint8_t									Key_Val;
uint8_t									Key_Down;
uint8_t									Key_Up;
uint8_t									Key_Old;

//LCD
char										LCD_ShowSting[21];
uint8_t									LCD_Flag;
uint8_t									LCD_Count;

//RTC
RTC_TimeTypeDef 				sTime = {0};
RTC_DateTypeDef 				sDate = {0};

//main
uint8_t									Current_floor = 2;
uint8_t									Select_floor = 0x00;
uint8_t									floor_Up_Flag = 0;
uint8_t									floor_Down_Flag = 0;
uint8_t									State;

uint8_t									i;
uint8_t									j;

int main(void)
{

  HAL_Init();
  SystemClock_Config();

	Key_Led_Init();
	RTC_Init();
	LCD_Init();
	PA6_PWM_Init();
	PA7_PWM_Init();
	

	//Lcd
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//TIM
	HAL_TIM_Base_Start(&htim16);
	
	HAL_TIM_Base_Start(&htim17);
	

  while (1)
  {
		Lcd_Proc();
		Key_Proc();
		Led_Proc();
		State_Proc();
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
	if(State == 0 || State == 1)
	{
		if(Key_Down > 0)
		{
			if(Key_Down == Current_floor)		return;
			
			Select_floor |= (0x01 << (Key_Down - 1));
			uwTick_Key_Tim_Point = uwTick;
		}
	}
	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 250)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	
	
	sprintf(LCD_ShowSting, "         %d,%d,%d", Current_floor, State, Select_floor);
	if(LCD_Flag == 1)
	{
		
		LCD_Count++;
		if(LCD_Count % 2 == 1)
		{
			LCD_ShowSting[9] = ' ';
		}
		if(LCD_Count > 4)
		{
			LCD_Count = 0;
			LCD_Flag = 0;
		}
	}
	LCD_DisplayStringLine(Line4,(unsigned char *)LCD_ShowSting);	
	sprintf(LCD_ShowSting, "      %02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
	LCD_DisplayStringLine(Line8,(unsigned char *)LCD_ShowSting);	
}

void State_Proc(void)
{
	if(uwTick - uwTick_State_Set_Point < 50)		return;
	uwTick_State_Set_Point = uwTick;

	if(uwTick_Key_Tim_Point > 20 && State == 0)
	{
		State = 1;
	}
	if(uwTick - uwTick_Key_Tim_Point > 3000 && State == 1)
	{
		State = 2;
		uwTick_Key_Tim_Point = 0;
		uwTick_Lift_Tim_Point = uwTick;
	}
	//Close
	if(State == 2)
	{
		HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
		__HAL_TIM_SetCompare(&htim17, TIM_CHANNEL_1, 50);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		
		if(uwTick - uwTick_Lift_Tim_Point > 2000)
		{
			State = 3;
			uwTick_Lift_Tim_Point = uwTick;
			HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
		}
	}
	//Up
	if(State == 3)
	{
		if((Select_floor - (0x01 << (Current_floor - 1))) > 0)
		{
			HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
			__HAL_TIM_SetCompare(&htim16, TIM_CHANNEL_1, 80);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
			floor_Up_Flag = 1;
		}
		else if((Select_floor - (0x01 << (Current_floor - 1))) < 0)
		{
			HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
			__HAL_TIM_SetCompare(&htim16, TIM_CHANNEL_1, 60);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
			floor_Down_Flag = 1;
		}
		if(uwTick - uwTick_Lift_Tim_Point > 6000)
		{
			HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
			
			State = 4;
			uwTick_Lift_Tim_Point = uwTick;
		}
	}
	//Open
	if(State == 4)
	{
		if(floor_Up_Flag == 1)
		{
			
			
			for(;Current_floor<=4;Current_floor++)
			{
				if(Select_floor & (0x01 << (Current_floor - 1)))
				{
					Select_floor &= ~(0x01 << (Current_floor - 1));
					break;
				}
			}
			floor_Up_Flag = 0;
			LCD_Flag = 1;
		}
		else if(floor_Down_Flag == 1)
		{
			
			
			for(;Current_floor>0;Current_floor--)
			{
				if(Select_floor & (0x01 << (Current_floor - 1)))
				{
					Select_floor &= ~(0x01 << (Current_floor - 1));
					break;
				}
			}
			floor_Down_Flag = 0;
			LCD_Flag = 1;
		}
		HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
		__HAL_TIM_SetCompare(&htim17, TIM_CHANNEL_1, 60);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		if(uwTick - uwTick_Lift_Tim_Point > 2000)
		{
			State = 5;
			uwTick_Lift_Tim_Point = uwTick;
			HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
		}
	}
	if(State == 5)
	{
		if(uwTick - uwTick_Lift_Tim_Point > 2000)
		{
			State = 6;
			uwTick_Lift_Tim_Point = uwTick;
		}
	}
	if(State == 6)
	{
		if(Select_floor == 0)
		{
			State = 0;
		}
		else
		{
			State = 2;
		}
	}
}

void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 500)		return;
	uwTick_Led_Set_Point = uwTick;
	if(floor_Up_Flag == 1)
	{
		LED_Disp(Select_floor | (0x10<<i));
		i++;
		i%=4;
	}
	else if(floor_Down_Flag == 1)
	{
		LED_Disp(Select_floor | (0x80>>i));
		i++;
		i%=4;
	}
	else
	{
		LED_Disp(Select_floor);
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
	
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;

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
