
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "motor.h"
#include "robot.h"
#include "common.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
//  GLobal variables
uint32_t volatile g_sysTicks_50ms = 0;		// Externed in common.h
uint32_t volatile g_sysTicks_100ms = 0;	// Externed in common.h
// Local variables
uint32_t sysTick_50ms = 0;
uint32_t sysTick_100ms = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */	
	//Receive dummy data via UART2
	uint8_t initialRxBuffer[8];
  //CMD_HandlerTypeDef UART2_CMD;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM9_Init();
  MX_TIM12_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
	printf("PooNim Firmware\n");
	
	//Start UART2 receive in non-blocking mode
	HAL_UART_Receive_IT(&huart2, (uint8_t *)initialRxBuffer, sizeof(initialRxBuffer)); 	//Dummy receive data to get thing started
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)initialRxBuffer, sizeof(initialRxBuffer));	//Dummy receive data to get thing started
	//Start Timer5 interrupt
	HAL_TIM_Base_Start_IT(&htim5);	//20Hz interrupt

	//Some Hardware check
	//SystemCheck();
	// Init system state-machine
	uint8_t systemState = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		uint32_t ticks_50ms = g_sysTicks_50ms;
		uint32_t ticks_100ms = g_sysTicks_100ms;
		
		/*------------------------------*/
		/*		Periodic function call		*/
		/*------------------------------*/
		if(sysTick_50ms != ticks_50ms)
		{
			// 20 Hz call
			//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);	//Toggle PooNim's on-board Red LED, PD14
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);	//Set PooNim's on-board Red LED, PD14 High
			
			// Update encoder reading
			MotorUpdate_Encoder();
			// Motor control-loop
			MotorControl_PID(&motor1);
			MotorControl_PID(&motor2);
			MotorControl_PID(&motor3);
			MotorControl_PID(&motor4);
			
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);	//Set PooNim's on-board Red LED, PD14 Low
			
			sysTick_50ms = ticks_50ms;
		}
		
		if(sysTick_100ms != ticks_100ms)
		{
			// 10 Hz call
			// System State-Machine Update
			systemState = systemSM_Update(systemState);

			sysTick_100ms = ticks_100ms;
		}
		
		/*
		//Process UART2 message
		UART2_CMD = GetSerialCMD();
		switch(UART2_CMD.cmd_id) {
			case 0x31:
				//Set PooNim speed
				//received cmd as (vel_x,vel_y,rot_w)
				//convert received uint8_t to float -> (float)(((int8_t)(UART2_CMD.data[0])*1.0f)/INT8_MAX)
				PooNim_CMD.vel_x = (float)(((int8_t)(UART2_CMD.data[0])*1.0f)/INT8_MAX);
				PooNim_CMD.vel_y = (float)(((int8_t)(UART2_CMD.data[1])*1.0f)/INT8_MAX);
				PooNim_CMD.rot_w = (float)(((int8_t)(UART2_CMD.data[2])*1.0f)/INT8_MAX);
			
				float wheel_speed[4];
				Robot_CalWheelSpeed(&PooNim_CMD, wheel_speed);
			
				//printf("speed cmd:: (%.3f, %.3f, %.3f) | Wheel speed:: (%.3f, %.3f, %.3f, %.3f)\n", 
				//	PooNim_CMD.vel_x, PooNim_CMD.vel_y, PooNim_CMD.rot_w, wheel_speed[0], wheel_speed[1], wheel_speed[2], wheel_speed[3]);
			
				MotorSet_speed(&motor1, wheel_speed[0]);
				MotorSet_speed(&motor2, wheel_speed[1]);
				MotorSet_speed(&motor3, wheel_speed[2]);
				MotorSet_speed(&motor4, wheel_speed[3]);
				
				printf("%i\t%i\t%i\t%i\n", motor1.Encoder_value, motor2.Encoder_value, motor3.Encoder_value, motor4.Encoder_value);
				break;
			
			case 0x32:
				//Do something
				break;
			
			default:
				//What to do? Print out message for now.
				printf("Unknown command ID, recieved: %x\n", UART2_CMD.cmd_id);
		}
		
		//Process Button state
		//Green
		if(button_G.ButtonState)
		{
			//Do something
			//button_G.PressedCount = 0;		//clear count
			//button_G.ButtonState = false;	//clear state
		}
		
		//Blue
		if(button_B.ButtonState)
		{
			//Do something
			//button_B.PressedCount = 0;		//clear count
			//button_B.ButtonState = false;	//clear state
		}
		
		//Red
		if(button_R.ButtonState)
		{
			//Do something
			//button_R.PressedCount = 0;		//clear count
			//button_R.ButtonState = false;	//clear state
		}
		
		//Orange
		if(button_O.ButtonState)
		{
			//Run system check function
			SystemCheck();
			button_O.PressedCount = 0;		//clear count
			button_O.ButtonState = false;	//clear state
		}
		
		//printf("Encoder:\t%i\t%i\t%i\t%i\n", motor1.Encoder_value, motor2.Encoder_value, motor3.Encoder_value, motor4.Encoder_value);
		HAL_Delay(20);
		*/
		
	}	//  While=loop
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* EXTI4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  /* EXTI9_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* TIM5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

/* USER CODE BEGIN 4 */

