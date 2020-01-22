/**
  ******************************************************************************
  * @file    main.c
  * @author  P.K
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include <string.h>
#include "stm32f1xx.h"
#include <stdlib.h>


#define FALSE 0
#define TRUE 1
#define MAX_MOVES 1000

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

typedef enum {
	BTN_NONE,
	BTN_S1,
	BTN_S2,
	BTN_CLK,
	BTN_ANT,
	BTN_F1,
	BTN_F2,
	BTN_U
} BIntr;

typedef struct move {
    int32_t rotation;
    Direction direction;
    Axis axis;
} Move;

TIM_HandleTypeDef timer;
TIM_HandleTypeDef timer_FI2;
TIM_HandleTypeDef timer_U;
TIM_HandleTypeDef debounceTimer;

TIM_Encoder_InitTypeDef encoder;
TIM_Encoder_InitTypeDef encoder_FI2;
TIM_Encoder_InitTypeDef encoder_U;

UART_HandleTypeDef uart;
char message[50];

//direction to PA_9 -- step pulse to PA_8

struct move moves[MAX_MOVES];

void begin_movement();
void start_position();
void rotate(Move m);
void start_axis(Move move);
void stop_axis(Move move);
uint32_t get_axis_rotation(Axis axis);
void set_axis_rotation(Axis axis, int value);
void manipulator_config();


// volatile must be applied because the variable is changed during the interrupt routine
volatile int in_move = FALSE;
volatile int start_move = FALSE;
volatile int register_move = FALSE;
volatile int was_test_performed = FALSE;

volatile int move_index = 0;
volatile Move current_move;
volatile BIntr last_button_pushed = BTN_NONE;

int32_t abs_val(int32_t a)
{
	if (a < 0) {
		return -a;
	}
	return a;
}


void send_string(char* s)
{
	HAL_UART_Transmit(&uart, (uint8_t*)s, strlen(s), 1000);
}

void TIM4_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&debounceTimer);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_Base_Stop(&debounceTimer);
	if (last_button_pushed == BTN_CLK) {

	} else if (last_button_pushed == BTN_ANT) {

	} else if (last_button_pushed == BTN_F1) {
		f1_button_pushed();
	} else if (last_button_pushed == BTN_F2) {

	} else if (last_button_pushed == BTN_U) {

	} else if (last_button_pushed == BTN_S1) {

	} else if (last_button_pushed == BTN_S2) {

	}

	// reset the timer value
	TIM4->CNT = 0;
}

void f1_button_pushed() {
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET)
	{
		send_string("AXIS F1 ON\n");
		current_move.axis = F1;
	}
}

void F1_start_clk() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}


void F1_start_ant() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_RESET);
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
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
}


void F2_stop() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
}

void U_start_clk() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}


void U_start_ant() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}


void U_stop() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}

void start_position(int from)
{
	Move f1 = {0};
	Move f2 = {0};
	Move u = {0};

	f1.axis = F1;
	f2.axis = F2;
	u.axis = U;

	for (int i = from; i < move_index; i++)
	{
		if (moves[i].axis == F1)
		{
			if (moves[i].direction == CLOCKWISE)
			{
				f1.rotation += moves[i].rotation;
			}
			else
			{
				f1.rotation -= moves[i].rotation;
			}
		}
		else if (moves[i].axis == F2)
		{
			if (moves[i].direction == CLOCKWISE)
			{
				f2.rotation += moves[i].rotation;
			}
			else
			{
				f2.rotation -= moves[i].rotation;
			}
		}
		else if (moves[i].axis == U)
		{
			if (moves[i].direction == CLOCKWISE)
			{
				u.rotation += moves[i].rotation;
			}
			else
			{
				u.rotation -= moves[i].rotation;
			}
		}
	}

	if (u.rotation > 0)
	{
		u.direction = ANTICLOCKWISE;
		u.rotation = abs_val(u.rotation);
	}
	else
	{
		u.direction = CLOCKWISE;
		u.rotation = abs_val(u.rotation);
	}

	if (f1.rotation > 0)
	{
		f1.direction = ANTICLOCKWISE;
		f1.rotation = abs_val(f1.rotation);
	}
	else
	{
		f1.direction = CLOCKWISE;
		f1.rotation = abs_val(f1.rotation);
	}

	if (f2.rotation > 0)
	{
		f2.direction = ANTICLOCKWISE;
		f2.rotation = abs_val(f2.rotation);
	}
	else
	{
		f2.direction = CLOCKWISE;
		f2.rotation = abs_val(f2.rotation);
	}
	// if start_position_distance is positive then clockwise otherwise anti

	rotate(f1);
	rotate(f2);
	rotate(u);
}


void begin_movement()
{
	if (move_index == 0) {
		send_string("Cannot start movement - no data.\n");
		return;
	}
	set_axis_rotation(F1, 0);
	set_axis_rotation(F2, 0);
	set_axis_rotation(U, 0);

	start_position(0);

	set_axis_rotation(F1, 0);
	set_axis_rotation(F2, 0);
	set_axis_rotation(U, 0);

	rotate(moves[0]);

	set_axis_rotation(F1, 0);
	set_axis_rotation(F2, 0);
	set_axis_rotation(U, 0);

	send_string("begin_movement\n");
	while (start_move)
	{
		struct move next;
		for (int i = 1; i < move_index; i++)
		{
			next = moves[i];
			sprintf(message, "Move: %d, rotation = %d", next.rotation);
			rotate(next);
			set_axis_rotation(next.axis, 0); // err
		}
		start_position(1);
		set_axis_rotation(F1, 0);
		set_axis_rotation(F2, 0);
		set_axis_rotation(U, 0);
	}
}


void rotate(Move m)
{
    int32_t old_val = 0;
    int32_t total = 0;
    // 435 800
    old_val = get_axis_rotation(m.axis);


	start_axis(m);
	while (total < m.rotation)
	{
		int32_t diff = get_axis_rotation(m.axis) - old_val;
		if (diff < 3000 && diff > -3000)
			total += abs_val(diff);

		sprintf(message, "diff = %d\n", diff);
		send_string(message);
		sprintf(message, "Suma: %d\n", total);
		send_string(message);
		old_val = get_axis_rotation(m.axis);

		sprintf(message, "Zostalo: %d\n", m.rotation - total);
		send_string(message);
		HAL_Delay(42.5);
	}
    stop_axis(m);
}


void start_axis(Move move)
{
	if (move.axis == F1)
	{
		if (move.direction == CLOCKWISE)
		{
			F1_start_clk();
		}
		else
		{
			F1_start_ant();
		}
	}
	else if (move.axis == F2)
	{
		if (move.direction == CLOCKWISE)
		{
			F2_start_clk();
		}
		else
		{
			F2_start_ant();
		}
	}
	else if (move.axis == U)
	{
		if (move.direction == CLOCKWISE)
		{
			U_start_clk();
		}
		else
		{
			U_start_ant();
		}
	}
}


void stop_axis(Move move)
{
	if (move.axis == F1)
	{
		F1_stop();
	}
	else if (move.axis == F2)
	{
		F2_stop();
	}
	else if (move.axis == U)
	{
		U_stop();
	}
}


uint32_t get_axis_rotation(Axis axis)
{
	if (axis == F1)
	{
		return TIM1->CNT;
	}
	else if (axis == F2)
	{
		return TIM2->CNT;
	}
	else if (axis == U)
	{
		return TIM3->CNT;
	}
	return 0;
}


void set_axis_rotation(Axis axis, int value)
{
	if (axis == F1)
	{
		TIM2->CNT = value;
	}
	else if (axis == F2)
	{
		TIM1->CNT = value;
	}
	else if (axis == U)
	{
		TIM3->CNT = value;
	}
}


void EXTI0_IRQHandler()
{
	send_string("S1 ON\n");
	send_string("Register movement start..\n");

	move_index = 0;

	register_move = !register_move;
	send_string(register_move ? "TRUE\n" : "FALSE\n");

	last_button_pushed = BTN_S1;
	HAL_TIM_Base_Start(&debounceTimer);

	EXTI->PR = EXTI_PR_PR0;
}

void EXTI1_IRQHandler()
{
	send_string("S2 ON\n");
	send_string("Manipulator movement start..\n");

	start_move = !start_move;

	send_string(start_move ? "TRUE\n" : "FALSE\n");

	last_button_pushed = BTN_S2;
	HAL_TIM_Base_Start(&debounceTimer);

	EXTI->PR = EXTI_PR_PR1;
}

void EXTI2_IRQHandler()
{
	send_string("JOYSTICK FORWARD ON\n");
	send_string("CLOCKWISE! START\n");

	current_move.direction = CLOCKWISE;
	in_move = TRUE;

	last_button_pushed = BTN_CLK;
	HAL_TIM_Base_Start(&debounceTimer);

	EXTI->PR = EXTI_PR_PR2;
}

void EXTI3_IRQHandler()
{
	send_string("JOYSTICK BACKWARD ON\n");
	send_string("ANTICLOCKWISE! START\n");

	current_move.direction = ANTICLOCKWISE;
	in_move = TRUE;

	last_button_pushed = BTN_ANT;
	HAL_TIM_Base_Start(&debounceTimer);

	EXTI->PR = EXTI_PR_PR3;
}

void EXTI4_IRQHandler()
{
	send_string("AXIS F1 ON\n");
	current_move.axis = F1;

	last_button_pushed = BTN_F1;
	HAL_TIM_Base_Start(&debounceTimer);

	EXTI->PR = EXTI_PR_PR4;
}

void EXTI9_5_IRQHandler()
{
	if (EXTI->PR & EXTI_PR_PR5)
	{
		send_string("AXIS F2 ON\n");

		current_move.axis = F2;

		last_button_pushed = BTN_F2;
		HAL_TIM_Base_Start(&debounceTimer);
	}
	else if (EXTI->PR & EXTI_PR_PR6)
	{
		send_string("AXIS U ON\n");

		current_move.axis = U;

		last_button_pushed = BTN_U;
		HAL_TIM_Base_Start(&debounceTimer);
	}

	EXTI->PR = EXTI_PR_PR5;
	EXTI->PR = EXTI_PR_PR6;
}

// BUTTON S PB0
// BUTTON T PB1
// JOYSTICK FORWARD PB2
// JOYSTICK BACKWARD PC3
// AXIS F1 BUTTON PB4
// AXIS F2 BUTTON PB5
// AXIS U BUTTON PB6
void check_input_status(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
	{
		send_string("S1 ON\n");
		send_string("Register movement start..\n");

		move_index = 0;

		register_move = !register_move;
		send_string(register_move ? "TRUE\n" : "FALSE\n");
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
	{
		send_string("S2 ON\n");
		send_string("Manipulator movement start..\n");
		start_move = !start_move;
		send_string(start_move ? "TRUE\n" : "FALSE\n");
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET)
	{
		send_string("JOYSTICK FORWARD ON\n");
		send_string("CLOCKWISE! START\n");

		current_move.direction = CLOCKWISE;
		in_move = TRUE;
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET)
	{
		send_string("JOYSTICK BACKWARD ON\n");
		send_string("ANTICLOCKWISE! START\n");

		current_move.direction = ANTICLOCKWISE;
		in_move = TRUE;
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET)
	{
		send_string("JOYSTICK MIDDLE POSITION\n");
		send_string("STPOP MOVE \n");

//		was_test_performed = FALSE;
		in_move = FALSE;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET)
	{
		send_string("AXIS F1 ON\n");

		current_move.axis = F1;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET)
	{
		send_string("AXIS F2 ON\n");

		current_move.axis = F2;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_RESET)
	{
		send_string("AXIS U ON\n");

		current_move.axis = U;
	}
}


void print_moves()
{
	if (move_index== 0)
	{
		return;
	}

	send_string("-------------------\n");
	for (int i = 0; i < move_index; i++)
	{
		sprintf(message, "{%s, %s, %d}\n", axis_names[moves[i].axis], dir_names[moves[i].direction], moves[i].rotation);
		send_string(message);
	}
	send_string("-------------------\n");
}

// TIM1 PIN PA8 PA9 FI_1
// TIM2 PIN PA0 PA1 FI_2
// TIM3 PIN PA6 PA7 U
// TIM4 PIN PB6 PB7

// F1 PINS: PA10, PA11, PA12, PA13, PA14, PA15
// F2 PINS: PC10, PC11, PC12, PC13, PC14, PC15
// U  PINS: PB10, PB11, PB12, PB13, PB14, PB15

// BUTTON S1 PB0
// BUTON S2 PB1
// JOYSTICK FORWARD PB2
// JOYSTICK BACKWARD PC3
// AXIS F1 BUTTON PB4
// AXIS F2 BUTTON PB5
// AXIS U BUTTON PB6
// DIOD F1 AXIS PC0
// DIOD F2 AXIS PC1
// DIOD U AXIS PC2

// Possible states for S1 (start_move flag) and S2 (stop_move flag) buttons:
// S | S2
// 0 | 0 Manipulator off
// 1 | 0 Manipulator movement
// 1 | 1 Manipulator on axis diagnosis
// 0 | 1 Register movement

/*
 * S
 */
