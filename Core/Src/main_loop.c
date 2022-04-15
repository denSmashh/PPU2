#include "main_loop.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef hlpuart1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;
extern IWDG_HandleTypeDef hiwdg;

//функции состояний
void error(void);
void recieve_data_from_orbitron(void);
void show_orbitron_data(void);
void show_rotator_data(void);
void send_to_rotator(void);
void manual_mode(void); 
void recieve_data_from_rotator(void);
void set_rotator_to_zero(void);
void emergency_brake(void);

// вспомогательные функции
void ParseToDouble(void); 
void copy_data(void);
void read_message(uint8_t* buffer);
void create_message(uint8_t * tx_buffer, uint8_t * az_bf, uint8_t * el_bf);
void create_message_from_joystick(uint8_t * tx_buffer);


//для приёма из орбитрона
volatile uint8_t RxBf[1] = {0};                                // буфер для посимвольного приема из орбитрона
volatile uint8_t orbitron_buffer[ORBITRON_BUFFER_SIZE] = {0};  // массив с принятыми данными из орбитрона
volatile int orbitron_buffer_position;                         // индекс для движения по массиву

// для приема данных с rs485 
uint8_t rx_rs485_buffer[11] = {0};         


//для мигания светодиода в ручном режиме
const uint32_t rotator_led_timeout = 500;
uint32_t rotator_led_timer;

volatile int btn_joystick_pressed = 0;
volatile int set0_pressed = 0;

_Bool rotator_move = 0;				               // сообщает движется ПУ или нет

int axis_X = 0;                              // для опроса АЦП
int axis_Y = 0;

uint8_t satellite_name_buffer[11] = {0};     // сохраняется название спутника
uint8_t AZ_buffer[3] = {0};                  // для данных об азимуте из орбитрона
uint8_t EL_buffer[3] = {0};                  // для данных об элевации из орбитрона
uint8_t AZ_rotator_buffer[3] = {0};          // для данных об азимуте ПУ 
uint8_t EL_rotator_buffer[3] = {0};          // для данных об элевации ПУ

OrbitronData_t Coords;                       // структура для данных из орбитрона
RotatorData_t RotatorCoords;                 // структура данных для положения поворотного устройства (ПУ)

//флаги для прерываний
volatile _Bool usart1_recieve_complete = 0;
volatile _Bool rs485_recieve_complete = 0;


volatile Menu_state_t current_menu_state = MANUAL_MODE_MENU;   // текущее состояние меню (при запуске ручной режим)
volatile State_t current_state = STATE_JOYSTICK_MODE;          // при запуске устройства мы автоматически в ручном режиме
volatile Event_t current_event = EVENT_NONE;                     
volatile State_t prev_state = STATE_JOYSTICK_MODE;             // необходимо для кнопок, чтобы вернуться в функцию где прервали выполнение программы

//программа построена на принципе машины состояний
//таблица переходов между состояниями
TRANSITION_FUNC_PTR_t transition_table[STATE_MAX][EVENT_MAX] = {

	[STATE_RECIEVE_DATA][EVENT_NONE] = recieve_data_from_orbitron,
	[STATE_RECIEVE_DATA][EVENT_OK] = show_orbitron_data,
	
	[STATE_SHOW_ORBITRON_DATA][EVENT_NONE] = error,
	[STATE_SHOW_ORBITRON_DATA][EVENT_OK] = send_to_rotator, 

	[STATE_SEND_DATA_TO_ROTATOR][EVENT_NONE] = error,
	[STATE_SEND_DATA_TO_ROTATOR][EVENT_OK] = recieve_data_from_rotator,

	[STATE_RECIEVE_DATA_FROM_ROTATOR][EVENT_NONE] = recieve_data_from_rotator,
	[STATE_RECIEVE_DATA_FROM_ROTATOR][EVENT_OK] = show_rotator_data,  
	
	[STATE_SHOW_ROTATOR_DATA][EVENT_NONE] = show_rotator_data,
	[STATE_SHOW_ROTATOR_DATA][EVENT_OK] = recieve_data_from_orbitron, 
	
	[STATE_JOYSTICK_MODE][EVENT_NONE] = manual_mode,
	
	[STATE_SET_0][EVENT_NONE] = set_rotator_to_zero,
	
	[STATE_BRAKING_MOTOR][EVENT_NONE] = emergency_brake,
		
};


void loop()
{	
	while(1) 
	{	
		transition_table[current_state][current_event]();		
	}
}

void error (void) 
{
	__disable_irq();
	while(1){};       //если мы тут, то произошла ошибка
}


