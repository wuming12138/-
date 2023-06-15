#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "tim\tim.h"
#include "uart\uart.h"


void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);
void Uart_Proc(void);
	
//Point
uint32_t								uwTick_Key_Set_Point;
uint32_t								uwTick_Lcd_Set_Point;
uint32_t								uwTick_Led_Set_Point;
uint32_t								uwTick_Uart_Set_Point;

//Key
uint8_t									Key_Val;
uint8_t									Key_Down;
uint8_t									Key_Up;
uint8_t									Key_Old;

//LCD
char										LCD_ShowStr[21];

//Uart
uint32_t								Rx_Time;
uint8_t									Uart_Flag;
uint8_t									Rx_Buffer;
uint8_t									Rx_Buf[50];
uint8_t									pRx_Buf;
uint16_t								Rx_Old_Password;
uint16_t								Rx_New_Password;

//main
uint8_t									Mode = 0;
uint8_t									uwLED;

//Password
uint8_t									AT_S[3] = {0};
uint16_t								Password[3] = {0};
unsigned int						Password_Right = 123;
uint8_t									Password_Right_Flag;
uint32_t								Password_Right_Tim = 0xFFFF;
unsigned int						Password_All;
unsigned int						Password_NOT_Count;
uint8_t									Password_NOT_Flag;
uint32_t								Password_NOT_Tim= 0xFFFF;


void exchange_PWM(uint16_t Prescaler, uint16_t Compare)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
	htim2.Init.Prescaler = Prescaler;
	HAL_TIM_Base_Init(&htim2);
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, Compare);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}


int main(void)
{

  HAL_Init();
  SystemClock_Config();

	UART_Init();
	Key_Led_Init();
	TIM2_Init();
	LCD_Init();
	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//Tim
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	
	//Uart
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	
  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Led_Proc();
		Uart_Proc();
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
	
	if(Mode == 0)
	{
		if(Key_Down == 1)
		{
			if(AT_S[0] == 1)
			{
				Password[0]++;
				Password[0] %= 10;
			}
			AT_S[0] = 1;
		}
		if(Key_Down == 2)
		{
			if(AT_S[1] == 1)
			{
				Password[1]++;
				Password[1] %= 10;
			}
			AT_S[1] = 1;
		}
		if(Key_Down == 3)
		{
			if(AT_S[2] == 1)
			{
				Password[2]++;
				Password[2] %= 10;
			}
			AT_S[2] = 1;
		}
		if(Key_Down == 4 && AT_S[0] == 1 && AT_S[1] == 1 && AT_S[2] == 1)
		{
			Password_All = Password[0]*100 + Password[1]*10 + Password[2];
			if(Password_All == Password_Right)
			{
				Mode = 1;
				Password_Right_Flag = 1;
				exchange_PWM(399, 10);
			}
			else
			{
				Password_NOT_Count++;
				if(Password_NOT_Count >= 3)
				{
					Password_NOT_Flag = 1;
				}
			}
		}
	}
	
	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 500)		return;
	uwTick_Lcd_Set_Point = uwTick;
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "       PSD     ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		if(AT_S[0] == 1)
		{
			sprintf(LCD_ShowStr, "    B1:%d      ", Password[0]);
			LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		}
		else if(AT_S[0] == 0)
		{
			sprintf(LCD_ShowStr, "    B1:@       ");
			LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		}
		if(AT_S[1] == 1)
		{
			sprintf(LCD_ShowStr, "    B2:%d      ", Password[1]);
			LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);
		}
		else if(AT_S[1] == 0)
		{
			sprintf(LCD_ShowStr, "    B2:@       ");
			LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);
		}
		if(AT_S[2] == 1)
		{
			sprintf(LCD_ShowStr, "    B3:%d      ", Password[2]);
			LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
		}
		else if(AT_S[2] == 0)
		{
			sprintf(LCD_ShowStr, "    B3:@       ");
			LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
		}
		
		
	}
	else if(Mode == 1)
	{
		sprintf(LCD_ShowStr, "       STA     ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "    F:2000HZ    ");
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "    D:10%       ");
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "               ");
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
	
	}
	
}

void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 100)		return;
	uwTick_Led_Set_Point = uwTick;

	if(Password_NOT_Flag == 1)
	{
		Password_NOT_Tim = uwTick;
		Password_NOT_Flag = 0;
	}
	if(uwTick - Password_NOT_Tim < 5000)
	{
		uwLED ^= 0x02;
	}
	if(Password_Right_Flag == 1)
	{
		Password_Right_Tim = uwTick;
		Password_Right_Flag = 2;
	}
	
	
	//密码正确标志位
	if(uwTick - Password_Right_Tim < 5000 && Password_Right_Flag == 2)
	{
		uwLED |= 0x01;
	}
	//5s后恢复出厂设置
	else if(Password_Right_Flag == 2)
	{
		AT_S[0] = 0;
		AT_S[1] = 0;
		AT_S[2] = 0;
		Password[0] = 0;
		Password[1] = 0;
		Password[2] = 0;
		exchange_PWM(799, 50);
		Mode = 0;
		uwLED &= ~0x01;
		Password_Right_Flag = 0;
	}
	LED_Disp(uwLED);
}


void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 100)		return;
	uwTick_Uart_Set_Point = uwTick;

	if(uwTick - Rx_Time >= 200 && uwTick - Rx_Time < 300)
	{
		if(pRx_Buf == 7 && Rx_Buf[3] == 0x2D)
		{
			Rx_Old_Password = (Rx_Buf[0] - 0x30)*100 + (Rx_Buf[1] - 0x30)*10 + (Rx_Buf[2] - 0x30);
			Rx_New_Password = (Rx_Buf[4] - 0x30)*100 + (Rx_Buf[5] - 0x30)*10 + (Rx_Buf[6] - 0x30);
			if(Rx_Old_Password == Password_Right)
			{
				Password_Right = Rx_New_Password;
			}
			else
			{
				sprintf(LCD_ShowStr, "Password:error\r\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
			}
		}
		else
		{
			sprintf(LCD_ShowStr, "error\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
		}
		
		Uart_Flag = 0;
		pRx_Buf = 0;
	}
	
}

//CallBack
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(pRx_Buf == 0)
	{
		Rx_Time = uwTick;
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
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
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
