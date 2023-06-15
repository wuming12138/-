#include "main.h"
#include "stdio.h"
#include "string.h"
#include "adc\adc.h"
#include "iic\i2c_hal.h"
#include "lcd\lcd.h"
#include "uart\uart.h"
#include "key_led\key_led.h"

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);
void Led_Proc(void);


//Point
uint32_t										uwTick_Key_Set_Point;
uint32_t										uwTick_Lcd_Set_Point;
uint32_t										uwTick_Uart_Set_Point;
uint32_t										uwTick_Led_Set_Point;


//Key
uint8_t											Key_Val;
uint8_t											Key_Down;
uint8_t											Key_Up;
uint8_t											Key_Old;

//Lcd
char												LCD_ShowString[21];

//Uart
char												UART_ShowString[21];
uint8_t											Rx_Date;

//main Flag
uint8_t											Mode = 0;
uint8_t											Level_Flag;
uint8_t											LED_State;
uint8_t											LED1_Count;
uint8_t											LED2_Flag;
uint8_t											LED2_Count;
uint8_t											LED3_Flag;
uint8_t											LED3_Count;

//main
uint8_t											Height;
float												ADC;
uint8_t											Level = 0;
uint8_t											Level_Old = 0;
uint8_t											Level_Count = 0;
uint8_t											Threshold_Ctrl[3] = {30, 50, 70};
uint8_t											Threshold_Disp[3] = {5, 10, 15};
uint8_t											pThreshold = 0;

int main(void)
{
 
  HAL_Init();
  SystemClock_Config();

	Key_Led_Init();
	LCD_Init();
	ADC2_Init();
	UART_Init();
	I2CInit();
	
	//LCD
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//UART
	HAL_UART_Receive_IT(&huart1, &Rx_Date, 1);
	
	AT24C02_ReadDate(Threshold_Ctrl, 0, 3);
	AT24C02_ReadDate(Threshold_Disp, 0, 3);
	if(Height <= Threshold_Ctrl[0])
	{
		Level = 0;
	}
	else if(Height <= Threshold_Ctrl[1] && Height > Threshold_Ctrl[0])
	{
		Level = 1;
	}
	else if(Height <= Threshold_Ctrl[2] && Height > Threshold_Ctrl[1])
	{
		Level = 2;
	}
	else if(Height > Threshold_Ctrl[2])
	{
		Level = 3;
	}
	
	Level_Old = Level;
	
  while (1)
  {
    Key_Proc();
		Lcd_Proc();
		Uart_Proc();
		Led_Proc();
		
  }
  
}