void manual_mode(void) 
{
	current_state = STATE_JOYSTICK_MODE;
	current_event = EVENT_NONE;
	
	update_menu(current_menu_state);                   // обновляем меню
	HAL_IWDG_Refresh(&hiwdg);				                   // обновляем сторожевой таймер
	
	HAL_ADC_Start(&hadc1);                             // опрос АЦП
	HAL_ADC_PollForConversion(&hadc1,100);
	axis_Y = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_PollForConversion(&hadc1,100);
	axis_X = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	
	update_data_from_joystick(AZ_rotator_buffer,EL_rotator_buffer);          // обновляем значения на дисплее
	
	//отправка данных джойстика
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_SET);    // устанавливаем rs485 на передачу

	uint8_t tx_rs485_joystick_buffer[10] = {0};
	create_message_from_joystick(tx_rs485_joystick_buffer);                  // создаем сообщение для отправки

	if(huart1.gState == HAL_UART_STATE_READY)                     
	{
		HAL_UART_Transmit(&hlpuart1,tx_rs485_joystick_buffer,10,0xFFFF);     
	}
	
	// прием данных с ПУ
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_RESET); // устанавливаем rs485 на прием
	while(1)                                 															   // ждем приема данных из ПУ
	{
		HAL_UART_Receive_IT(&hlpuart1,rx_rs485_buffer,11);
		if(rs485_recieve_complete)             
		{
			read_message(rx_rs485_buffer);                                       // читаем принятые данные    
			
			HAL_UART_AbortReceive_IT(&huart1);
			rs485_recieve_complete = 0;	
			
			if(HAL_GetTick() - rotator_led_timer > rotator_led_timeout)           // прием произошел => мигаем светодиодом
			{
				HAL_GPIO_TogglePin(LEDS_Port,LED_ROTATOR_Pin);
				rotator_led_timer = HAL_GetTick();
			}
			break;
		}
	}

}

/* формат принимаемых данных $[AZ] [EL] [SN]!   AZ - азимут, EL - элевация, SN - название спутника
                                                $ - знак начала, ! - знак конца      */
void recieve_data_from_orbitron(void) 
{	                                                                                               
	current_state = STATE_RECIEVE_DATA;
	current_event = EVENT_NONE;
			
	update_menu(current_menu_state);                               // обновляем меню
	HAL_IWDG_Refresh(&hiwdg);                                      // обновляем watchdog
	
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);                    // включаем прерывание на приём
	
	if(usart1_recieve_complete) 
	{		
		copy_data();		                                             // копируем нужные данные в отдельные буферы 
		
		for(int i = 0; i < ORBITRON_BUFFER_SIZE; i++) orbitron_buffer[i] = 0;			// очищаем буфер приема		  
		orbitron_buffer_position = 0;
			
		usart1_recieve_complete = 0;
			
		__HAL_UART_DISABLE_IT(&huart1,UART_IT_RXNE);
		
		HAL_GPIO_WritePin(LEDS_Port,LED_PC_Pin,GPIO_PIN_SET);       // включаем светодиод
		
		current_event = EVENT_OK;                                   // сигнал о том что функция выполнена						
	}	
}


void send_to_rotator(void) 
{
	current_state = STATE_SEND_DATA_TO_ROTATOR;
	current_event = EVENT_NONE;
	
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_SET);      // устанавливаем rs485 на передачу
	
	uint8_t tx_rs485_buffer[10] = {0};
	create_message(tx_rs485_buffer,AZ_buffer,EL_buffer);                        // готовим сообщение на отправку, размером 10 байт
	
	HAL_UART_Transmit(&hlpuart1,tx_rs485_buffer,10,0xFFFF);                     // отправляем данные
	current_event = EVENT_OK;
}

void recieve_data_from_rotator(void)
{
	current_state = STATE_RECIEVE_DATA_FROM_ROTATOR;
	current_event = EVENT_NONE;
	
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_RESET);    // устанавливаем rs485 на прием 
	HAL_UART_Receive_IT(&hlpuart1,rx_rs485_buffer,11);                          // принимаем... 
	
	if(rs485_recieve_complete)
	{
		read_message(rx_rs485_buffer);
		
		HAL_UART_AbortReceive_IT(&huart1);		
		
		HAL_GPIO_WritePin(LEDS_Port,LED_ROTATOR_Pin,GPIO_PIN_SET);                 // мигаем светодиодом

		rs485_recieve_complete = 0;			
		current_event = EVENT_OK;
	}
}


