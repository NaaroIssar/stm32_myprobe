/*
##############################################################################
##############################################################################
##
## Генерация синусоидального сигнала 1 Гц - 20 кГц
##
##############################################################################
##############################################################################
*/

#ifndef __SINUS_SIGNAL_H
#define __SINUS_SIGNAL_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"
#include "stm32f3xx_ll_tim.h"
#include "stm32f3xx_hal_dac.h"

#ifdef __cplusplus
}
#endif

/* Definitions ---------------------------------------------------------------*/

#define SINUS_SYSCLK_MAX_FREQ                      72010000 // максимальная системная частота для генерации синуса
                                                            // другими словами - допустимый разгон контроллера

#define SIN_MIN_FREQ   10    // минимальная частота синусоиды
#define SIN_MAX_FREQ   20000 // максимальная частота 
#define K              60    // коэффициент деления системной частоты для задания опорной частоты
#define DMA_BUF_LENGTH 64

// Преобразование из обычного формата представления чисел
// (с плавающей точкой или целого) в формат с фиксированной
// точкой: 16 бит дробная и 16 бит целая часть.
#define float_to_fix16(x) (int32_t)((x)*0x10000)

// Преобразование числа с фиксированной точкой (16 бит в дробной части)
// в число с плавающей точкой.
#define fix16_to_float(x) ((x)/65536.0)

// Определяем тип с фиксированной точкой (16 бит в дробной части).
// Эквивалентен целому со знаком и используется только в целях
// обеспечения выразительности кода.
typedef int32_t fix16;


/* Functions -----------------------------------------------------------------*/

void sinus_signal_init(void);

/* External functions --------------------------------------------------------*/

extern void DrawMainScreen(char *title);
extern void (*esc_func)(void);
extern uint8_t ChangePLL(uint8_t PreDiv, uint8_t PLLMUL);
extern void rect_signal_draw_screen(void);
extern void rect_signal_key1_handler(void);
extern void rect_signal_key2_handler(void);
extern void rect_signal_key4_handler(void);
//extern HAL_StatusTypeDef HAL_DAC_Init(DAC_HandleTypeDef* hdac);
//extern HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef* hdac, DAC_ChannelConfTypeDef* sConfig, uint32_t Channel);


/* External variables --------------------------------------------------------*/

extern t_keys_handlers keys_handlers;
extern const uint32_t gen_freq_step_arr [];   // шаг изменения частоты
extern const char *gen_freq_step_name_arr []; // названия шага изменения частоты
extern uint8_t gen_freq_step_idx;             // переменная хранения шага изменения частоты
extern uint8_t change_num;                    // что изменяется кнопками изменения: 0 - частота, 1 - шаг изменения частоты, 2 - другое
extern UART_HandleTypeDef huart1;
extern char tmp_str[30];
extern uint32_t  rect_freq;       // заданная частота, Гц
extern uint32_t  rect_freq_real;  // действительно генерируемая частота, Гц
extern uint16_t  rect_freq_step;  // Шаг изменения частоты
extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern TIM_HandleTypeDef htim6;
extern void (* dma_buf_first_half_init_pointer)();
extern void (* dma_buf_second_half_init_pointer)();

#endif /* __SINUS_SIGNAL_H */ 
/*----------------------------------------------------------------------------*/