int main(void)
{
	manipulator_config();

	send_string("Program is starting...\n");

	int32_t old_val = 0;
	int32_t total = 0;
	// 435 800

	int counter = 0;

	F1_stop();
	F2_stop();
	U_stop();

//
//	Move test_axis;
//	test_axis.axis = F1;

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, SET);
	while(1)
	{
		check_input_status();
//		sprintf(message, "T1 = %d\n", TIM1->CNT);
//		send_string(message);
//		sprintf(message, "T2 = %d\n", TIM2->CNT);
//		send_string(message);
//		sprintf(message, "T3 = %d\n", TIM3->CNT);
//						send_string(message);
//						int32_t diff = get_axis_rotation(test_axis.axis) - old_val;
//						if (diff < 3000 && diff > -3000)
//							total += abs_val(diff);
//
//						sprintf(message, "diff = %d\n", diff);
//						send_string(message);
//						sprintf(message, "Sum: %d\n", total);
//						send_string(message);
//						old_val = get_axis_rotation(test_axis.axis);
//						HAL_Delay(42.5);
		/* rejestracja ruchu */
		while (register_move && !start_move)
		{
			if (in_move)
			{
				start_axis(current_move);
			}
			while (in_move)
			{
				int32_t diff = get_axis_rotation(current_move.axis) - old_val;
				if (diff < 3000 && diff > -3000)
					total += abs_val(diff);

				sprintf(message, "diff = %d\n", diff);
				send_string(message);
				sprintf(message, "Sum: %d\n", total);
				send_string(message);
				old_val = get_axis_rotation(current_move.axis);
				HAL_Delay(42.5);
			}
			if (!in_move)
			{
				if (total > 0)
				{
					current_move.rotation = total;
					moves[move_index++] = current_move;
					total = 0;
				}
			}
			if (counter > 100)

			{
				print_moves();
				counter = 0;
			}
			HAL_Delay(50);
			counter++;
		}

		/* odtwarzanie ruchu */
		while (start_move && !register_move)
		{
			begin_movement();
		}

		/* ruch testowy */
		while (start_move && register_move && !was_test_performed)
		{
			was_test_performed = TRUE;

			Move f1_move;
			Move f2_move;
			Move u_move;

			f1_move.direction = f2_move.direction = u_move.direction = CLOCKWISE;
			f1_move.rotation = f2_move.rotation = u_move.rotation = 1000;
			f2_move.axis = F2;
			u_move.axis = U;
			f1_move.axis = F1;

			rotate(f1_move);
			rotate(f2_move);
			rotate(u_move);

			f1_move.direction = f2_move.direction = u_move.direction = ANTICLOCKWISE;
			rotate(u_move);
			rotate(f2_move);
			rotate(f1_move);

		}
	}
}

