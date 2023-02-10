/*
##############################################################################
##############################################################################
##
## Модуль генерации прямоугольных импульсов
## Модуль обеспечивает генерацию импульсов в диапазоне частот от 1 Гц до 
## 100 кГц, используя изменение системной частоты и делители таймера.
## Частота стабильная, артефактов генерируемой частоты нет.
## Максимальное отклонение генерируемой частоты от заданной - не более 120 Гц
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "util.h"
#include "config.h"
#include "rect_signal.h"
#include "screen.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* Variables -----------------------------------------------------------------*/

uint32_t rect_freq;       // заданная частота, Гц
uint32_t rect_freq_real;  // действительно генерируемая частота, Гц
uint16_t rect_freq_step;  // Шаг изменения частоты
uint16_t min_prediv, min_mul, min_psc, min_cnt;
float min_diff, min_freqreal;

/* Functions -----------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
Расчёт системного тактирования и делителей таймера для наилучшей генерации 
заданной частоты. 
Рассчитанные числа будут помещены в min_prediv, min_mul, min_psc, min_cnt
В min_freqreal будет записана реально получившаяся частота
В min_diff будет записано отклонение от заданной частоты
Поиск ведётся методом перебора вариантов предделителя и множителя PLL,
предпочтение будет отдано наименьшей частоте.
Частота таймера TIM1 предполагается удвоенной системной.
\*----------------------------------------------------------------------------*/
void rect_signal_calculate_clocking(uint32_t freq, uint16_t use_only_allowed_freqs)
{
   float diff, mindiff = 9999.0;
   uint32_t sysclk;
   float freqreal;
   uint32_t Kreal;
   uint16_t Kpsc, Kcnt;
   float K;
   for (uint16_t pllmul=2; pllmul<17; pllmul++)
   {
      for (uint16_t prediv=1; prediv<9; prediv++)
      {
          sysclk = Fosc * pllmul / prediv;
          if (((use_only_allowed_freqs == 0) && (sysclk*2 <= SYSCLK_MAX)) ||
             ((use_only_allowed_freqs == 1) && (sysclk >= 16000000)&&(sysclk <= 72000000)))
           {
              K = round((float)Fosc / (float)prediv * (float)pllmul / (float)freq );
              Kpsc = ((uint32_t)K) / 65536 + 1;
              Kcnt = ((uint32_t)K) % 65536;
              Kreal = Kpsc * Kcnt;
              freqreal = (float)Fosc / (float)prediv * (float)pllmul / (float)Kreal;
              diff = freqreal - (float)freq;
              if (mindiff > fabs(diff))
              {
                 mindiff = fabs(diff);
                 min_diff = diff;
                 min_prediv = prediv;
                 min_mul = pllmul;
                 min_psc = Kpsc;
                 min_cnt = Kcnt;
                 min_freqreal = freqreal;
              }
           }
      } // for (uint8_t prediv=1; prediv<9; previd++)
   } // for (uint8_t pllmul=16, pllmul>0; pllmul--)
}  


/*----------------------------------------------------------------------------*\
Инициализация необходимого оборудования
\*----------------------------------------------------------------------------*/
void rect_signal_hw_init(void)
{
   TIM_ClockConfigTypeDef sClockSourceConfig = {0};
   TIM_MasterConfigTypeDef sMasterConfig = {0};
   TIM_OC_InitTypeDef sConfigOC = {0};
 
   htim1.Instance = TIM1;
   htim1.Init.Prescaler = 7199;
   htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
   htim1.Init.Period = 19;
   htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
   htim1.Init.RepetitionCounter = 0;
   htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
 
   HAL_TIM_Base_Init(&htim1);
   
   sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
   HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
   
   HAL_TIM_OC_Init(&htim1);
   
   sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
   sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
   sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
   HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);
   
   sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
   sConfigOC.Pulse = 1;
   sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
   sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
   sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
   sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
   HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4);
 
   __HAL_RCC_TIM1_CLK_ENABLE();

   GPIO_InitTypeDef GPIO_InitStruct = {0};

   __HAL_RCC_GPIOE_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PE14     ------> TIM1_CH4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}


/*----------------------------------------------------------------------------*\
Деинициализация оборудования
\*----------------------------------------------------------------------------*/
void rect_signal_hw_deinit(void)
{
   __HAL_RCC_TIM1_CLK_DISABLE();
   HAL_GPIO_DeInit(GPIOE, GPIO_PIN_14);   
}


