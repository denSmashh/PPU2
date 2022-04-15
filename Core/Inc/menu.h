#ifndef MENU_H_
#define MENU_H_

#include "lcd.h"


//все состояния меню
typedef enum 
{
	ERROR_MODE_MENU = 0,
	AUTO_MODE_MENU = 1,
	MANUAL_MODE_MENU = 2,
	STATE_MENU_MAX
	
} Menu_state_t;


typedef void (*TRANSITION_MENU_PTR_t) (void);

// структура для управления меню в программе
typedef struct 
{
	Menu_state_t state;
	_Bool error_state;
	_Bool updated;	
	
} Menu_t;


void update_recieve_data_from_orbitron(uint8_t* azimuth_orbitron, uint8_t* elevation_orbitron, uint8_t* sat_name);
void update_recieve_data_from_ppu(uint8_t* azimuth_ppu, uint8_t* elevation_ppu);
void update_data_from_joystick(uint8_t* azimuth_ppu, uint8_t* elevation_ppu);

void update_menu(Menu_state_t state);
void need_update_menu(void);
void init_menu(void);

void update_XY(int X, int Y);

#endif /* MENU_H_ */
