#include "main.h"
#include "string.h"
#include "stdio.h"
#include "lcd\lcd.h"
#include "adc\adc.h"
#include "uart\uart.h"
#include "key_led\key_led.h"
#include "tim\tim.h"


void SystemClock_Config(void);

uint32_t								uwTick_Key_Set_Point;
uint32_t								uwTick_Lcd_Set_Point;
uint32_t								uwTick_Uart_Set_Point;
uint32_t								Uart_Time;

void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);

//Key
uint8_t									Key_Val;
uint8_t									Key_Down;
uint8_t									Key_Up;
uint8_t									Key_Old;

//LCD
char										LCD_ShowStr[21];

//UART
char										UART_ShowStr[50];
uint8_t									Rx_Buffer;
uint8_t									Rx_Buf[50];
uint8_t									pRx_Buf;
uint8_t									UART_Flag;

//TIM
uint32_t								PULS1_Count;
uint32_t								PWM1_T;
uint32_t								PWM1_t;
uint32_t								PWM2_T;
uint32_t								PWM2_t;

//main
uint32_t								f;
uint32_t								Pf = 1000;
float										PWM1_Duty;
float										PWM2_Duty;
uint8_t									Mode;
uint8_t									T_mode;
float										a;
float										b;
float										aa[5];
float										bb[5];
float										pa[5];
float										pb[5];
float										a_old;
float										b_old;
uint16_t								ax;
uint16_t								bx;
uint16_t								Pax = 20;
uint16_t								Pbx = 20;
uint8_t									PWM_mode_Flag;
uint16_t								ADC;
uint16_t								ADC_Old;
uint8_t									uwLED;

int main(void)
{

  HAL_Init();
  SystemClock_Config();
	
	MX_GPIO_Init();
	LCD_Init();
	ADC2_Init();
	UART_Init();
	TIM2_Init();

	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//Uart
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	
	//TIM
	
  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);		  /* 启动定时器通道输入捕获并开启中断 */
	HAL_TIM_Base_Start(&htim2);
	

	

  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
  }

}

