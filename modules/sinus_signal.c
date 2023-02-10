/*
##############################################################################
##############################################################################
##
## Генерация синусоидального сигнала 1 Гц - 20 кГц
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "util.h"
#include "config.h"
#include "hardware.h"
#include "screen.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "sinus_signal.h"

/* Variables -----------------------------------------------------------------*/

// Максимальный выходной сигнал, постоянное смещение и амплитуда синтезируемой синусоиды
const uint16_t out_max = 0xFFFF;
uint16_t out_a0 = 0xFFFF / 2;
uint16_t out_a1 = 0xFFFF / 3;

// Тактовая частота сигнала на входе таймера
uint32_t FSYSCLK;

// Опорная частота 
// Возможны только значения вида FSYSCLK/K, где
// K-целое из диапазона 2..65536.
uint32_t F0;

// Текущее значение индекса фазы.
uint32_t ph_ind = 0;

// Приращение индекса фазы в каждом цикле синтеза;
// частота синтезируемого сигнала f = F0 * ph_m / (2.0**32),
// где F0 - опорная частота.
uint32_t ph_m = 0;

// Буфер значений, загружаемых в DAC с использованием DMA.
const uint16_t dma_buf_length = DMA_BUF_LENGTH;
int16_t dma_buf[DMA_BUF_LENGTH];



/* Functions -----------------------------------------------------------------*/

// Функция быстрого вычисления sin, |x|<=M_PI/4.
fix16 _fast_sin(fix16 x)
{
    // Вычисляется x*x: предкоррекция, целочисленное умножение, посткоррекция.
    fix16 x2=(x>>1)*x>>15; // x2=x**2.

    fix16 xn=x2*x>>16;     // xn=x**3.

    fix16 r=x-(xn*float_to_fix16(1/6.0)>>16);

    xn*=x2; xn>>=16;       // xn=x**5.

    // r=x-(x**3)/6.0+(x**5)/120.0.
    r+=(xn*float_to_fix16(1/120.0)>>16);

    return r;
}

// Функция быстрого вычисления cos, |x|<=M_PI/4.
fix16 _fast_cos(fix16 x)
{
    // Вычисляется x*x: предкоррекция, целочисленное умножение, посткоррекция.
    fix16 x2=x*(x>>1)>>15; // x2=x**2.

    fix16 xn=x2*x2>>16;    // xn=x**4.

    fix16 r=float_to_fix16(1.0)-
        (x2*float_to_fix16(0.5)>>16)+
        (xn*float_to_fix16(1/24.0)>>16);

    xn *= x2; xn >>= 16;       // xn=x**6.

    // r=1-(x**2)/2.0+(x**4)/24.0-(x**6)/720.0.
    r -= xn * float_to_fix16(1.0/720) >> 16;
    
    return r;
}

// Функция вычисления sin от аргумента, заданного индексом фазы.
fix16 fast_sin(uint32_t k)
{
    fix16 (*ftable[])(fix16) = {_fast_sin, _fast_cos};

    // Индексу фазы k соответствует фаза (аргумент) phi=2*M_PI*k/(2**32).
    // phi можно представить в виде:
    // phi=n*M_PI/2+alpha, где n - некоторое целое, а |alpha|<=M_PI/4.

    // Вычисляем n.
    uint32_t n = (k + 0x20000000) >> 30;

    // Вычисляем соответствующий alpha индекс фазы (целое со знаком).
    int32_t ka = k - (n << 30);  // fix32, 30bits

    // Преобразуем индекс фазы в фазу в формате fix16.
    fix16 alpha =                   // fix30, 31bits
        (int)(4096 * 2 * M_PI) *    // 2*M_PI, fix12, 15bits
        (ka >> 14);                 // fix18, 16bits
    alpha >>= 14;                   // fix16

    // r = _fast_sin(alpha) или r = _fast_cos(alpha), в зависимости
    // от младшего бита числа n:
    fix16 r = ftable[n&1](alpha);

    // Если бит [1] в n установлен, в качестве результата берём
    // r с противоположным знаком: if (n & 2) r = -r;
    int32_t s = (n>>1) & 1;
    return (r^(-s)) + s;
}


