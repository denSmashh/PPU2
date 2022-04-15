#include "menu.h"

volatile Menu_t menu;
extern volatile State_t current_state;
extern _Bool rotator_move;

char AZ_string[7];
char EL_string[7];

// доп. символы
uint8_t symbol_pause[8] = {0,8,12,14,12,8,0};
uint8_t symbol_movement[8] = {0,27,27,27,27,27,27,0};

// функции для отрисовки меню
void error_connection_menu(void);
void auto_mode_menu(void);
void manual_mode_menu(void);

// таблица переходов для меню
TRANSITION_MENU_PTR_t menu_transition[STATE_MENU_MAX] = {
	
	error_connection_menu,
	auto_mode_menu,
	manual_mode_menu
	
};


void init_menu(void)
{
	menu.state = MANUAL_MODE_MENU;
	menu_transition[MANUAL_MODE_MENU]();
	
	menu.updated = 1;
}

void update_menu(Menu_state_t state)
{
	if(menu.updated == 0)
	{
		if(state == AUTO_MODE_MENU)
		{
			current_state = STATE_SHOW_ROTATOR_DATA;
		}
		else if(state == MANUAL_MODE_MENU)
		{
			current_state = STATE_JOYSTICK_MODE;
		}
		
		menu.state = state;
		menu_transition[state]();
	}
	
}

void need_update_menu()
{
	menu.updated = 0;
}


void error_connection_menu(void)
{
	LCD_Clear();
	LCD_SetCursor(0,0);
	LCD_String("Connect USB and");
	LCD_SetCursor(0,1);
	LCD_String("turn on Orbitron");	
	
	menu.updated = 1;
}

void auto_mode_menu(void)
{
	
	LCD_Clear();
	LCD_SetCursor(3,0);
	LCD_String("AUTO");
	LCD_SetCursor(8,1);
	LCD_String("MODE");
	HAL_Delay(1500);

	LCD_Clear();
	LCD_SetCursor(13,0);
	LCD_String("A|S");
	
	menu.updated = 1;
}

void manual_mode_menu(void)
{
	LCD_Clear();
	LCD_SetCursor(2,0);
	LCD_String("MANUAL");
	LCD_SetCursor(8,1);
	LCD_String("MODE");
	HAL_Delay(1500);
	
	LCD_Clear();
	LCD_SetCursor(0,0);
	LCD_String("Azim = ");
	LCD_SetCursor(0,1);
	LCD_String("Elev = ");
	
	LCD_SetCursor(13,0);
	LCD_String("M|S");
	
	
	menu.updated = 1;
}

void update_recieve_data_from_orbitron(uint8_t* azimuth_orbitron, uint8_t* elevation_orbitron, uint8_t* sat_name)
{
	LCD_SetCursor(0,0);
	LCD_String("           ");
	LCD_SetCursor(0,0);
	for(int i = 0; (sat_name[i] != '*' && i < 11); i++)
	{
		if((sat_name[i] == '_' && i > 6) || sat_name[i] == '(') break;
		if(sat_name[i] == '_') LCD_SendChar(' ');
		else LCD_SendChar((char)sat_name[i]);
	} 
	
	LCD_SetCursor(4,1);
	LCD_String("   ");	
	int pos = 6;
	LCD_SetCursor(pos,1);
	for(int i = 2; i >= 0; i--) 
	{
		if(azimuth_orbitron[i] == '*') continue;
		else
		{
			LCD_SendChar((char)azimuth_orbitron[i]);
			LCD_SetCursor(--pos,1);
		}	
	}
		
	LCD_SetCursor(13,1);
	LCD_String("   ");	
	LCD_SetCursor(15,1);
	pos = 15;
	for(int i = 2; i >= 0; i--) 
	{
		if(elevation_orbitron[i] == '*') continue;
		else
		{
			LCD_SendChar((char)elevation_orbitron[i]);
			LCD_SetCursor(--pos,1);
		}		
	}
	
}

void update_recieve_data_from_ppu(uint8_t* azimuth_ppu, uint8_t* elevation_ppu)
{
	LCD_SetCursor(0,1);
	LCD_String("   ");	
	LCD_SetCursor(0,1);
	for(int i = 0; (azimuth_ppu[i] != '*' && i < 3); i++) LCD_SendChar((char)azimuth_ppu[i]);
	LCD_SetCursor(3,1);
	LCD_SendChar((char)126);
	
	
	LCD_SetCursor(9,1);
	LCD_String("   ");	
	LCD_SetCursor(9,1);
	for(int i = 0; (elevation_ppu[i] != '*' && i < 3); i++) LCD_SendChar((char)elevation_ppu[i]);
	LCD_SetCursor(12,1);
	LCD_SendChar((char)126);
	
	LCD_SetCursor(15,0);
	if(rotator_move == 1) LCD_SendChar('D');
	else LCD_SendChar('S');
	
}


void update_data_from_joystick(uint8_t* azimuth_ppu, uint8_t* elevation_ppu)
{
	if(menu.state != AUTO_MODE_MENU)
	{
	LCD_SetCursor(7,0);
	LCD_String("   ");
	LCD_SetCursor(7,0);	
	for(int i = 0; (azimuth_ppu[i] != '*' && i < 3); i++) LCD_SendChar((char)azimuth_ppu[i]);

	LCD_SetCursor(7,1);
	LCD_String("   ");
	LCD_SetCursor(7,1);	
	for(int i = 0; (elevation_ppu[i] != '*' && i < 3); i++) LCD_SendChar((char)elevation_ppu[i]);
		
	LCD_SetCursor(15,0);
	if(rotator_move == 1) LCD_SendChar('D');
	else LCD_SendChar('S');	
	}
}


void update_XY(int X, int Y)			// debug
{
	sprintf(AZ_string, "%d   ", X);
	sprintf(EL_string, "%d   ", Y);
	
	LCD_SetCursor(4,0);
	LCD_String(AZ_string);
	
	LCD_SetCursor(4,1);
	LCD_String(EL_string);
	
	for(int i = 0; i < 7; i++)
	{
		AZ_string[i] = 0;
		EL_string[i] = 0;	
	}
}
