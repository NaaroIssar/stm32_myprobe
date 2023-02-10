/*
##############################################################################
##############################################################################
##
## Модуль меню
## Обеспечивает управление всей программой и запуск функциональных модулей
##
##############################################################################
##############################################################################
*/

#ifndef __MENU_H
#define __MENU_H

/* Includes ------------------------------------------------------------------*/

#include "icons.h"

/* External functions --------------------------------------------------------*/

extern void (*pLoop_Func)(void);
extern void (*esc_func)(void);

// Modules
extern void rect_signal_init(void);
extern void rect_signal2_init(void);
extern void sinus_signal_init(void);

/* Definitions ---------------------------------------------------------------*/

#define MENU_LOOP_CYCLE  250 // 50 ms

// структура пункта меню
typedef struct menu_item {
    void *next;         // указатель на следующий пункт
    void *prev;         // указатель на предыдущий пункт
    void *parent;       // указатель на родительский пункт
    void *child;        // указатель на дочерний пункт меню
    void (*func)(void); // указатель на функцию обработчик
    const char *text;         // указатель на текстовое название пункта меню для отображения
    const void *icon;         // указатель на графическую иконку
} menu_item_t; 

// пустой пункт меню
menu_item_t NULL_MENU = {0};//{NULL, NULL, NULL, NULL, NULL, "", NULL};

// макрос создания пункта меню
#define NEW_MENU_ITEM(name, func, text, icon) \
   menu_item_t name = {&NULL_MENU, &NULL_MENU, &NULL_MENU, &NULL_MENU, func, text, icon};

// макрос установления связей меню
#define MENU_ITEM_LINKS(Name, Next, Prev, Parent, Child) \
   Name.next = &Next; \
   Name.prev = &Prev; \
   Name.parent = &Parent; \
   Name.child = &Child;


// структура данных о состоянии кнопок
typedef struct
{
   uint16_t pressed;                           /* 0=button free, 1=button pressed, 2=button long pressed */
   uint32_t time_pressed;                      /* ticks when button pressed*/
   uint16_t repetition_cnt;                    /* buton long press repetition counter */
   void (* ButtonClickCallback)(void);         /* Button click callback */
} button_data;


// создание пунктов меню
NEW_MENU_ITEM(menu1,   NULL, "Measurements",  icon_measurements_32x32); 
NEW_MENU_ITEM(menu2,   NULL, "Monitoring",    icon_oscilloscope_32x32); 
NEW_MENU_ITEM(menu3,   NULL, "Generation",    icon_generation_32x32);
NEW_MENU_ITEM(menu4,   NULL, "Settings",      icon_settings_32x32); 
                                              
NEW_MENU_ITEM(menu1_1, NULL, "Log probe",     icon_logicprobe_32x32); 
NEW_MENU_ITEM(menu1_2, NULL, "Voltmeter",     icon_voltmeter_32x32); 
NEW_MENU_ITEM(menu1_3, NULL, "Freqmeter",     icon_impulse_measurement_32x32); 
NEW_MENU_ITEM(menu1_4, NULL, "Imp meter",     icon_impulse_measurement_32x32); 
NEW_MENU_ITEM(menu1_5, NULL, "UART speed",    icon_uart_measurement_32x32); 
NEW_MENU_ITEM(menu1_6, NULL, "Counter",       NULL); 
NEW_MENU_ITEM(menu1_7, NULL, "Diode volt",    NULL); 
NEW_MENU_ITEM(menu1_8, NULL, "Capacitance",   NULL); 

NEW_MENU_ITEM(menu2_1, NULL,  "Oscilloscope", icon_oscilloscope_32x32); 
NEW_MENU_ITEM(menu2_2, NULL,  "UART",         NULL); 
NEW_MENU_ITEM(menu2_3, NULL,  "Spectral",     icon_spectral_32x32); 

NEW_MENU_ITEM(menu3_1, rect_signal_init,  "Rectang signal", icon_rectangular_32x32); 
NEW_MENU_ITEM(menu3_2, rect_signal2_init, "Rectg signal 2", icon_rectangular_32x32); 
NEW_MENU_ITEM(menu3_3, sinus_signal_init, "Sinus signal",   NULL); 
NEW_MENU_ITEM(menu3_4, NULL, "PWM signal", icon_pwm_32x32); 
NEW_MENU_ITEM(menu3_5, NULL, "UART", NULL); 
NEW_MENU_ITEM(menu3_6, NULL, "R/C servo", icon_rcservo_32x32); 
NEW_MENU_ITEM(menu3_7, NULL, "White noise", NULL); 
NEW_MENU_ITEM(menu3_8, NULL, "IR LED", NULL); 
NEW_MENU_ITEM(menu3_9, NULL, "One pulse", icon_impulse_32x32); 

NEW_MENU_ITEM(menu4_1, NULL, "Display set",   icon_size_32x32); 
NEW_MENU_ITEM(menu4_2, NULL, "Freq calib",    icon_fx_32x32); 
NEW_MENU_ITEM(menu4_3, NULL, "Voltage calib", icon_voltmeter2_32x32); 

/* Functions -----------------------------------------------------------------*/

void menu_init(void);
void key_press_handler(uint8_t key);
void SysTick_Handler(void);

/* External variables --------------------------------------------------------*/

extern uint16_t main_loop_time;
extern UART_HandleTypeDef huart1;
 
#endif /* __MENU_H */
/*----------------------------------------------------------------------------*/
