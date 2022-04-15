#ifndef _MAIN_LOOP_H_
#define _MAIN_LOOP_H_

#include "main.h"

//определяем какой формат данных собираем 
#define PARSE_NON_FLOAT_VALUE
//#define PARSE_FLOAT_VALUE


//структура для приема данных из орбитрона
#ifdef PARSE_NON_FLOAT_VALUE
typedef struct 
{
	int azimuth;
	int elevation;
	
} OrbitronData_t;

typedef struct 
{
	int azimuth;
	int elevation;
	
} RotatorData_t;
#endif

#ifdef PARSE_FLOAT_VALUE
typedef struct 
{
	double azimuth;
	double elevation;
	
} orbitronDate_t;

typedef struct 
{
	double azimuth;
	double elevation;
	
} RotatorData_t;
#endif

// для таблицы переходов
typedef void (*TRANSITION_FUNC_PTR_t)(void);

// состояния системы
typedef enum
{
	STATE_RECIEVE_DATA ,
	STATE_SHOW_ORBITRON_DATA,
	STATE_SHOW_ROTATOR_DATA,
	STATE_SEND_DATA_TO_ROTATOR,
	STATE_RECIEVE_DATA_FROM_ROTATOR,
	STATE_JOYSTICK_MODE,
	STATE_SET_0,
	STATE_BRAKING_MOTOR,
	STATE_MAX,
	
} State_t;

// события по которым отслеживаем переходы
typedef enum
{
	EVENT_NONE,
	EVENT_OK,
	EVENT_MAX,
	
} Event_t;


#define ORBITRON_BUFFER_SIZE 40     // размер буфера для приема данных с орбитрона


void loop(void);    // суперцикл программы

#endif 