// Функции для обновления буфера.
// Используют глобальные переменные ph_ind,  ph_m.

// Обновить заданный фрагмент буфера.
inline void dma_buf_fragment_init(int16_t *begin, const int16_t *end)
{
    int32_t a1 = out_a1>>1; // Предкоррекция во избежание переполнения.
    while (begin != end)
    {
        *begin = out_a0 + (a1 * fast_sin(ph_ind)>>15); // Умножение и посткоррекция.
        ph_ind += ph_m;
        begin++;
    }
}

// Вычислить значения для первой половины буфера.
inline void dma_buf_first_half_init(void)
{
    dma_buf_fragment_init(dma_buf, dma_buf + DMA_BUF_LENGTH/2);
}

// Вычислить значения для второй половины буфера.
inline void dma_buf_second_half_init(void)
{
    dma_buf_fragment_init(dma_buf + DMA_BUF_LENGTH/2, dma_buf + DMA_BUF_LENGTH);
}


/*----------------------------------------------------------------------------*\
Инициализация необходимого оборудования
\*----------------------------------------------------------------------------*/
void sinus_signal_DAC_init(void)
{
   DAC_ChannelConfTypeDef sConfig = {0};
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   hdac.Instance = DAC;
   HAL_DAC_Init(&hdac);
   sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
   sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
   HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
   /**DAC GPIO Configuration
   PA4     ------> DAC_OUT1
   */
   GPIO_InitStruct.Pin = GPIO_PIN_4;
   GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    
}


/*----------------------------------------------------------------------------*\
Инициализация DMA
\*----------------------------------------------------------------------------*/
void sinus_signal_DMA_init(void)
{
   __HAL_RCC_DMA1_CLK_ENABLE();
   HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
   hdma_dac_ch1.Instance = DMA1_Channel3;
   hdma_dac_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
   hdma_dac_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
   hdma_dac_ch1.Init.MemInc = DMA_MINC_ENABLE;
   hdma_dac_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
   hdma_dac_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
   hdma_dac_ch1.Init.Mode = DMA_CIRCULAR;
   hdma_dac_ch1.Init.Priority = DMA_PRIORITY_LOW;
   HAL_DMA_Init(&hdma_dac_ch1);
   __HAL_DMA_REMAP_CHANNEL_ENABLE(HAL_REMAPDMA_TIM6_DAC1_CH1_DMA1_CH3);
   //__HAL_LINKDMA(hdac,DMA_Handle1, hdma_dac_ch1);
}


/*----------------------------------------------------------------------------*\
Инициализация TIM6
\*----------------------------------------------------------------------------*/
void sinus_signal_TIM6_init(void)
{
   TIM_MasterConfigTypeDef sMasterConfig = {0};
   htim6.Instance = TIM6;
   htim6.Init.Prescaler = 0;
   htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
   htim6.Init.Period = 899;
   htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
   HAL_TIM_Base_Init(&htim6);
   sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
   sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
   HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
   __HAL_RCC_TIM6_CLK_ENABLE();
}





/*----------------------------------------------------------------------------*\
Инициализация необходимого оборудования
\*----------------------------------------------------------------------------*/
void sinus_signal_hw_init() 
{
   sinus_signal_DMA_init();
   sinus_signal_TIM6_init();
   sinus_signal_DAC_init();
}


/*----------------------------------------------------------------------------*\
Установка требуемой системной частоты
\*----------------------------------------------------------------------------*/
void sinus_signal_sysclk_init(uint32_t max_sysfreq)
{
   uint8_t i;
   for (i = 0; i < WORK_FREQS_LEN; i++)
      if (work_freqs[i].sysclk > max_sysfreq)
         break;
   i = i - 1;
   //ChangePLL(work_freqs[i].prediv, work_freqs[i].pllmul);      
   logint("freq ", work_freqs[i].sysclk, NEWLINE);
   logint("prediv", work_freqs[i].prediv, NEWLINE);
   logint("pll mul", work_freqs[i].pllmul, NEWLINE);
}

