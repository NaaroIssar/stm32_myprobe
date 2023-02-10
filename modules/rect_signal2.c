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

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "util.h"
#include "config.h"
#include "hardware.h"
#include "rect_signal2.h"
#include "screen.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* Variables -----------------------------------------------------------------*/

double bestpwm, bestsysclk;              // лучшие значения ШИМ и системной частоты для генерации сигнала
uint16_t bestpllmul, bestprediv;         // значения предделителя и множителя ФАПЧ
uint16_t minK16;                         // делитель для таймера-модулятора
uint16_t minK16_2;                       // делитель для таймера-модулятора для второй частоты генерации
uint32_t K3, K3psc, K3cnt;               // делитель выходного таймера
double rpwm, rpwm2;                      // реальные получившиеся PWM
double rfreq, rfreq2;                    // реальные получающиеся частоты
double difffreq;                         // отклонение выходной частоты
uint16_t pwm1, pwm2;                     // количество циклов ШИМ для двухчастотной генерации
uint16_t bestpwm1, bestpwm2;             // значения ШИМ для двухчастотной генерации 
double fpwm2, diff, bestdiff;            // 
double rfreq3;                           // реальная выходная частота при двухчастотной генерации
double adjitter;                         // амплитуда джиттера, нс
double fdjitter;                         // частота джиттера, Гц
double deviation;                        // девиация частоты, Гц
uint8_t method;                          // 0 - одночастотная генерация, 1 - двухчастотная генерация, 3 - TIM1

// буфер DMA для двухчастотной генерации
uint16_t  rect_signal2_buffer[RECT_SIGNAL2_BUFFER_SIZE];

/* Functions -----------------------------------------------------------------*/


/*----------------------------------------------------------------------------*\
Настройка ПДП
\*----------------------------------------------------------------------------*/
void rect_signal2_DMA_init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}


/*----------------------------------------------------------------------------*\
Настройка таймера TIM3 - выходной делитель
\*----------------------------------------------------------------------------*/
void rect_signal2_TIM3_init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2222;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim3);
  
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
  
  HAL_TIM_OC_Init(&htim3);
  
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
  
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
  __HAL_RCC_TIM3_CLK_ENABLE();
}


/*----------------------------------------------------------------------------*\
Настройка таймера TIM16 - модулятор сигнала
\*----------------------------------------------------------------------------*/
void rect_signal2_TIM16_init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 2-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 1111;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim16);
  HAL_TIM_PWM_Init(&htim16);
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  HAL_TIM_PWM_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1);
  
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig);
  __HAL_RCC_TIM16_CLK_ENABLE();
}


/*----------------------------------------------------------------------------*\
Настройка таймера TIM17 - сигнал несущей
\*----------------------------------------------------------------------------*/
void rect_signal2_TIM17_init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 1-1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 1;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim17);
  HAL_TIM_PWM_Init(&htim17);
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  HAL_TIM_PWM_ConfigChannel(&htim17, &sConfigOC, TIM_CHANNEL_1);
  
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim17, &sBreakDeadTimeConfig);
  __HAL_RCC_TIM17_CLK_ENABLE();
}


/*----------------------------------------------------------------------------*\
Настройка выводов
\*----------------------------------------------------------------------------*/
void rect_signal2_GPIO_init(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};
  
   __HAL_RCC_GPIOB_CLK_ENABLE();
   /**TIM3 GPIO Configuration
   PB3     ------> TIM3_ETR
   */
   GPIO_InitStruct.Pin = GPIO_PIN_3;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_TIM3;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /**TIM3 GPIO Configuration
   PB5     ------> TIM3_CH2
   */
   GPIO_InitStruct.Pin = GPIO_PIN_5;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /** Configure pin
   PB9   ------> IR_OUT
   */
   GPIO_InitStruct.Pin = GPIO_PIN_9;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF6_IR;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


/*----------------------------------------------------------------------------*\
Инициализация необходимого оборудования
\*----------------------------------------------------------------------------*/
void rect_signal2_hw_init(void)
{
   rect_signal2_DMA_init();
   rect_signal2_TIM17_init();
   rect_signal2_TIM16_init();
   rect_signal2_TIM3_init();
   rect_signal2_GPIO_init();
   HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_2);
   HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
}


