#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "tim\tim.h"
#include "uart\uart.h"
#include "lcd\lcd.h"

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);

//Point
uint32_t							uwTick_Key_Set_Point;
uint32_t							uwTick_Lcd_Set_Point;
uint32_t							uwTick_Uart_Set_Point;

uint32_t							Uart_Tim;

//Key
uint8_t								Key_Val;
uint8_t								Key_Down;
uint8_t								Key_Up;
uint8_t								Key_Old;

//Led
uint8_t								uwLED;

//Lcd
char									LCD_ShowStr[21];

//main
typedef struct
{
	uint8_t							xNBR[4];
	uint8_t							CarName[4];
	uint8_t							TimeStr[12];
}xNBRtypedef;

xNBRtypedef						Car[8];
xNBRtypedef						Car_Buf;
uint8_t								pCar;

uint8_t								i;
uint8_t								j;
uint8_t								Mode = 0;
uint8_t								PWM_Mode = 0;
uint8_t								CNBR = 0;
uint8_t								VNBR = 0;
uint8_t								IDLE = 8;

float									CNBR_Money_H = 3.5;
float									VNBR_Money_H = 2.0;
float									CNBR_Money;
float									VNBR_Money;

uint8_t								Rx_Buffer;
uint8_t								Rx_Buf[100] = {0};
uint8_t								pRx_Buf;
uint8_t								Rx_Status_Flag;
uint8_t								Tx_Status_Flag;
uint8_t								Car_Flag;


int										Day;
int										Hour;
int										Min;