/*----------------------------------------------------------------------------*\
Изменение генерируемой частоты
\*----------------------------------------------------------------------------*/
void rect_signal_change_freq(void)
{
   // рассчитаем необходимые числа
   rect_signal_calculate_clocking(rect_freq, USE_ONLY_ALLOWED_FREQS);
   // изменим системную частоту
   ChangePLL(min_prediv, min_mul);
   // изменим делители в таймере
   MODIFY_REG(RCC->CFGR3, RCC_CFGR3_TIM1SW, (uint32_t)(RCC_TIM1CLK_PLLCLK));
   LL_TIM_SetPrescaler(TIM1, min_psc-1);
   LL_TIM_SetAutoReload(TIM1, min_cnt-1);
   //LL_TIM_SetCounter(TIM1, 0);
   LL_TIM_OC_SetCompareCH4(TIM1, 1);
}


/*----------------------------------------------------------------------------*\
Отрисовка экрана
\*----------------------------------------------------------------------------*/
void rect_signal_draw_screen(void)
{
   ST7735_WriteNumberCentered(1, TWOLINE_TEXT_1L_Y, 158, 26, rect_freq, 1, Font_16x26, SCREEN_TEXT_COLOR, SCREEN_BACKGROUND, 0);
   ST7735_WriteNumberCentered(1, TWOLINE_TEXT_2L_Y, 158, 26, rect_freq_step, 1, Font_16x26, SCREEN_TEXT_COLOR, SCREEN_BACKGROUND, 0);

   if (change_num == 0)
      ST7735_WriteString(SCREEN_PARAM_X, SCREEN_PARAM_Y, "Freq", SCREEN_PARAM_FONT, SCREEN_PARAM_COLOR, SCREEN_COLOR,0);
   else
      ST7735_WriteString(SCREEN_PARAM_X, SCREEN_PARAM_Y, "Step", SCREEN_PARAM_FONT, SCREEN_PARAM_COLOR, SCREEN_COLOR,0);
}



/*----------------------------------------------------------------------------*\
Button UP / Увеличение выбранного параметра
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal_key1_handler(void)
{
   if (change_num == 0) {
      // изменяем частоту
      if (rect_freq + rect_freq_step <= RECT_MAX_FREQ) 
      {
         rect_freq += rect_freq_step;
         rect_signal_change_freq();
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
void rect_signal_key2_handler(void)
{
   if (change_num == 0) {
         // изменяем длительность
         if ((rect_freq - rect_freq_step >= RECT_MIN_FREQ) & (rect_freq > rect_freq_step))
         {
            rect_freq -= rect_freq_step;
            rect_signal_change_freq();
            rect_signal_draw_screen();
         }
   } else
   if (change_num == 1) {
         // изменяем шаг
         if (gen_freq_step_idx > 0)
         {
            gen_freq_step_idx--;
            rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
            rect_signal_draw_screen();
         }
   }

}


/*----------------------------------------------------------------------------*\
Button LEFT / Выход из модуля в меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal_key3_handler(void){
   // деактивируем всё оборудование, участвовавшее в генерации сигнала
   rect_signal_hw_deinit();
   // вернём обычные значения предделителя и множителя PLL
   ChangePLL(2, 9);
   // возврат в меню
   esc_func();
}


/*----------------------------------------------------------------------------*\
Button RIGHT / Выбор параметра для изменения
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal_key4_handler(void)
{
   if (change_num == 0)
      change_num = 1;
   else
      change_num = 0;
   rect_signal_draw_screen();
}



//##########################################################
//## Инициализация модуля генерации прямоугольных импульсов
//##########################################################
void rect_signal_init(void)
{
   // draw main screen
   DrawMainScreen("Rectagular");
   // init variables
   gen_freq_step_idx = 4; // шаг изменения частоты по умолчанию 100 Гц
   rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
   rect_freq = 1000; 
   change_num = 0;
   // init periferial
   rect_signal_hw_init();
   rect_signal_change_freq();
   rect_signal_draw_screen();
   HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_4);
   // functions
   keys_handlers.pFunc_key1 = rect_signal_key1_handler;
   keys_handlers.pFunc_key2 = rect_signal_key2_handler;
   keys_handlers.pFunc_key3 = rect_signal_key3_handler;
   keys_handlers.pFunc_key4 = rect_signal_key4_handler;
   pLoop_Func = NULL; 
}

/*----------------------------------------------------------------------------*/