/*----------------------------------------------------------------------------*\
Деинициализация оборудования
\*----------------------------------------------------------------------------*/
void rect_signal2_hw_deinit(void)
{
   __HAL_RCC_DMA1_CLK_DISABLE();
   __HAL_RCC_TIM16_CLK_DISABLE();
   __HAL_RCC_TIM17_CLK_DISABLE();
   __HAL_RCC_TIM3_CLK_DISABLE();
   HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);
   HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_9);
}


/*----------------------------------------------------------------------------*\
Подбор лучшей рабочей системной частоты.
Лучшая частота - та, при которой необходимое значение ШИМ максимально.
Но значение ШИМ не должно превышать 0,99998474 - иначе мы не сможем его
реализовать на 16-битном таймере. Максимально возможное значение
ШИМ = (65535-1)/65535 = 0,9999847409781033.
В идеале ШИМ должен уложиться в диапазон от 0.99 до 0,9999847409781033.
И ещё - модулятор IR не работает на частоте выше 18 МГц, то есть на
таймеры TIM16, TIM17 можно подавать тактовую частоту не выше 36 МГц.
Меньше 8 МГц на таймеры тоже нежелательно подавать - будет большая
амплитуда джиттера.
\*----------------------------------------------------------------------------*/
void rect_signal2_calculate_best_sysclk(uint32_t freq)
{
   double sysclk, pwm;
   uint32_t K3;
   uint32_t K3psc, K3cnt;
   bestpwm = 0;
   bestpllmul = 0;
   bestprediv = 0;
   for (uint16_t i = 0; i < WORK_FREQS_LEN; i++)
   {
      sysclk = work_freqs[i].sysclk;
      if ((sysclk >= IR_OUT_MIN_FREQ) && (sysclk <= IR_OUT_MAX_FREQ))
      {
         K3 = (uint32_t)((sysclk / 2) / (double)freq / 2);
         K3psc = K3 / 65536 + 1;
         K3cnt = K3 % 65536;
         K3 = K3psc * K3cnt;
         pwm = (double)freq * (K3) / (sysclk / 2) * 2;
         if (pwm < 0.99998474)
         {
            if (((bestpwm < pwm) && (pwm < 0.99998474)) || ((bestpwm == pwm) && (bestsysclk < sysclk)))
            {
               bestpwm = pwm;
               bestsysclk = sysclk;
               bestpllmul = work_freqs[i].pllmul;
               bestprediv = work_freqs[i].prediv;
            } // if (((bestpwm  
         } // if (pwm<0.99998)
      } // if (sysclk <=
   } // for (uint16_t i=0;
}


/*----------------------------------------------------------------------------*\
Расчёт количествa двух PWM для двухчастотной генерации
Рассчитываются количество импульсов для каждой из двух частот, чтобы при
выбранных значениях результирующий PWM минимально отличался от заданного.
В сумме рассчитанные количества импульсов не должны превышать размер буфера DMA.
bestpwm - целевое значение PWM, к которому нужно стремиться 
rpwm, rpwm2 - получающиеся значения PWM рядом с bestpwm, комбинируя которые
нужно добиться наилучшего совпадения с bestpwm.
Результат записывается в bestpwm1 и bestpwm2
\*----------------------------------------------------------------------------*/
void rect_signal2_calculate_pwm2(double bestpwm, double rpwm, double rpwm2)
{
   bestdiff = 9;
   double k = (bestpwm - rpwm) / (rpwm2 - bestpwm);
   for (pwm1 = 10; pwm1 < RECT_SIGNAL2_BUFFER_SIZE; pwm1++)
   {
      fpwm2 = k * pwm1;
      pwm2 = (uint16_t)fpwm2;
      diff = fpwm2 - pwm2;
      if ((pwm1 + pwm2) >= RECT_SIGNAL2_BUFFER_SIZE) break;
      if (bestdiff > diff)
      {
         bestdiff = diff;
         bestpwm1 = pwm1;
         bestpwm2 = pwm2;
      }
   }
}