void show_orbitron_data(void)                                                 // обновляем данные с орбитрона на дисплее
{
	current_state = STATE_SHOW_ORBITRON_DATA;
	current_event = EVENT_NONE;
	
	update_recieve_data_from_orbitron(AZ_buffer,EL_buffer,satellite_name_buffer);
	HAL_GPIO_WritePin(LEDS_Port,LED_PC_Pin,GPIO_PIN_RESET);
	
	current_event = EVENT_OK; 
	}		

void show_rotator_data(void)                                                // обновляем данные с ПУ на дисплее
{
	current_state = STATE_SHOW_ROTATOR_DATA;
	current_event = EVENT_NONE;
	
	update_recieve_data_from_ppu(AZ_rotator_buffer,EL_rotator_buffer);
	HAL_GPIO_WritePin(LEDS_Port,LED_ROTATOR_Pin,GPIO_PIN_RESET);
	
	current_event = EVENT_OK;
}

void set_rotator_to_zero(void)                   // устанавливаем значения ПУ в ноль и отправляем сигнал на ПУ по rs485
{
	current_state = STATE_SET_0;
	current_event = EVENT_NONE;
	
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_SET);   // устанавливаем rs485 на передачу
	uint8_t tx_rs485_set_zero_buffer[10] = {0};
	
	tx_rs485_set_zero_buffer[0] = '$';
	tx_rs485_set_zero_buffer[1] = 'z';             // знак того, что удерживаем кнопку "НУЛЬ"
	tx_rs485_set_zero_buffer[2] = '!';
	
	if(huart1.gState == HAL_UART_STATE_READY) 
	{
		HAL_UART_Transmit(&hlpuart1,tx_rs485_set_zero_buffer,10,0xFFFF);

		buzzer(400);                                 // звуковой сигнал
		set0_pressed = 0; 
		
		if(prev_state == STATE_JOYSTICK_MODE) current_state = STATE_JOYSTICK_MODE;
		else current_state = STATE_RECIEVE_DATA;		
	}
}

void emergency_brake(void)                  // отправляем сигнал для торможения моторов на ПУ 
{
	current_state = STATE_BRAKING_MOTOR;
	current_event = EVENT_NONE;
	
	HAL_GPIO_WritePin(LPUART_RS485_Port,LPUART_RS485_DE_Pin,GPIO_PIN_SET);
	uint8_t tx_rs485_braking_buffer[10] = {0};
	
	tx_rs485_braking_buffer[0] = '$';
	tx_rs485_braking_buffer[1] = 'b';         // знак того, что нажали кнопку "НУЛЬ"
	if(prev_state != STATE_JOYSTICK_MODE)     // знак того, в каком режиме останавливаем работу моторов
	{
		tx_rs485_braking_buffer[2] = 'a';
		tx_rs485_braking_buffer[3] = '!';
	}
	else
	{
		tx_rs485_braking_buffer[2] = 'm';
		tx_rs485_braking_buffer[3] = '!';
	}
	
	
	if(huart1.gState == HAL_UART_STATE_READY)  
	{
		HAL_UART_Transmit(&hlpuart1,tx_rs485_braking_buffer,10,0xFFFF);
		
		buzzer(150);                           // звуковой сигнал
		
		if(prev_state == STATE_JOYSTICK_MODE) current_state = STATE_JOYSTICK_MODE;
		else current_state = STATE_RECIEVE_DATA;
		
		set0_pressed = 0;
		rotator_move = 0;
		HAL_Delay(300); 
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)      // обработчик прерывания для приема данных
{	
	if(huart == &huart1)
	{
		switch(RxBf[0])
		{
			case '!': orbitron_buffer[orbitron_buffer_position] = '!'; usart1_recieve_complete = 1;
								break;
			default : 
				orbitron_buffer[orbitron_buffer_position] = RxBf[0];
				orbitron_buffer_position++;		
		}
	}	
	
	else if(huart == &hlpuart1)
	{
		rs485_recieve_complete = 1;
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)      // обработчик прерывания кнопок
{
		 HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
		 HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
		 HAL_TIM_Base_Start_IT(&htim6);           // таймер для кнопки "НОЛЬ"
	
	if(GPIO_Pin == MODE_BUTTON_Pin)        // нажатие на кнопку смены режима
		{		
			if (current_state == STATE_JOYSTICK_MODE) 
			{
					need_update_menu();
					current_menu_state = AUTO_MODE_MENU;
					HAL_GPIO_WritePin(LEDS_Port,LED_ROTATOR_Pin,GPIO_PIN_SET);
			}		
			else
			{
					need_update_menu();
					current_menu_state = MANUAL_MODE_MENU;		  	
			}
			current_event = EVENT_NONE;
		}
		
		else if(GPIO_Pin == SET0_BUTTON_Pin)     // нажатие на кнопку "НОЛЬ"
		{
			set0_pressed++;
		}
	
		else if(GPIO_Pin == JOYSTICK_BUTTON_Pin)    // нажатие на джойстик   
		{
		/*	btn_joystick_pressed++;
			if(btn_joystick_pressed == 1)
			{
				
			}	
			*/		
		}		
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)     // обработчик прерывания таймера, который решает нажали кнопку или удерживаем
{
	 HAL_TIM_Base_Stop_IT(&htim6);             // останавливаем таймер
		 
   __HAL_GPIO_EXTI_CLEAR_IT(EXTI4_15_IRQn);  // очищаем бит EXTI_PR (бит прерывания)
   NVIC_ClearPendingIRQ(EXTI4_15_IRQn);      // очищаем бит NVIC_ICPRx (бит очереди)
   HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);        // включаем внешнее прерывание
		 
	 __HAL_GPIO_EXTI_CLEAR_IT(EXTI2_3_IRQn);  
   NVIC_ClearPendingIRQ(EXTI2_3_IRQn); 
   HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);  
  
	if(set0_pressed)
	{
		if(HAL_GPIO_ReadPin(SET0_BUTTON_Port,SET0_BUTTON_Pin) == GPIO_PIN_SET)  
		{
	   prev_state = current_state;                                  
			current_state = STATE_BRAKING_MOTOR;                   // удержание кнопки
		 current_event = EVENT_NONE;
		}
	  else 
	  {
		 prev_state = current_state;
		 current_state = STATE_SET_0;
		 current_event = EVENT_NONE;
	  }
	}
}


