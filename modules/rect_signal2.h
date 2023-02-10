/*
##############################################################################
##############################################################################
##
## Модуль генерации прямоугольных импульсов
## Модуль обеспечивает генерацию импульсов в диапазоне частот от 1 Гц до 1 МГц,
## используя изменение системной частоты, делители таймера, ШИМ и чередование 
## генерируемых частот.
## Точность генерируемой частоты - не хуже +-0.2 Гц
## Побочными эффектами являются наличие джиттера и девиации частоты.
##
##############################################################################
##############################################################################
*/

#ifndef __RECT_SIGNAL2_H
#define __RECT_SIGNAL2_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"
#include "stm32f3xx_ll_tim.h"

#ifdef __cplusplus
}
#endif

/* Definitions ---------------------------------------------------------------*/

#define RECT_SIGNAL2_MIN_FREQ                 1        // минимальная частота прямоугольного сигнала
#define RECT_SIGNAL2_MAX_FREQ                 1000000  // максимальная частота 1 МГц
#define RECT_SIGNAL2_TWO_FREQ_GENERATION      1        // разрешена ли двухчастотная генерация сигнала
#define RECT_SIGNAL2_MAX_ALLOWED_DIFFERENCE   0.05     // максимально допустимое отклонение от заданной частоты 
                                                       // для одночастотной генерации сигнала
#define RECT_SIGNAL2_BUFFER_SIZE              4000     // размер буфера DMA для двухчастотной генерации
#define IR_OUT_MIN_FREQ                       4000000  // минимальная частота для IROUT
#define IR_OUT_MAX_FREQ                       36007000 // максимальная частота для IROUT

#define RECT_SIGNAL2_ONE_FREQ                 0
#define RECT_SIGNAL2_TWO_FREQ                 1

/* Functions -----------------------------------------------------------------*/

void rect_signal2_init(void);

/* External functions --------------------------------------------------------*/

extern double fabs(double X);
extern void init_freqs_array(void);
extern void DrawMainScreen(char *title);
extern void (*esc_func)(void);
extern uint8_t ChangePLL(uint8_t PreDiv, uint8_t PLLMUL);
extern void rect_signal_draw_screen(void);

/* External variables --------------------------------------------------------*/

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern DMA_HandleTypeDef hdma_tim16_ch1_up;
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


#endif /* __RECT_SIGNAL2_H */ 
/* ---------------------------------------------------------------------------*/
