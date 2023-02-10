/*
##############################################################################
##############################################################################
##
## Главная функция проекта
##
##############################################################################
##############################################################################
*/

#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

#ifdef __cplusplus
}
#endif

/* Definitions ---------------------------------------------------------------*/

#define VERSION                  1  // версия системы
#define gen_freq_step_size      11  // размер массива шага приращения частот
#define WORK_FREQS_LEN          72  // размер массива системных частот
#define MAIN_LOOP_TIME_DEFAULT 250  // loop цикл - 4 раза/сек



struct ControlBits
{
   unsigned rect_signal_freq_stored: 1;
   unsigned rect_signal2_freq_stored: 1;
   unsigned sinus_signal_freq_stored: 1;
   unsigned imp_width_stored: 1;
   unsigned freqmeter_mode: 1;
   unsigned logprobe_mode: 1;
   unsigned oscilloscope_timing: 1;
   unsigned oscilloscope_refresh: 1;
   unsigned pwm_pwm_stored: 1;
   unsigned rc_pwn_stored: 1;
   unsigned spectral_stored: 1;
   unsigned r8: 1;
   unsigned r9: 1;
   unsigned r10: 1;
   unsigned r11: 1;
   unsigned r12: 1;
};

// структура для хранения рабочих данных программы в памяти FLASH
typedef struct _control_data
{
   uint16_t  marker;   // маркер начала блока 0x55, 0xAA;
   uint8_t   version;  // версия системы
   uint32_t  Fos;      // действительная частота осциллятора
   //uint32_t  Control;  // управляющие биты - что запоминать во FLASH
   struct ControlBits Control;
   uint16_t  Menu;     // последний пункт меню
   uint32_t  Frect;    // последняя частота генератора прямоугольных импульсов
   uint32_t  Frect2;   // последняя частота генератора прямоугольных импульсов 2
   uint16_t  Fsin;     // последняя частота генератора синусоиды
   uint32_t  Timp;     // последняя длительность генератора одиночных импульсов
} control_data_t;

// структура массива системных частот
typedef struct sysclk_gen
{
   uint16_t prediv;
   uint16_t pllmul;
   double   sysclk;
} sysclk_gen_t;

// структура данных управления меню и модулями
typedef struct {
    void (* pFunc_key1)(); // указатель на обработчик кнопки 1
    void (* pFunc_key2)(); // указатель на обработчик кнопки 2
    void (* pFunc_key3)(); // указатель на обработчик кнопки 3
    void (* pFunc_key4)(); // указатель на обработчик кнопки 4
} t_keys_handlers;

/* Functions -----------------------------------------------------------------*/

void (*pLoop_Func)(void); // указатель на обработчик бесконечного цикла
void (*esc_func)(void);   // указатель на функцию выхода из модуля

/* External functions --------------------------------------------------------*/

extern void menu_init(void);
extern UART_HandleTypeDef huart1;

/* External variables --------------------------------------------------------*/


#endif /* __MAIN_H */
/*----------------------------------------------------------------------------*/