uint8_t systemSM_Update(uint8_t systemState)
{
		switch(systemState){
			case(0):
				// case 0
				// Initialize parameters and variables
				printf("Case 0\n");
				// --Init motors
				InitMotors();
				printf("Initialize Motors\n");
				// --Init Buttons
				InitButtons();
				printf("Initialize Buttons\n");
				// Move to next state
				systemState = 1;
				
			break;
			
			case(1):
				// case 1 -- Get State from button press
				printf("Case 1\n");
				// -- Turn all buttons LED on
				ButtonLED_set(&button_R);	// Green
				ButtonLED_set(&button_G);	// Red
				ButtonLED_set(&button_B);	// Blue
				ButtonLED_set(&button_O);	// Orange
			
				if(button_R.ButtonState){
					button_R.ButtonState = false;	// Reset button state
					systemState = 2;	//Motor Test
					ButtonLED_reset(&button_G);	// Green
					ButtonLED_reset(&button_B);	// Blue
					ButtonLED_reset(&button_O);	// Orange
				}else if(button_G.ButtonState){
					button_G.ButtonState = false;	// Reset button state
					systemState = 3;	//Comm Test
					ButtonLED_reset(&button_R);	// Red
					ButtonLED_reset(&button_B);	// Blue
					ButtonLED_reset(&button_O);	// Orange
				}else if(button_B.ButtonState){
					button_B.ButtonState = false;	// Reset button state
					systemState = 4;	//Sensor Test
					ButtonLED_reset(&button_G);	// Green
					ButtonLED_reset(&button_R);	// Red
					ButtonLED_reset(&button_O);	// Orange
				}else if(button_O.ButtonState){
					button_O.ButtonState = false;	// Reset button state
					systemState = 5;	//Joystick Control
					ButtonLED_reset(&button_G);	// Green
					ButtonLED_reset(&button_R);	// Red
					ButtonLED_reset(&button_B);	// Blue
					}
			break;
			
			case(2):
				// case 2 -- Motors Test
				printf("Case 2\n");
				// Set Motor speed set point, in term of encoder count per update
				MotorSet_Setpoint(&motor1, 50);
				MotorSet_Setpoint(&motor2, 50);
				MotorSet_Setpoint(&motor3, 50);
				MotorSet_Setpoint(&motor4, 50);
			
				//MotorSet_speed(&motor1, 0.2);
				//MotorSet_speed(&motor2, 0.2);
				//MotorSet_speed(&motor3, 0.2);
				//MotorSet_speed(&motor4, 0.2);
						
				printf("Encoder : %i\t%i\t%i\t%i\n", motor1.Encoder_value, motor2.Encoder_value, motor3.Encoder_value, motor4.Encoder_value);
				printf("Error : %f\t%f\t%f\t%f\n", motor1.Error, motor2.Error, motor3.Error, motor4.Error);
			break;
			
			case(3):
				// case 3 -- Communcation Test
				printf("Case 3\n");
				printf("Transmit USART at 10Hz\n");
				uint8_t test_TxBuffer[12];
				test_TxBuffer[0] = 0x5B;	// '['
				test_TxBuffer[11] = 0x5D;	// ']'
				HAL_UART_Transmit_IT(&huart2, (uint8_t *)test_TxBuffer, sizeof(test_TxBuffer));
			
			break;
			
			case(4):
				// case 4 - Sensors Test
				printf("Case 4\n");
				
			
			break;
			
			case(5):
			// case 5 -- Serial Control
				printf("Case 5\n");
				
			
			break;
			
			default:
				// Default state
			break;
		}// switch case
		
		//Return systemState
		return systemState;
}