void create_message(uint8_t * tx_buffer, uint8_t * az_bf, uint8_t * el_bf)
{
	tx_buffer[0] = '$';
	tx_buffer[1] = 'a'; // знак того, что работаем в режиме auto 
	
	int bf_position = 2;
	for(int i = 0; ( i < 3 && az_bf[i] != '*'); i++, bf_position++)
	{
		tx_buffer[bf_position] = az_bf[i];
	}
	
	tx_buffer[bf_position] = ' ';
	bf_position++;
	
	for(int i = 0; (i < 3 && el_bf[i] != '*'); i++, bf_position++)
	{
		tx_buffer[bf_position] = el_bf[i];
	}
	tx_buffer[bf_position] = '!';	
}

void create_message_from_joystick(uint8_t * tx_buffer)
{
	tx_buffer[0] = '$';
	tx_buffer[1] = 'm';  // знак того, что режим auto 
	
	char X_string[4] = {0};
	char Y_string[4] = {0};
	
	sprintf(X_string,"%d",axis_X);
	sprintf(Y_string,"%d",axis_Y);
	
	int bf_position = 2;
	for(int i = 0; ( i < 3 && X_string[i] != '\0'); i++, bf_position++)
	{
		tx_buffer[bf_position] = X_string[i];
	}
	
	tx_buffer[bf_position] = ' ';
	bf_position++;
	
	for(int i = 0; (i < 3 && Y_string[i] != '\0'); i++, bf_position++)
	{
		tx_buffer[bf_position] = Y_string[i];
	}
	tx_buffer[bf_position] = '!';	
}

void read_message(uint8_t* buffer)
{
	if(buffer[0] == '$')
	{	
		for(int i = 0; i < 3; i++)     //обнуляем буффер перед записью
		{
			AZ_rotator_buffer[i] = '*';  // символ '*' показывает, что ячейка массива пустая
			EL_rotator_buffer[i] = '*';
		}
		
		int buffer_position = 1;
		for(int i = 0; buffer[buffer_position] != ' ' ; buffer_position++, i++) AZ_rotator_buffer[i] = buffer[buffer_position];
		buffer_position++;
		
		for(int i = 0; buffer[buffer_position] != ' ' ; buffer_position++, i++) EL_rotator_buffer[i] = buffer[buffer_position];
		buffer_position++;
		
		if(buffer[buffer_position] == 'D') rotator_move = 1;
		else if(buffer[buffer_position] == 'S') rotator_move = 0;
		
	}
	
	//else{} // обработать ошибку в сообщении 
}