void PWM_Duty(void)
{
	uint8_t i = 0, j = 0;
	float	temp = 0;
	//PWM1
	PWM_mode_Flag = 0;
	TIM3_PWM1_Init();
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);
	HAL_Delay(10);
	if(PWM1_Duty < 10)
		a = 0;
	else if(PWM1_Duty>=10 && PWM1_Duty<=90)
		a = PWM1_Duty * 2.25f - 22.5f;
	else if(PWM1_Duty > 90)
		a = 180;
	
	HAL_TIM_IC_Stop_IT(&htim3,TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim3,TIM_CHANNEL_2);
	
	
	//PWM2
	PWM_mode_Flag = 1;
	TIM3_PWM2_Init();
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);
	HAL_Delay(10);
	
	if(PWM2_Duty < 10)
		b = 0;
	else if(PWM2_Duty>=10 && PWM1_Duty<=90)
		b = PWM2_Duty * 1.125f - 11.25f;
	else if(PWM2_Duty > 90)
		b = 90;
	
	
	HAL_TIM_IC_Stop_IT(&htim3,TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim3,TIM_CHANNEL_2);
	
	if(a >= a_old)
	{
		ax = a - a_old;
	}
	else
	{
		ax = a_old - a;
	}
	
	if(b >= b_old)
	{
		bx = b - b_old;
	}
	else
	{
		bx = b_old - b;
	}
	
	a_old = a;
	b_old = b;
	
	for(i = 4; i > 0; i--)
	{
		aa[i] = aa[i-1];
		bb[i] = bb[i-1];
	}
	
	aa[0]	= a;
	bb[0] = b;
	
	for(i = 0; i < 5; i++)
	{
		pa[i] = aa[i];
	}
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4 - i; j++)
		{
			if(pa[i] > pa[i + 1])
			{
				temp = pa[i];
				pa[i] = pa[i + 1];
				pa[i + 1] = temp;
			}
		}
	}
	
	
	for(i = 0; i < 5; i++)
	{
		pb[i] = bb[i];
	}
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4 - i; j++)
		{
			if(pb[i] > pb[i + 1])
			{
				temp = pb[i];
				pb[i] = pb[i + 1];
				pb[i + 1] = temp;
			}
		}
	}
	
}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)  return;
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
	}
	if(Key_Down == 2)
	{
		if(Mode == 1)
		{
			Pax += 10;
			Pbx += 10;
			if(Pax > 60)
			{
				Pax = 10;
				Pbx = 10;
			}
		}

	}
	if(Key_Down == 3)
	{
		if(Mode == 0)
		{
			T_mode++;
			T_mode %= 2;
		}
		else if(Mode == 1)
		{
			Pf += 1000;
			if(Pf > 10000)
			{
				Pf = 1000;
			}
		}
	}
	if(Key_Down == 4)
	{
		if(T_mode == 0)
			PWM_Duty();
	}
	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)  return;
	uwTick_Lcd_Set_Point = uwTick;
	
	f = 1000000 / PULS1_Count;
	
	ADC = ADC2_GetValue();
	
	if(ADC2_GetValue() > 2500 && T_mode == 1 && ADC - ADC_Old > 500)
		PWM_Duty();
	
	ADC_Old = ADC;
	
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "        DATA      ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   a:%.1f           ", a);
		LCD_DisplayStringLine(Line2, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   b:%.1f           ", b);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);		
		sprintf(LCD_ShowStr, "   f:%dHz         ", f);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);


		sprintf(LCD_ShowStr, "   ax:%d           ", ax);
		LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   bx:%d           ", bx);
		LCD_DisplayStringLine(Line7, (uint8_t * )LCD_ShowStr);
		if(T_mode == 0)
			sprintf(LCD_ShowStr, "   mode:A        ");
		else if(T_mode == 1)
			sprintf(LCD_ShowStr, "   mode:B        ");
		LCD_DisplayStringLine(Line8, (uint8_t * )LCD_ShowStr);
	}
	else if(Mode == 1)
	{
		sprintf(LCD_ShowStr, "        PARA      ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   Pax:%d           ", Pax);
		LCD_DisplayStringLine(Line2, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   Pbx:%d           ", Pbx);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);
		sprintf(LCD_ShowStr, "   Pf:%d         ", Pf);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);
		
	}
	
	if(ax > Pax)
		uwLED |= 0x01;
	else
		uwLED &= ~0x01;
	if(bx > Pbx)
		uwLED |= 0x02;
	else
		uwLED &= ~0x02;
	if(f > Pf)
		uwLED |= 0x04;
	else
		uwLED &= ~0x04;
	if(T_mode == 0)
		uwLED |= 0x08;
	else
		uwLED &= ~0x08;
	LED_Disp(uwLED);
	
}

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 100)  return;
	uwTick_Uart_Set_Point = uwTick;
	
	if(uwTick - Uart_Time >= 200 && uwTick - Uart_Time < 300)
	{
		if(Rx_Buf[0] == 'a' && Rx_Buf[1] == '?')
		{
			sprintf(UART_ShowStr, "a:%.1f\r\n", a);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}
		else if(Rx_Buf[0] == 'b' && Rx_Buf[1] == '?')
		{
			sprintf(UART_ShowStr, "b:%.1f\r\n", b);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}
		else if(Rx_Buf[0] == 'a' && Rx_Buf[1] == 'a' && Rx_Buf[2] == '?')
		{
			sprintf(UART_ShowStr, "aa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", aa[0], aa[1], aa[2], aa[3], aa[4]);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}
		else if(Rx_Buf[0] == 'b' && Rx_Buf[1] == 'b' && Rx_Buf[2] == '?')
		{
			sprintf(UART_ShowStr, "bb:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", bb[0], bb[1], bb[2], bb[3], bb[4]);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}		
		else if(Rx_Buf[0] == 'p' && Rx_Buf[1] == 'a' && Rx_Buf[2] == '?')
		{
			sprintf(UART_ShowStr, "pa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pa[0], pa[1], pa[2], pa[3], pa[4]);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}
		else if(Rx_Buf[0] == 'p' && Rx_Buf[1] == 'b' && Rx_Buf[2] == '?')
		{
			sprintf(UART_ShowStr, "pb:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pb[0], pb[1], pb[2], pb[3], pb[4]);
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}		
		else
		{
			sprintf(UART_ShowStr, "Error\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t * )UART_ShowStr, strlen(UART_ShowStr), 50);
		}
		
		
		
		pRx_Buf = 0;
		UART_Flag = 0;
	}
	
}



//Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(pRx_Buf == 0)
	{
		Uart_Time = uwTick;
		UART_Flag = 1;
	}
	if(UART_Flag == 1)
	{
		Rx_Buf[pRx_Buf++] = Rx_Buffer;
	}
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	
	
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PULS1_Count = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
		}
	}
	
	if(htim->Instance == TIM3 && PWM_mode_Flag == 0)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM1_T = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
			PWM1_Duty = (float)PWM1_t / PWM1_T * 100;
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM1_t = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
		}
	}
	
	if(htim->Instance == TIM3 && PWM_mode_Flag == 1)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM2_T = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
			PWM2_Duty = (float)PWM2_t / PWM2_T * 100;
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM2_t = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
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
