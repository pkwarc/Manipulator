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
#include <stdlib.h>



#define MAX_MOVES 1000
#define JOYSTICK_STATE_1 HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)
#define JOYSTICK_STATE_2 HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)


typedef enum {
	F1,
	F2,
	Z,
	U
} Axis;

const char* axis_names[] = {"F1", "F2", "Z", "U"};
const char* dir_names[] = {"CLK", "ANT", "STOP"};

typedef enum {
	CLOCKWISE,
	ANTICLOCKWISE,
	STOP
} Direction;


void begin_movement();
void start_position();
void rotate(Axis axis, Direction direction, int32_t value);

TIM_HandleTypeDef timer;
TIM_HandleTypeDef timer_FI2;
TIM_HandleTypeDef timer_U;

TIM_Encoder_InitTypeDef encoder;
TIM_Encoder_InitTypeDef encoder_FI2;
TIM_Encoder_InitTypeDef encoder_U;

UART_HandleTypeDef uart;
char message[50];

//direction to PA_9 -- step pulse to PA_8


struct move {
    int32_t rotation;
    Direction direction;
    Axis axis;
};


struct move moves[MAX_MOVES];


unsigned int current_move = 0;
// volatile must be applied because the variable is changed during the interrupt routine
volatile int in_move = 0;
volatile Direction dir = CLOCKWISE;
volatile int man_start = 0;


int32_t abs_val(int32_t a) {
	if (a < 0) {
		return -a;
	}
	return a;
}


void send_string(char* s)
{
 HAL_UART_Transmit(&uart, (uint8_t*)s, strlen(s), 1000);
}


void F1_start_clk() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}


void F1_start_ant() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
}


void F1_stop() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}

void F2_start_clk() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
}


void F2_start_ant() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
}


void F2_stop() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
}

void U_start_clk() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}


void U_start_ant() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}


void U_stop() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}

void start_position()
{
	int32_t start_position_distance = 0;

	for (int i = 0; i < current_move; i++)
	{
		if (moves[i].direction == CLOCKWISE)
		{
			start_position_distance += moves[i].rotation;
		}
		else
		{
			start_position_distance -= moves[i].rotation;
		}
	}
	// if start_position_distance is positive then clockwise otherwise anti
	sprintf(message, "start position distance: %d\n", start_position_distance);
	send_string(message);
	if (start_position_distance > 0)
	{
		rotate(F1, ANTICLOCKWISE, abs_val(start_position_distance));
	}
	else
	{
		rotate(F1, CLOCKWISE, abs_val(start_position_distance));
	}
}


void begin_movement()
{
	TIM2->CNT = 0;
	struct move next;
	start_position();
	TIM2->CNT = 0;
	send_string("begin_movement\n");
	while(1)
	{
		for (int i = 0; i < current_move; i++)
		{
			next = moves[i];
			sprintf(message, "Move: %d, rotation = %d", next.rotation);
			rotate(next.axis, next.direction, next.rotation);
			TIM2->CNT = 0;
		}
		start_position();
		TIM2->CNT = 0;
	}
}




void rotate(Axis ax, Direction dir, int32_t value)
{
    int32_t old_val = 0;
    int32_t total = 0;
    // 435 800
    old_val = TIM2->CNT;


	if (ax == F1)
	{
		if (dir == CLOCKWISE)
		{
	    	F1_start_clk();
		}
		else
		{
			F1_start_ant();
		}
		while (total < value)
		{
			int32_t diff = TIM2->CNT - old_val;
			if (diff < 3000 && diff > -3000)
				total += abs_val(diff);

			sprintf(message, "diff = %d\n", diff);
			send_string(message);
			sprintf(message, "Suma: %d\n", total);
			send_string(message);
			old_val = TIM2->CNT;

			sprintf(message, "Zostalo: %d\n", value - total);
			send_string(message);
			HAL_Delay(50);
		 }
    	 F1_stop();
	}
}


void EXTI15_10_IRQHandler()
{
	if (in_move)
	{
		in_move = 0;
	}
	else if (!in_move)
	{
		in_move = 1;
	}

	send_string("Interrupt! \n");
	EXTI->PR = EXTI_PR_PR13;
}

void EXTI0_IRQHandler()
{
	send_string("Manipulator movement start..\n");
	man_start = 1;
	EXTI->PR &= ~EXTI_PR_PR0;
}

