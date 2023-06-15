#include "main.h"
#include "key_led\key_led.h"
#include "adc\adc.h"
#include "tim\tim.h"
#include "uart\uart.h"
#include "lcd\lcd.h"
#include "iic\i2c_hal.h"
#include "ds18b20\ds18b20_hal.h"
#include "seg\seg.h"
#include "string.h"
#include "stdio.h"


void SystemClock_Config(void);

void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);
void Seg_Proc(void);

//Point
uint32_t									uwTick_Key_Set_Point;
uint32_t									uwTick_Lcd_Set_Point;
uint32_t									uwTick_Uart_Set_Point;
uint32_t									uwTick_Seg_Set_Point;

uint8_t										Uart_Count_Point;

//Key
uint8_t										Key_Val;
uint8_t										Key_Down;
uint8_t										Key_Up;
uint8_t										Key_Old;

//LCD
char											LCD_ShowStr[21];

//TIM
uint16_t									PWM_T;
uint16_t									PWM_t;

//IIC
typedef union
{
	uint16_t	N_16;
	uint8_t		N_8[2];
}N_TYPEDEF;
N_TYPEDEF N_R;

//UART
uint8_t										Rx_Buffer;
uint8_t										Rx_Buf[50];
uint8_t										pRx_Buf;
uint32_t									Uart_Set_Point;
uint8_t										Uart_Flag;

//main
uint8_t										Mode;
uint8_t										Seg_Mode;
uint8_t										pPara;
float											AO1;
float											AO2;
float											PWM_Duty;
float											Temp;
uint8_t										Temp_Compare_Disp = 30;
uint8_t										Temp_Compare = 30;
uint8_t										AO_Seclet_Disp = 0;
uint8_t										AO_Seclet = 0;
uint8_t										Temp_Flag;
uint8_t										AO_Flag;
uint8_t										uwLED;


int main(void)
{
	
  HAL_Init();
  SystemClock_Config();

	GPIO_Init();
	LCD_Init();
	ADC2_Init();
	TIM3_Init();
	I2CInit();
	UART_Init();
	ds18b20_init_x();
	Seg_Init();
	

	
	//UART
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//TIM
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	
	AT24C02_Read(N_R.N_8, 0, 2);
	
	
	
  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
		Seg_Proc();
  }

}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)	return;
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
			if(Temp_Compare_Disp != Temp_Compare || AO_Seclet_Disp != AO_Seclet)
			{
				N_R.N_16++;
				AT24C02_Write(N_R.N_8, 0, 2);
				
				AO_Seclet = AO_Seclet_Disp;
				Temp_Compare = Temp_Compare_Disp;
			}
		}
	}
	
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			pPara++;
			pPara %= 2;
		}
		
		if(pPara == 0)
		{
			if(Key_Val == 3)
			{
					Temp_Compare_Disp++;
					if(Temp_Compare_Disp > 40)
						Temp_Compare_Disp = 40;
			}
			if(Key_Val == 4)
			{
					Temp_Compare_Disp--;
					if(Temp_Compare_Disp < 20)
						Temp_Compare_Disp = 20;
			}		
		}
		else
		{
			if(Key_Val == 3)
			{
					AO_Seclet_Disp++;
					if(AO_Seclet_Disp > 1)
						AO_Seclet_Disp = 1;
			}
			if(Key_Val == 4)
			{
					AO_Seclet_Disp--;
					if(AO_Seclet_Disp > 200)
						AO_Seclet_Disp = 0;
			}	
		}

	}

}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)	return;
	uwTick_Lcd_Set_Point = uwTick;
	
	AO1 = ADC_GetPR5() * 3.3 / 4095;
	AO2 = ADC_GetPR6() * 3.3 / 4095;
	Temp = ds18b20_read() / 16.0;

	if(Temp > Temp_Compare)
		Temp_Flag = 1;
	else
		Temp_Flag = 0;

	if(AO_Seclet == 0)
	{
		if(AO1 > 3.3f * PWM_Duty)
			AO_Flag = 1;
		else 
			AO_Flag = 0;
	}
	else if(AO_Seclet == 1)
	{
		if(AO2 > 3.3f * PWM_Duty)
			AO_Flag = 1;
		else 
			AO_Flag = 0;
	}
	

	
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "       Main   %d   ", AO_Flag);
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "  AO1:%.2fV      ", AO1);
		LCD_DisplayStringLine(Line2, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "  AO2:%.2fV      ", AO2);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "  PWM2:%.0f      ", PWM_Duty * 100);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "  Temp:%.2fC      ", Temp);
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "  N:%d         ", N_R.N_16);
		LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);
	}
	else if(Mode == 1)
	{
		sprintf(LCD_ShowStr, "       Para      ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "  T:%d           ", Temp_Compare_Disp);
		if(pPara == 0)
			LCD_SetBackColor(Yellow);
		else if(pPara == 1)
			LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line2, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "  X:AO%1d           ", AO_Seclet_Disp + 1);
		if(pPara == 1)
			LCD_SetBackColor(Yellow);
		else if(pPara == 0)
			LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		
		LCD_SetBackColor(Black);
	}

}

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 200)	return;
	uwTick_Uart_Set_Point = uwTick;
	
	
	Uart_Count_Point++;
	if(Uart_Count_Point > 5 && AO_Flag)
	{
		printf("&%.2f\r\n", Temp);
		Uart_Count_Point = 0;
	}
	
	if(uwTick - Uart_Set_Point >= 200 && uwTick - Uart_Set_Point < 400)
	{
		if(Rx_Buf[0] == 'A' && Rx_Buf[1] == 'T' && Rx_Buf[2] == 0x0D && Rx_Buf[3] == 0x0A)
		{
			printf("&%.2f\r\n", Temp);
		}
		else if(Rx_Buf[0] == 'P' && Rx_Buf[1] == 'A' && Rx_Buf[2] == 'R' && Rx_Buf[3] == 'A' 
			&& Rx_Buf[4] == 0x0D && Rx_Buf[5] == 0x0A)
		{
			printf("#%d,AO%d\r\n", Temp_Compare_Disp, AO_Seclet_Disp + 1);
		}
		else
		{
			printf("no");
		}
		
		Uart_Flag = 0;
		pRx_Buf = 0;
	}

}	

void Seg_Proc(void)
{
	if(uwTick - uwTick_Seg_Set_Point < 2000)	return;
	uwTick_Seg_Set_Point = uwTick;
	
	Seg_Mode++;
	Seg_Mode %= 2;
	
	if(Seg_Mode == 0)
	{
		Seg_Display_Value(12, Temp_Compare / 10, Temp_Compare % 10);
	}
	else if(Seg_Mode == 1)
	{
		Seg_Display_Value(10, 0, AO_Seclet + 1);
	}

}	

//CallBack
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM_T = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
			PWM_Duty = (float)PWM_t / PWM_T;
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(pRx_Buf == 0)
	{
		Uart_Set_Point = uwTick;
		Uart_Flag = 1;
	}
	if(Uart_Flag == 1)
	{
		Rx_Buf[pRx_Buf] = Rx_Buffer;
		pRx_Buf++;
	}

	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);	
	
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
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
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