//Proc
void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)			return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	if(Mode == 0)
	{
		if(Key_Down == 1)
		{
			Mode = 1;
			
		}
	}
	else if(Mode == 1)
	{
		if(Key_Down == 1)
		{
			Mode = 0;
			Threshold_Ctrl[0] = Threshold_Disp[0];
			Threshold_Ctrl[1] = Threshold_Disp[1];
			Threshold_Ctrl[2] = Threshold_Disp[2];
			AT24C02_WriteDate(Threshold_Ctrl, 0, 3);
		}
		if(Key_Down == 2)
		{
			pThreshold++;
			pThreshold%=3;
		}
		if(Key_Down == 3)
		{
			Threshold_Disp[pThreshold] += 5;
			if(Threshold_Disp[pThreshold] > 95)
			{
				Threshold_Disp[pThreshold] = 95;
			}
		}
		if(Key_Down == 4)
		{
			Threshold_Disp[pThreshold] -= 5;
			if(Threshold_Disp[pThreshold] == 0)
			{
				Threshold_Disp[pThreshold] = 5;
			}
		}
	}
	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)			return;
	uwTick_Lcd_Set_Point = uwTick;
	
	if(Mode == 0)
	{
		
		Height = ADC2_GetValue()*100/4095;
		ADC = ADC2_GetValue()*3.3/4095;
		
		if(Height <= Threshold_Ctrl[0])
		{
			Level = 0;
		}
		else if(Height <= Threshold_Ctrl[1] && Height > Threshold_Ctrl[0])
		{
			Level = 1;
		}
		else if(Height <= Threshold_Ctrl[2] && Height > Threshold_Ctrl[1])
		{
			Level = 2;
		}
		else if(Height > Threshold_Ctrl[2])
		{
			Level = 3;
		}
		if(Level != Level_Old)
		{
			if(Level > Level_Old)
			{
				Level_Flag = 1;
			}
			else if(Level < Level_Old)
			{
				Level_Flag = 2;
			}
			LED2_Flag = 1;
 		}
		Level_Old = Level;
		
		
		
		sprintf(LCD_ShowString, "   Liquid Level:    ");
		LCD_DisplayStringLine(Line2, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "   Height:%03dcm    ", Height);
		LCD_DisplayStringLine(Line3, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "   ADC:%.1fV        ", ADC);
		LCD_DisplayStringLine(Line4, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "   Level:%d         ", Level);
		LCD_DisplayStringLine(Line5, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "                    ");
		LCD_DisplayStringLine(Line6, (unsigned char *)LCD_ShowString);
		
	}
	else if(Mode == 1)
	{
		
		sprintf(LCD_ShowString, "   Parameter Setup: ");
		LCD_DisplayStringLine(Line2, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "                    ");
		LCD_DisplayStringLine(Line3, (unsigned char *)LCD_ShowString);
		if(pThreshold == 0)
		{
			LCD_SetTextColor(Black);
		}
		else
		{
			LCD_SetTextColor(White);
		}
		sprintf(LCD_ShowString, "   Threshold1:%02d  ", Threshold_Disp[0]);
		LCD_DisplayStringLine(Line4, (unsigned char *)LCD_ShowString);
			if(pThreshold == 1)
		{
			LCD_SetTextColor(Black);
		}
		else
		{
			LCD_SetTextColor(White);
		}
		sprintf(LCD_ShowString, "   Threshold2:%02d  ", Threshold_Disp[1]);
		LCD_DisplayStringLine(Line5, (unsigned char *)LCD_ShowString);
			if(pThreshold == 2)
		{
			LCD_SetTextColor(Black);
		}
		else
		{
			LCD_SetTextColor(White);
		}
		sprintf(LCD_ShowString, "   Threshold3:%02d  ", Threshold_Disp[2]);
		LCD_DisplayStringLine(Line6, (unsigned char *)LCD_ShowString);
		LCD_SetTextColor(White);
	}
}

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 50)			return;
	uwTick_Uart_Set_Point = uwTick;
	if(Level_Flag == 1)
	{
		sprintf(UART_ShowString, "A:H%02d+L%d+U\r\n", Height, Level);
		HAL_UART_Transmit(&huart1, (unsigned char *)UART_ShowString, strlen(UART_ShowString), 50);
		Level_Flag = 0;
	}
	else if(Level_Flag == 2)
	{
		sprintf(UART_ShowString, "A:H%02d+L%d+D\r\n", Height, Level);
		HAL_UART_Transmit(&huart1, (unsigned char *)UART_ShowString, strlen(UART_ShowString), 50);
		Level_Flag = 0;
	}
}

void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 200)			return;
	uwTick_Led_Set_Point = uwTick;
	
	

	LED1_Count++;
	if(LED1_Count == 4)
	{
		LED_State ^= 0x01;
		LED1_Count = 0;
	}
	if(LED2_Flag == 1)
	{
		LED_State ^= 0x02;
		LED2_Count++;
		if(LED2_Count == 9)
		{
			LED2_Count = 0;
			LED2_Flag = 0;
			LED_State &= ~0x02;
		}
	}
	if(LED3_Flag == 1)
	{
		LED_State ^= 0x04;
		LED3_Count++;
		if(LED3_Count == 9)
		{
			LED3_Count = 0;
			LED3_Flag = 0;
			LED_State &= ~0x04;
		}
	}
	LED_Disp(LED_State);
}




//Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(Rx_Date == 'C')
	{
		sprintf(UART_ShowString, "C:H%02d+L%d\r\n", Height, Level);
		HAL_UART_Transmit(&huart1, (unsigned char *)UART_ShowString, strlen(UART_ShowString), 50);
	}
	else if(Rx_Date == 'S')
	{
		sprintf(UART_ShowString, "S:TL%2d+TM%2d+TH%2d", Threshold_Ctrl[0], Threshold_Ctrl[1], Threshold_Ctrl[2]);
		HAL_UART_Transmit(&huart1, (unsigned char *)UART_ShowString, strlen(UART_ShowString), 50);
	}
	LED3_Flag = 1;
	
	HAL_UART_Receive_IT(&huart1, &Rx_Date, 1);
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
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
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