void EXTI1_IRQHandler()
{
	send_string("Register movement start..\n");
	EXTI->PR &= ~EXTI_PR_PR1;
}

void EXTI2_IRQHandler()
{
	send_string("CLOCKWISE! \n");
	dir = CLOCKWISE;
	EXTI->PR &= ~EXTI_PR_PR2;
}

void EXTI3_IRQHandler()
{
	send_string("ANTICLOCKWISE! \n");
	dir = ANTICLOCKWISE;
	EXTI->PR &= ~EXTI_PR_PR3;
}

void EXTI4_IRQHandler()
{
	send_string("Axis F1 Button..\n");
	EXTI->PR &= ~EXTI_PR_PR4;
}

void EXTI9_5_IRQHandler()
{
	if (EXTI->PR & EXTI_PR_PR5)
	{
		send_string("Axis F2 Button..\n")
	}
	else if (EXTI->PR & EXTI_PR_PR6)
	{
		send_string("Axis U Button..\n")
	}

	EXTI->PR &= ~EXTI_PR_PR5;
	EXTI->PR &= ~EXTI_PR_PR6;
}


void TIM2_IRQHandler()
{
	send_string("clock interrupt\n");
	TIM2->SR &= ~TIM_SR_UIF;
}

int get_direction()
{
	if (!JOYSTICK_STATE_1 && !JOYSTICK_STATE_2)
	{
		return STOP;
	}
	if (JOYSTICK_STATE_1)
	{
		return CLOCKWISE;
	}
	if (JOYSTICK_STATE_2)
	{
		return ANTICLOCKWISE;
	}
}

void print_moves()
{
	if (current_move == 0)
	{
		return;
	}

	send_string("-------------------\n");
	for (int i = 0; i < current_move; i++)
	{
		sprintf(message, "{%s, %s, %d}\n", axis_names[moves[i].axis], dir_names[moves[i].direction], moves[i].rotation);
		send_string(message);
	}
	send_string("-------------------\n");
}

// TIM1 PIN PA8 PA9 FI_1
// TIM2 PIN PA0 PA1 FI_2
// TIM3 PIN PA6 PA7
// TIM4 PIN PB6 PB7

// F1 PINS: PA10, PA11, PA12, PA13, PA14, PA15
// F2 PINS: PC10, PC11, PC12, PC13, PC14, PC15
// U  PINS: PB10, PB11, PB12, PB13, PB14, PB15

// BUTTON S PB0
// BUTTON T PB1
// JOYSTICK FORWARD PB2
// JOYSTICK BACKWARD PB3
// AXIS F1 BUTTON PB4
// AXIS F2 BUTTON PB5
// AXIS U BUTTON PB6
// DIOD F1 AXIS PC0
// DIOD F2 AXIS PC1
// DIOD U AXIS PC2

// Possible states for S and T buttons:
// S | T
// 0 | 0 Manipulator off
// 1 | 0 Manipulator movement
// 1 | 1 For now no action
// 0 | 1 Register movement

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

	GPIO_InitTypeDef gpio;

	// GPIO A
	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
	    	| GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio);
	// GPIO A

	// GPIO B
		// output
	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio);
		// input internal pull up no external resistors needed.
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_IT_FALLING;
	gpio.Pull = GPIO_PULLUP;
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_GPIO_Init(GPIOB, &gpio);
	// GPIO B


	// GPIO C
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
	    	| GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio);
	// GPIO C

	// GPIO D
//	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
//	    	| GPIO_PIN_15;
//	gpio.Mode = GPIO_MODE_OUTPUT_PP;
//	gpio.Pull = GPIO_NOPULL;
//	gpio.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOA, &gpio);
	// GPIO D

	// BOARD BUTTON UWAGA czy PC15 nie jest tylko do buttona?