void InitMotors(void)
{
	//	Start required components for motors
	//	eg. Timer for PWM, Timer for Encoder
	
	//Start TIM9 for PWM1 and PWM2, 5kHz
	HAL_TIM_Base_Start(&htim9);
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);	//PWM1
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);	//PWM2
	
	//Start TIM12 for PWM3 and PWM4 generation, 5kHz
	HAL_TIM_Base_Start(&htim12);
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);	//PWM3
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);	//PWM4
	
	//Start TIM1 for Encoder1 interface
	HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_ALL);	//Start Timer1 to count encoder pulses
	__HAL_TIM_SetCounter(&htim1, 0);	//Set count to 0
	
	//Start TIM2 for Encoder2 interface
	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);	//Start Timer2 to count encoder pulses
	__HAL_TIM_SetCounter(&htim2, 0);	//Set count to 0
	
	//Start TIM3 for Encoder3 interface
	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);	//Start Timer3 to count encoder pulses
	__HAL_TIM_SetCounter(&htim3, 0);	//Set count to 0

	//Start TIM4 for Encoder4 interface
	HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_ALL);	//Start Timer4 to count encoder pulses
	__HAL_TIM_SetCounter(&htim4, 0);	//Set count to 0
	
	//Initialize motors
	MotorInit(&motor1, 1);
	MotorInit(&motor2, 2);
	MotorInit(&motor3, 3);
	MotorInit(&motor4, 4);
	
	//Set motor speed to zero
	MotorSet_speed(&motor1, 0.0);
	MotorSet_speed(&motor2, 0.0);
	MotorSet_speed(&motor3, 0.0);
	MotorSet_speed(&motor4, 0.0);
	
	//Reset Encoder count
	motor1.Encoder_feedback = 0;
	motor2.Encoder_feedback = 0;
	motor3.Encoder_feedback = 0;
	motor4.Encoder_feedback = 0;
}


void InitButtons(void)
{
	ButtonInit(&button_R, 1);
	ButtonInit(&button_G, 2);
	ButtonInit(&button_B, 3);
	ButtonInit(&button_O, 4);
}

void SystemCheck(void)
{
	//To do:
	//- Make it smarter, check encoder value when motor moving
	//- Check PID control instead of just set open-loop speed
	//- Test UART Transmit and Receive as well
	//- Make this check optional, by button press
	
	//Check Motors & LEDs
	MotorSet_speed(&motor1, 0.2);
	ButtonLED_set(&button_G);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor2, 0.2);
	ButtonLED_set(&button_B);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor3, 0.2);
	ButtonLED_set(&button_R);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor4, 0.2);
	ButtonLED_set(&button_O);
	HAL_Delay(1000);
	
	//Stop motor
	MotorSet_speed(&motor1, 0.0);
	ButtonLED_reset(&button_G);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor2, 0.0);
	ButtonLED_reset(&button_B);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor3, 0.0);
	ButtonLED_reset(&button_R);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor4, 0.0);
	ButtonLED_reset(&button_O);
	HAL_Delay(1000);
	
	//Turn another way
	MotorSet_speed(&motor1, -0.2);
	ButtonLED_set(&button_G);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor2, -0.2);
	ButtonLED_set(&button_B);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor3, -0.2);
	ButtonLED_set(&button_R);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor4, -0.2);
	ButtonLED_set(&button_O);
	HAL_Delay(1000);
	
	//Stop motor
	MotorSet_speed(&motor1, 0.0);
	ButtonLED_reset(&button_G);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor2, 0.0);
	ButtonLED_reset(&button_B);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor3, 0.0);
	ButtonLED_reset(&button_R);
	HAL_Delay(1000);
	
	MotorSet_speed(&motor4, 0.0);
	ButtonLED_reset(&button_O);
	HAL_Delay(1000);
}

int fputc(int ch, FILE *f)
{
	return(ITM_SendChar(ch));
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
