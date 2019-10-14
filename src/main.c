/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include <string.h>
#include "stm32f1xx.h"


TIM_HandleTypeDef timer;
TIM_Encoder_InitTypeDef encoder;
UART_HandleTypeDef uart;
char message[20];

//direction to PA_9 -- step pulse to PA_8


void send_string(char* s)
{
 HAL_UART_Transmit(&uart, (uint8_t*)s, strlen(s), 1000);
}


void servo_left() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}

void servo_right() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
}

void servo_f2_start() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
}

void servo_f2_stop() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
}

void servo_f1_start() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}

void servo_f1_stop() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
}

// TIM1 PIN PA8 PA9 FI_1
// TIM2 PIN PA0 PA1 FI_2
// TIM3 PIN PA6 PA7
// TIM4 PIN PB6 PB7

int16_t abs_val(int16_t a) {
	if (a < 0) {
		return -a;
	}
	return a;
}

int main(void)
{
	     SystemCoreClock = 8000000;
	     HAL_Init();

	     // CLOCK
		     __HAL_RCC_GPIOB_CLK_ENABLE();
		     __HAL_RCC_GPIOA_CLK_ENABLE();
		     __HAL_RCC_GPIOC_CLK_ENABLE();
		     __HAL_RCC_GPIOD_CLK_ENABLE();
		     // CLOCK

	     // GPIO
	     GPIO_InitTypeDef gpio;
	     gpio.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8
	    		 | GPIO_PIN_9 | GPIO_PIN_10;
	     gpio.Mode = GPIO_MODE_OUTPUT_PP;
	     gpio.Pull = GPIO_NOPULL;
	     gpio.Speed = GPIO_SPEED_FREQ_LOW;
	     HAL_GPIO_Init(GPIOA, &gpio);
	     // GPIO

	     // BOARD BUTTON
	     gpio.Pin = GPIO_PIN_13;
	     gpio.Mode = GPIO_MODE_INPUT;
	     gpio.Pull = GPIO_PULLUP;
	     HAL_GPIO_Init(GPIOC, &gpio);

	     // BOARD BUTTON

	     /*
	     // ENCODER 1
	     	     GPIO_InitTypeDef GPIO_InitStruct;
	     	     __HAL_RCC_TIM1_CLK_ENABLE();

	     	     GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	     	     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	              GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	     	     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	     	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	     	    timer.Instance = TIM1;
	     	    timer.Init.Period = 0xffff;
	     	    timer.Init.Prescaler = 0;
	     	    timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	     	    timer.Init.CounterMode = TIM_COUNTERMODE_UP;

	     	    encoder.EncoderMode = TIM_ENCODERMODE_TI12;
	     	    encoder.IC1Filter = 0x0f;
	     	    encoder.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	     	    encoder.IC1Prescaler = TIM_ICPSC_DIV4;
	     	    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;

	     	    encoder.IC2Filter = 0x0f;
	     	    encoder.IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	     	    encoder.IC2Prescaler = TIM_ICPSC_DIV4;
	     	    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;

	     	    HAL_TIM_Encoder_Init(&timer, &encoder);

	     	    HAL_TIM_Encoder_Start(&timer,TIM_CHANNEL_1);

	     	    TIM1->EGR = 1;           // Generate an update event
	     	    TIM1->CR1 = 1;           // Enable the counter
	     	    // ENCODER

*/
	     // ENCODER 2
	     GPIO_InitTypeDef GPIO_InitStruct;
	     	     __HAL_RCC_TIM2_CLK_ENABLE();

	     	     GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	     	     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	              GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	     	     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	     	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	     	    timer.Instance = TIM2;
	     	    timer.Init.Period = 0xffff;
	     	    timer.Init.Prescaler = 0;
	     	    timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	     	    timer.Init.CounterMode = TIM_COUNTERMODE_UP;

	     	    encoder.EncoderMode = TIM_ENCODERMODE_TI12;
	     	    encoder.IC1Filter = 0x0f;
	     	    encoder.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	     	    encoder.IC1Prescaler = TIM_ICPSC_DIV4;
	     	    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;

	     	    encoder.IC2Filter = 0x0f;
	     	    encoder.IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	     	    encoder.IC2Prescaler = TIM_ICPSC_DIV4;
	     	    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;

	     	    HAL_TIM_Encoder_Init(&timer, &encoder);

	     	    HAL_TIM_Encoder_Start(&timer,TIM_CHANNEL_1);

	     	    TIM2->EGR = 1;           // Generate an update event
	     	    TIM2->CR1 = 1;           // Enable the counter
	     	    // ENCODER

	    // UART
	    	__HAL_RCC_USART2_CLK_ENABLE();
		    gpio.Mode = GPIO_MODE_AF_PP;
		    gpio.Pin = GPIO_PIN_2;
		    gpio.Pull = GPIO_NOPULL;
		    gpio.Speed = GPIO_SPEED_FREQ_LOW;
		    HAL_GPIO_Init(GPIOA, &gpio);

		    gpio.Mode = GPIO_MODE_AF_INPUT;
		    gpio.Pin = GPIO_PIN_3;
		    HAL_GPIO_Init(GPIOA, &gpio);


		    uart.Instance = USART2;
		    uart.Init.BaudRate = 115200;
		    uart.Init.WordLength = UART_WORDLENGTH_8B;
		    uart.Init.Parity = UART_PARITY_NONE;
		    uart.Init.StopBits = UART_STOPBITS_1;
		    uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		    uart.Init.OverSampling = UART_OVERSAMPLING_16;
		    uart.Init.Mode = UART_MODE_TX_RX;
		    HAL_UART_Init(&uart);
		    // UART

	    send_string("message\n");

	    while (1) {

	            int16_t count1 = 0;
	            servo_f1_start();
	            servo_f2_start();
//	            count1=TIM1->CNT;
//	            sprintf(message, "FI 1: %d\r\n", count1);
//	            send_string(message);


//	            while ((count1 = abs_val(TIM2->CNT)) < 8192) {
//
//	            	sprintf(message, "F1 2: %d\r\n", count1);
//	            		            send_string(message);
//	            }
//	            servo_stop();
//	            break;
	     };
}