int main(void)
{

  HAL_Init();
  SystemClock_Config();
	
	Key_Led_Init();
	TIM17_Init();
	UART_Init();
	LCD_Init();
	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//Tim
	HAL_TIM_Base_Start(&htim17);
	HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
	
	//Uart
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);

  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
		
  }

}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)			return;
	uwTick_Key_Set_Point = uwTick;

	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	if(Key_Down == 1)
	{
		if(Mode == 0)
		{
			Mode = 1;
		}
		else if(Mode == 1)
		{
			Mode = 0;
		}
	}
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			CNBR_Money_H += 0.5f;
			VNBR_Money_H += 0.5f;
		}
		if(Key_Down == 3)
		{
			CNBR_Money_H -= 0.5f;
			VNBR_Money_H -= 0.5f;
		}
	}
	
	
	if(Key_Down == 4)
	{
		if(PWM_Mode == 0)
		{
			PWM_Mode = 1;
			HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
			uwLED |= 0x02;
		}
		else if(PWM_Mode == 1)
		{
			PWM_Mode = 0;
			HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
			uwLED &= ~0x02;
		}
	}
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)			return;
	uwTick_Lcd_Set_Point = uwTick;

	LED_Disp(uwLED);
	IDLE = 8 - CNBR - VNBR;
	if(IDLE > 0)
	{
		uwLED |= 0x01;
	}
	else if(IDLE == 0)
	{
		uwLED &= ~0x01;
	}
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "       %d;%d;%d    ", Day, Hour, Min);
		LCD_DisplayStringLine(Line0, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "       Data         ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   CNBR:%d          ", CNBR);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   VNBR:%d          ", VNBR);
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   IDLE:%d          ", IDLE);
		LCD_DisplayStringLine(Line7, (uint8_t * )LCD_ShowStr);
		
	}
	else if(Mode == 1)
	{
		sprintf(LCD_ShowStr, "       Para         ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   CNBR:%.2f        ", CNBR_Money_H);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   VNBR:%.2f        ", VNBR_Money_H);
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "                    ");
		LCD_DisplayStringLine(Line7, (uint8_t * )LCD_ShowStr);
	}
}

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 100)			return;
	uwTick_Uart_Set_Point = uwTick;
	

	if(uwTick - Uart_Tim >= 200 && uwTick - Uart_Tim < 500 && Rx_Status_Flag == 1)
	{
		if(pRx_Buf == 22 && Rx_Buf[4] == 0x3A && Rx_Buf[9] == 0x3A)
		{
			while(1)
			{
				
				if(Car_Flag == 0)
				{
					Car_Buf.xNBR[i] = Rx_Buf[i];
					i++;
					if(i==4)
					{
						Car_Flag = 1;
						i++;
					}
				}
				else if(Car_Flag == 1)
				{
					Car_Buf.CarName[i-5] = Rx_Buf[i];
					i++;
					if(i==9)
					{
						Car_Flag = 2;
						i++;
					}
				}
				else if(Car_Flag == 2)
				{
					Car_Buf.TimeStr[i-10] = Rx_Buf[i];
					i++;
					if(i==22)
					{
						break;
					}
				}
			}
			i = 0;
			Car_Flag = 0;
			//查有位
			for(i=0;i<8;i++)
			{
				if((pCar & (0x01 << i)) != 0)
				{
					if(Car_Buf.CarName[0] == Car[i].CarName[0]
		  	   &&Car_Buf.CarName[1] == Car[i].CarName[1]
					 &&Car_Buf.CarName[2] == Car[i].CarName[2]
				 	 &&Car_Buf.CarName[3] == Car[i].CarName[3]
					)
					{
						pCar &= ~(0x01 << i);
						Car_Flag = 1;
						Day = (int)((Car_Buf.TimeStr[4] - 0x30)*10 + (Car_Buf.TimeStr[5] - 0x30))
								- (int)((Car[i].TimeStr[4] - 0x30)*10 + (Car[i].TimeStr[5] - 0x30));
						Hour = (int)((Car_Buf.TimeStr[6] - 0x30)*10 + (Car_Buf.TimeStr[7] - 0x30))
								- (int)((Car[i].TimeStr[6] - 0x30)*10 + (Car[i].TimeStr[7] - 0x30));
						Min = (int)(Car_Buf.TimeStr[8] - 0x30)*10 + (Car_Buf.TimeStr[9] - 0x30)
								- (int)((Car[i].TimeStr[8] - 0x30)*10 + (Car[i].TimeStr[9] - 0x30));
						if(Min > 0)
						{
							Hour += 1;
						}
						if(Day > 0)
						{
							Hour += Day*24;
						}	
						if(Car[i].xNBR[0] == 67)
						{
							CNBR--;
							CNBR_Money = Hour * CNBR_Money_H;
							
							sprintf(LCD_ShowStr, "%d:%.2f\r\n", Hour, CNBR_Money);
							HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
						}
						else if(Car[i].xNBR[0] == 86)
						{
							VNBR--;
							VNBR_Money = Hour * VNBR_Money_H;
							sprintf(LCD_ShowStr, "%.2f\r\n", VNBR_Money);
							HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
						}
						
						sprintf(LCD_ShowStr, "%s\r\n", Car[i].xNBR);
						HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
						sprintf(LCD_ShowStr, "%s\r\n", Car_Buf.xNBR);
						HAL_UART_Transmit(&huart1, (uint8_t *)LCD_ShowStr, strlen(LCD_ShowStr), 50);
					}
				}
			}
			//找空位
			if(Car_Flag == 0)
			{
				for(i=0;i<8;i++)
				{
					if((pCar & (0x01 << i)) == 0)
					{
						pCar |= (0x01 << i);
						for(j=0;j<4;j++)
						{
							Car[i].xNBR[j] = Car_Buf.xNBR[j];
						}
						for(j=0;j<4;j++)
						{
							Car[i].CarName[j] = Car_Buf.CarName[j];
						}
						for(j=0;j<12;j++)
						{
							Car[i].TimeStr[j] = Car_Buf.TimeStr[j];
						}
						if(Car[i].xNBR[0] == 67)
						{
							CNBR++;
						}
						else if(Car[i].xNBR[0] == 86)
						{
							VNBR++;
						}
						break;
					}
				}
			}
		
			i = 0;
			Car_Flag = 0;
		}
		else
		{
			HAL_UART_Transmit(&huart1, (uint8_t * )"Error", 5, 50);
		}
		pRx_Buf = 0;
		Rx_Status_Flag = 0;
		Uart_Tim = 0;
	}
}



//Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(pRx_Buf == 0)
	{
		Uart_Tim = uwTick;
		Rx_Status_Flag = 1;
	}
	if(Rx_Status_Flag == 1)
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