/*----------------------------------------------------------------------------*\
Изменение генерируемой частоты
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal2_change_freq(void)
{
   uint16_t two_pwm_realizeble = 1;
   // рассчитаем наиболее подходящую системную частоту
   rect_signal2_calculate_best_sysclk(rect_freq);
   // рассчитаем делитель выходного таймера
   K3 = bestsysclk / 2 / rect_freq / 2;
   K3psc = ((uint32_t)K3) / 65536 + 1;
   K3cnt = ((uint32_t)K3) % 65536;
   K3 = K3psc * K3cnt;
   // рассчитаем делитель для таймера-модулятора
   minK16 = (uint16_t)(1.0 / (1.0 - bestpwm));
   minK16_2 = minK16 + 1;
   // параметры второй частоты
   rpwm = ((double)minK16 - 1.0) / (double)minK16;
   rpwm2 = ((double)minK16_2 - 1.0) / (double)minK16_2;
   double k = (bestpwm - rpwm) / (rpwm2 - bestpwm);
   // если соотношение частот слишком невыгодное, 
   // то двухчастотная генерация нереализуема (с заданным размером буфера)
   if (k > (RECT_SIGNAL2_BUFFER_SIZE - 50)) two_pwm_realizeble = 0;
   minK16 = (uint16_t)round(1.0 / (1.0 - ((double)rect_freq * (double)K3 / (bestsysclk / 2.0) * 2.0)));
   // рассчитаем реально получившиеся параметры сигнала
   rpwm = ((double)minK16-1.0) / (double)minK16;
   rfreq = ((bestsysclk / 2) * rpwm / (double)K3 / 2);
   difffreq = rfreq - (double)rect_freq;
   adjitter = 2000000000 / bestsysclk;
   fdjitter = bestsysclk / 2 / minK16;
   // Изменим системную частоту
   ChangePLL(bestprediv, bestpllmul);
   // Проверим отклонение генерируемой частоты от заданной
   if ((fabs(difffreq) > RECT_SIGNAL2_MAX_ALLOWED_DIFFERENCE) && (RECT_SIGNAL2_TWO_FREQ_GENERATION != 0)
       && (two_pwm_realizeble != 0))
   {
      // отклонение частоты слишком большое, применяем двухчастотную генерацию
      minK16 = (uint16_t)(1.0 / (1.0 - bestpwm));
      rpwm = ((double)minK16-1.0) / (double)minK16;
      rfreq = ((bestsysclk / 2) * rpwm / (double)K3 / 2);
      // параметры второй частоты
      minK16_2 = minK16 + 1;
      rpwm2 = ((double)minK16_2-1.0) / (double)minK16_2;
      rfreq2 = ((bestsysclk / 2) * rpwm2 / (double)K3 / 2);
      difffreq = rfreq2 - (double)rect_freq;
      // найдём два числа для двухчастотной генерации
      rect_signal2_calculate_pwm2(bestpwm, rpwm, rpwm2);
      // рассчитаем реально получившиеся параметры сигнала
      rfreq3 = rfreq * bestpwm1 / (bestpwm1 + bestpwm2) + rfreq2 * bestpwm2 / (bestpwm1 + bestpwm2);
      difffreq = rfreq3 - rect_freq;
      deviation = MAX(fabs(rfreq - rect_freq), fabs(rfreq2 - rect_freq));
      // параметры - bestprediv, bestpllmul, K3, buffer_size, minK16, bestpwm1, minK16_2, bestpwm2 
      // частота фактическая rfreq3, отклонение difffreq, девиация deviation, амплитуда джиттера adjitter, частота джиттера fdjitter
      //
      // заполним буфер DMA
      minK16--; minK16_2--;
      for (uint16_t i1 = 0; i1 < bestpwm1; i1++)
         rect_signal2_buffer[i1] = minK16; // первая половина буфера для pwm1
      for (uint16_t i2 = bestpwm1; i2 < bestpwm2+bestpwm1; i2++)
         rect_signal2_buffer[i2] = minK16_2; // вторая половина буфера для pwm2
      // если ранее была двухчастотная генерация, нужно остановить DMA
      if (method == RECT_SIGNAL2_TWO_FREQ)
      {
         HAL_TIM_DMABurst_WriteStop(&htim16, TIM_DMA_UPDATE);
      }
      // таймер не останавливаем, просто меняем параметры
      LL_TIM_SetPrescaler(TIM3, K3psc - 1);
      LL_TIM_SetAutoReload(TIM3, K3cnt - 1); 
      LL_TIM_SetCounter(TIM3, 0);
      //LL_TIM_SetAutoReload(TIM16, 33333 - 1);
      // нужно изменить параметры и перезапустить DMA
      HAL_TIM_DMABurst_MultiWriteStart(&htim16, TIM_DMABASE_ARR, TIM_DMA_UPDATE, (uint32_t*)rect_signal2_buffer, TIM_DMABURSTLENGTH_1TRANSFER, bestpwm1+bestpwm2);
      method = RECT_SIGNAL2_TWO_FREQ;
   } else { // if ((fabs(difffreq) 
      // отклонение частоты не превышает установленный максимум, одночастотная генерация
      // или двухчастотная генерация запрещена
      // параметры - bestprediv, bestpllmul, minK16, K3
      // частота фактическая rfreq, отклонение difffreq, амплитуда джиттера adjitter, частота джиттера fdjitter
      if (method == RECT_SIGNAL2_TWO_FREQ)
      {
         // ранее была двухчастотная генерация - нужно отключить DMA
         HAL_TIM_DMABurst_WriteStop(&htim16, TIM_DMA_UPDATE);
      }
      // таймер не останавливаем, просто меняем параметры
      LL_TIM_SetPrescaler(TIM3, K3psc - 1);
      LL_TIM_SetAutoReload(TIM3, K3cnt - 1); 
      LL_TIM_SetCounter(TIM3, 0);
      LL_TIM_SetAutoReload(TIM16, minK16 - 1 - 1);
      
      method = RECT_SIGNAL2_ONE_FREQ;
   }
}


/*----------------------------------------------------------------------------*\
Отрисовка экрана
\*----------------------------------------------------------------------------*/
void rect_signal2_draw_screen(void)
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
void rect_signal2_key1_handler(void)
{
   if (change_num == 0) {
      // изменяем частоту
      if (rect_freq + rect_freq_step <= RECT_SIGNAL2_MAX_FREQ) 
      {
         rect_freq += rect_freq_step;
         rect_signal2_change_freq();
         rect_signal2_draw_screen();
      }
   } else
   if (change_num == 1) {
      // изменяем шаг
      if (gen_freq_step_idx < gen_freq_step_size-1)
      {
         gen_freq_step_idx++;
         rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
         rect_signal2_draw_screen();
      }
   }
}