void manipulator_config()
{
	SystemCoreClock = 8000000;
	HAL_Init();

	// CLOCK
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	// CLOCK

	// Enable PA13 PA14
	 __HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_AFIO_REMAP_SWJ_DISABLE();

//	SysTick_Config(800000);

	GPIO_InitTypeDef gpio;

	GPIO_InitTypeDef gpio_a;

	// GPIO A
	gpio_a.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
	    	| GPIO_PIN_15;
	gpio_a.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_a.Pull = GPIO_NOPULL;
	gpio_a.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_a);
	// GPIO A

	// GPIO B
	// BUTTON S PB0
	// BUTTON T PB1
	// JOYSTICK FORWARD PB2
	// JOYSTICK BACKWARD PC3
	// AXIS F1 BUTTON PB4
	// AXIS F2 BUTTON PB5
	// AXIS U BUTTON PB6
		// output
	GPIO_InitTypeDef gpio_b_output;
	gpio_b_output.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio_b_output.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_b_output.Pull = GPIO_NOPULL;
	gpio_b_output.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_b_output);

		// input axis buttons internal pull up falling.
	GPIO_InitTypeDef gpio_axis_buttons;
	gpio_axis_buttons.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
	gpio_axis_buttons.Mode = GPIO_MODE_INPUT;
	gpio_axis_buttons.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &gpio_axis_buttons);

	GPIO_InitTypeDef gpio_s1s2;
	gpio_s1s2.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	gpio_s1s2.Mode = GPIO_MODE_INPUT;
	gpio_s1s2.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &gpio_s1s2);

	//input joy stick internal pull up raising falling, PB3 doesn't work
	GPIO_InitTypeDef gpio_joystick_c;
	gpio_joystick_c.Pin = GPIO_PIN_3;
	gpio_joystick_c.Mode = GPIO_MODE_INPUT;
	gpio_joystick_c.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &gpio_joystick_c);
	
	//input joy stick internal pull up raising falling
	GPIO_InitTypeDef gpio_joystick;
	gpio_joystick.Pin = GPIO_PIN_2;
	gpio_joystick.Mode = GPIO_MODE_INPUT;
	gpio_joystick.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &gpio_joystick);

	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	// GPIO B


	// GPIO C
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
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
	HAL_TIM_Encoder_Init(&timer_FI2, &encoder_FI2);
	HAL_TIM_Encoder_Start(&timer_FI2,TIM_CHANNEL_1);
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
	HAL_TIM_Encoder_Init(&timer_U, &encoder_U);
	HAL_TIM_Encoder_Start(&timer_U,TIM_CHANNEL_1);
	TIM3->EGR = 1;           // Generate an update event
	TIM3->CR1 = 1;           // Enable the counter
	// ENCODER U

	/*
	 * Timer for debouncing
	 *
	 * This timer counts to 50ms every time a button interrupt is called.
	 * After 50ms the button state is checked and the result is assigned
	 * to the variable related to this button state.
	 */


	__HAL_RCC_TIM4_CLK_ENABLE();
	debounceTimer.Instance = TIM4;
	debounceTimer.Init.Period = 50 - 1;
	debounceTimer.Init.Prescaler = 8000 - 1;
	debounceTimer.Init.ClockDivision = 0;
	debounceTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	debounceTimer.Init.RepetitionCounter = 0;
	debounceTimer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&debounceTimer);

	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_Base_Start_IT(&debounceTimer);

	// same as TIMx->CR1 |= value
	HAL_TIM_Base_Stop(&debounceTimer);

	// debounceTimer

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
}