//	gpio.Pin = GPIO_PIN_13;
//	gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
//	gpio.Pull = GPIO_PULLUP;
//	HAL_GPIO_Init(GPIOC, &gpio);
//	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	// BOARD BUTTON

	gpio.Pin = GPIO_PIN_9 | GPIO_PIN_8 | GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_IT_RISING;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &gpio);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);


	// AXIS F1 ENCODER
	// PA0 (bialy) do niebieskiego (enkoder)
	// PA1 (zolty) do zoltego (enkoder)`
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
	// ENCODER F1

	// AXIS F2 ENCODER
	// TIM1
	// PA8 (bialy) do niebieskiego (enkoder)
	// PA9 (zolty) do zoltego (enkoder)`
	GPIO_InitTypeDef gpio_timer_FI2;
	__HAL_RCC_TIM1_CLK_ENABLE();

	gpio_timer_FI2.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	gpio_timer_FI2.Mode = GPIO_MODE_AF_PP;
	gpio_timer_FI2.Pull = GPIO_PULLDOWN;
	gpio_timer_FI2.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &gpio_timer_FI2);

	timer_FI2.Instance = TIM1;
	timer_FI2.Init.Period = 0xffff;
	timer_FI2.Init.Prescaler = 0;
	timer_FI2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_FI2.Init.CounterMode = TIM_COUNTERMODE_UP;

	encoder_FI2.EncoderMode = TIM_ENCODERMODE_TI12;
	encoder_FI2.IC1Filter = 0x0f;
	encoder_FI2.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	encoder_FI2.IC1Prescaler = TIM_ICPSC_DIV4;
	encoder_FI2.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	encoder_FI2.IC2Filter = 0x0f;
	encoder_FI2.IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	encoder_FI2.IC2Prescaler = TIM_ICPSC_DIV4;
	encoder_FI2.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	HAL_TIM_Encoder_Init(timer_FI2, &encoder_FI2);
	HAL_TIM_Encoder_Start(timer_FI2,TIM_CHANNEL_1);
	TIM1->EGR = 1;           // Generate an update event
	TIM1->CR1 = 1;           // Enable the counter
	// ENCODER F2

	// AXIS U ENCODER
	// TIM3
	// PA6 (bialy) do niebieskiego (enkoder)
	// PA7 (zolty) do zoltego (enkoder)`
	GPIO_InitTypeDef gpio_timer_U;
	__HAL_RCC_TIM3_CLK_ENABLE();

	gpio_timer_U.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio_timer_U.Mode = GPIO_MODE_AF_PP;
	gpio_timer_U.Pull = GPIO_PULLDOWN;
	gpio_timer_U.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &gpio_timer_U);

	timer_U.Instance = TIM3;
	timer_U.Init.Period = 0xffff;
	timer_U.Init.Prescaler = 0;
	timer_U.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_U.Init.CounterMode = TIM_COUNTERMODE_UP;

	encoder_U.EncoderMode = TIM_ENCODERMODE_TI12;
	encoder_U.IC1Filter = 0x0f;
	encoder_U.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	encoder_U.IC1Prescaler = TIM_ICPSC_DIV4;
	encoder_U.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	encoder_U.IC2Filter = 0x0f;
	encoder_U.IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	encoder_U.IC2Prescaler = TIM_ICPSC_DIV4;
	encoder_U.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	HAL_TIM_Encoder_Init(timer_U, &encoder_U);
	HAL_TIM_Encoder_Start(timer_U,TIM_CHANNEL_1);
	TIM3->EGR = 1;           // Generate an update event
	TIM3->CR1 = 1;           // Enable the counter
	// ENCODER U

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

	send_string("Program is starting...\n");

	int32_t old_val = 0;
	int32_t total = 0;
	// 435 800
	old_val = TIM2->CNT;
	int counter = 0;

	while(1)
	{
		if (in_move)
		{
			if (dir == CLOCKWISE)
				F1_start_clk();
			else
				F1_start_ant();
		}
		while (in_move)
		{
			int32_t diff = TIM2->CNT - old_val;
			if (diff < 3000 && diff > -3000)
				total += abs_val(diff);

			sprintf(message, "diff = %d\n", diff);
			send_string(message);
			sprintf(message, "Sum: %d\n", total);
			send_string(message);
			old_val = TIM2->CNT;
			HAL_Delay(50);
		}
		if (!in_move)
		{
			if (total > 0)
			{
				struct move mv;
				mv.axis = F1;
				mv.direction = dir;
				mv.rotation = total;
				moves[current_move++] = mv;
				total = 0;
			}
		}
		if (man_start)

		{
			begin_movement();
		}

		if (counter > 100)

		{
			print_moves();
			counter = 0;
		}
		HAL_Delay(50);
		counter++;
	}
}