/*----------------------------------------------------------------------------*\
Button DOWN / Уменьшение выбранного параметра
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal2_key2_handler(void)
{
   if (change_num == 0) {
         // изменяем длительность
         if ((rect_freq - rect_freq_step >= RECT_SIGNAL2_MIN_FREQ) & (rect_freq > rect_freq_step))
         {
            rect_freq -= rect_freq_step;
            rect_signal2_change_freq();
            rect_signal2_draw_screen();
         }
   } else
   if (change_num == 1) {
         // изменяем шаг
         if (gen_freq_step_idx > 0)
         {
            gen_freq_step_idx--;
            rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
            rect_signal2_draw_screen();
         }
   }

}


/*----------------------------------------------------------------------------*\
Button LEFT / Выход из модуля в меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal2_key3_handler(void){
   // деактивируем всё оборудование, участвовавшее в генерации сигнала
   rect_signal2_hw_deinit();
   // вернём обычные значения предделителя и множителя PLL
   ChangePLL(2, 9);
   // возврат в меню
   esc_func();
}


/*----------------------------------------------------------------------------*\
Button RIGHT / Выбор параметра для изменения
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void rect_signal2_key4_handler(void)
{
   if (change_num == 0)
      change_num = 1;
   else
      change_num = 0;
   rect_signal_draw_screen();
}



//##########################################################
//## Инициализация модуля генерации прямоугольных импульсов2
//##########################################################
void rect_signal2_init(void)
{
   // draw main screen
   DrawMainScreen("Rectagular2");
   // init variables
   init_freqs_array(); // заполним таблицу рабочих частот
   gen_freq_step_idx = 4; // шаг изменения частоты по умолчанию 100 Гц
   rect_freq_step = gen_freq_step_arr[gen_freq_step_idx];
   rect_freq = 1000;
   change_num = 0;
   method = RECT_SIGNAL2_ONE_FREQ;
   // init periferial
   rect_signal2_hw_init();
   rect_signal2_change_freq();
   rect_signal2_draw_screen();
   // functions
   keys_handlers.pFunc_key1 = rect_signal2_key1_handler;
   keys_handlers.pFunc_key2 = rect_signal2_key2_handler;
   keys_handlers.pFunc_key3 = rect_signal2_key3_handler;
   keys_handlers.pFunc_key4 = rect_signal2_key4_handler;
   pLoop_Func = NULL; 
}

/*----------------------------------------------------------------------------*/