/*----------------------------------------------------------------------------*\
Деинициализация оборудования
\*----------------------------------------------------------------------------*/
void sinus_signal_hw_deinit(void)
{
   HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
   //HAL_DAC_DeInit(&hdac);
   //HAL_DMA_DeInit(hdac->DMA_Handle1);
   __HAL_RCC_TIM6_CLK_DISABLE();
}

/*
void DMA1_Channel3_IRQHandler(void)
{
   if (DMA1->ISR & DMA_ISR_HTIF3)
      dma_buf_first_half_init();
   else
   if (DMA1->ISR & DMA_ISR_TCIF3)
      dma_buf_second_half_init();
   HAL_DMA_IRQHandler(&hdma_dac_ch1);
}
*/

void sinus_signal_change_freq() 
{
   // рассчитать новое приращение фазы
   ph_m = (uint32_t)(4294967296.0 / F0 * 2 * rect_freq);
   // изменить делитель таймера
   __HAL_TIM_SET_PRESCALER(&htim6, FSYSCLK / F0 - 1);
}

/*----------------------------------------------------------------------------*\
Отрисовка экрана
\*----------------------------------------------------------------------------*/
void sinus_signal_draw_screen(void)
{
   rect_signal_draw_screen();
}


/*----------------------------------------------------------------------------*\
Button UP / Увеличение выбранного параметра
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void sinus_signal_key1_handler(void) 
{
   if (change_num == 0) {
      // изменяем частоту
      if (rect_freq + rect_freq_step <= SIN_MAX_FREQ) 
      {
         rect_freq += rect_freq_step;
         sinus_signal_change_freq();
         rect_signal_draw_screen();
      }
   } else
   if (change_num == 1) {
      // изменяем шаг
      if (gen_freq_step_idx < gen_freq_step_size-1)
      {
         gen_freq_step_idx++;
         rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
         rect_signal_draw_screen();
      }
   }
}


/*----------------------------------------------------------------------------*\
Button DOWN / Уменьшение выбранного параметра
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void sinus_signal_key2_handler(void) 
{
   rect_signal_key2_handler();
}


/*----------------------------------------------------------------------------*\
Button LEFT / Выход из модуля в меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void sinus_signal_key3_handler(void){
   // деактивируем всё оборудование, участвовавшее в генерации сигнала
   sinus_signal_hw_deinit();
   // вернём обычные значения предделителя и множителя PLL
   ChangePLL(2, 9);
   // возврат в меню
   esc_func();
}


/*----------------------------------------------------------------------------*\
Button RIGHT / Выбор параметра для изменения
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void sinus_signal_key4_handler(void)
{
   rect_signal_key4_handler();
}



//##########################################################
//## Инициализация модуля 
//##########################################################
void sinus_signal_init(void)
{
   // draw main screen
   DrawMainScreen("Sinus");
   // init variables
   gen_freq_step_idx = 4; // шаг изменения частоты по умолчанию 100 Гц
   rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
   rect_freq = 1000; 
   change_num = 0;
   init_freqs_array();
   // init periferial
   sinus_signal_hw_init();
   sinus_signal_sysclk_init(SINUS_SYSCLK_MAX_FREQ);
   FSYSCLK = HAL_RCC_GetSysClockFreq();
   F0 = FSYSCLK / K;
   sinus_signal_change_freq();
   sinus_signal_draw_screen();
   dma_buf_first_half_init_pointer  = dma_buf_first_half_init;
   dma_buf_second_half_init_pointer = dma_buf_second_half_init;
    
   HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)dma_buf, dma_buf_length, DAC_ALIGN_12B_L);
   HAL_TIM_Base_Start(&htim6);

   // functions
   keys_handlers.pFunc_key1 = sinus_signal_key1_handler;
   keys_handlers.pFunc_key2 = sinus_signal_key2_handler;
   keys_handlers.pFunc_key3 = sinus_signal_key3_handler;
   keys_handlers.pFunc_key4 = sinus_signal_key4_handler;
   pLoop_Func = NULL; }

/*----------------------------------------------------------------------------*/