void copy_data(void)
{
	int buffer_index = 0;      // индексы для движения по буферам
	int az_buffer_index = 0;
	int el_buffer_index = 0;
	int sat_buffer_index = 0;
	
	for(int i = 0; i < 3; i++)  //очищаем буферы для az и el перед копированием
	{
		AZ_buffer[i] = '*';      // символ '*' показывает что ячейка массива пустая
		EL_buffer[i] = '*';
	} 
	
  for(int i = 0; i < 11; i++)  //очищаем буфер для имени спутника перед копированием
	{
		satellite_name_buffer[i] = '*';
	}	
	
		if (orbitron_buffer[0] == '$')
		{
			for (buffer_index = 1; orbitron_buffer[buffer_index] != '.'; buffer_index++)		// копируем данные азимута
			{
				AZ_buffer[az_buffer_index] = orbitron_buffer[buffer_index];
				az_buffer_index++;
			}
			
			for(;orbitron_buffer[buffer_index] != ' '; buffer_index++);
			buffer_index++;
			
			for (;orbitron_buffer[buffer_index] != '.'; buffer_index++) 				// копируем данные элевации
			{
				EL_buffer[el_buffer_index] = orbitron_buffer[buffer_index];
				el_buffer_index++;
			}
			
			for(;orbitron_buffer[buffer_index] != ' '; buffer_index++);
			buffer_index++;	
			
			for (;orbitron_buffer[buffer_index] != '!' && sat_buffer_index < 11; buffer_index++) // копируем имя спутника			 
			{
				satellite_name_buffer[sat_buffer_index] = orbitron_buffer[buffer_index];
				sat_buffer_index++;
			}			
		}		
		else{}  // обработать ошибку приёма!
}

#ifdef PARSE_NON_FLOAT_VALUE
void ParseToDouble()  				// формат принимаемых данных $[AZ] [EL] [SN]!   
{
	int buffer_position = 0;    // индекс элемента для движения по буфферу
	
	Coords.azimuth = 0;
	Coords.elevation = 0;
	
	if (orbitron_buffer[0] == '$')
	{
		for (buffer_position = 1; orbitron_buffer[buffer_position] != '.'; buffer_position++);
		buffer_position--;
		
		//парсим и записываем значение в поле azimuth 
		for (int k = 1; orbitron_buffer[buffer_position] != '$'; buffer_position--, k *= 10) 
		{
			Coords.azimuth += (orbitron_buffer[buffer_position] - '0') * k;
		}

		for ( ;orbitron_buffer[buffer_position] != '!'; buffer_position++);
		buffer_position-=3;

		//парсим и записываем значение в поле elevation 
		for (int k = 1; orbitron_buffer[buffer_position] != ' ' ; buffer_position--, k *= 10) //считаем единицы, десятки, сотни значения
		{
			if (orbitron_buffer[buffer_position] == '-')
			{
				Coords.elevation *= -1;
				break;
			}
			Coords.elevation += (orbitron_buffer[buffer_position] - '0') * k;
		}
	}
}
#endif

#ifdef PARSE_FLOAT_VALUE
void ParseToDouble()  // формат принимаемых данных $[AZ] [EL]!
{
	int buffer_position = 0; // индекс элемента для движения по буфферу
	
	Coords.azimuth = 0;
	Coords.elevation = 0;
	
	if (orbitron_buffer[0] == '$')
	{
		for (buffer_position = 1; orbitron_buffer[buffer_position] != ' '; buffer_position++);
		buffer_position--;

		//парсим и записываем в поле azimuth значение
		Coords.azimuth += (double)(orbitron_buffer[buffer_position] - '0') / 10; //считаем десятичную часть AZ
		
		buffer_position -= 2;
		for (int k = 1; orbitron_buffer[buffer_position] != '$'; buffer_position--, k *= 10) //считаем единицы, десятки, сотни значения
		{
			Coords.azimuth += (orbitron_buffer[buffer_position] - '0') * k;
		}

		for ( ;orbitron_buffer[buffer_position] != '!'; buffer_position++);
		buffer_position--;

		//парсим и записываем в поле elevation значение
		Coords.elevation += (double)(orbitron_buffer[buffer_position] - '0') / 10; //считаем десятичную часть EL
		buffer_position -= 2;
		for (int k = 1; orbitron_buffer[buffer_position] != ' ' ; buffer_position--, k *= 10) //считаем единицы, десятки, сотни значения
		{
			if (orbitron_buffer[buffer_position] == '-')
			{
				Coords.elevation *= -1;
				break;
			}
			Coords.elevation += (orbitron_buffer[buffer_position] - '0') * k;
		}
	}
}
#endif
