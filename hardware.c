/*
##############################################################################
##############################################################################
##
## Функции настройки оборудования контроллера
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "lcd.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "util.h"
#include "hardware.h"

/* Variables -----------------------------------------------------------------*/

SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;
DMA_HandleTypeDef hdma_tim16_ch1_up;
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac_ch1;

/* Functions -----------------------------------------------------------------*/


/*----------------------------------------------------------------------------*\
Настройка системы тактирования
\*----------------------------------------------------------------------------*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
  
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_TIM1;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_PLLCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);

  HAL_RCC_EnableCSS();
}


/*----------------------------------------------------------------------------*\
Изменение системной частоты
Изменяется делитель генератора в внешним кварцем и множитель ФАПЧ (PLL)
Позволяет установить системную частоту от 4 до 144 МГц
Коды ошибок:
0 - всё нормально
1 - не запустился встроенный RC генератор
2 - не запускается генератор с внешним кварцем
3 - не удаётся запустить ФАПЧ
\*----------------------------------------------------------------------------*/
uint8_t ChangePLL(uint8_t PreDiv, uint8_t PLLMUL)
{
   // включим HSI 
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET) 
   {
      __HAL_RCC_HSI_ENABLE();
      uint32_t tickstart = HAL_GetTick();
      while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY)  == RESET)
      {
        if((HAL_GetTick() - tickstart ) > 100) // 100 ms
        {
          return 1;
        }
      }
   }
   
   // установим HSI источником системного тактирования
   __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);

   // отключим PLL
   __HAL_RCC_PLL_DISABLE();

   // включим HSE
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET) 
   {
      __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
      uint32_t tickstart = HAL_GetTick();
      while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY)  == RESET)
      {
        if((HAL_GetTick() - tickstart ) > 100) // 100 ms
        {
          return 2;
        }
      }
   }

   // уменьшим до минимума множитель PLL
   // чтобы при изменении делителя HSE не получить нерабочую частоту PLL
   __HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL2);
   
   // установим делитель HSE
   switch (PreDiv)
   {
      case 2: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV2); break;
      case 3: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV3); break;
      case 4: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV4); break;
      case 5: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV5); break;
      case 6: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV6); break;
      case 7: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV7); break;
      case 8: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV8); break;
      case 9: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV9); break;
      case 10: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV10); break;
      case 11: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV11); break;
      case 12: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV12); break;
      case 13: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV13); break;
      case 14: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV14); break;
      case 15: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV15); break;
      case 16: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV16); break;
      default: __HAL_RCC_HSE_PREDIV_CONFIG(RCC_HSE_PREDIV_DIV1); break;
   }
   
   // установим множитель PLL
   switch (PLLMUL)
   {
      case  2: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL2);  break; }
      case  3: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL3);  break; }
      case  4: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL4);  break; }
      case  5: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL5);  break; }
      case  6: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL6);  break; }
      case  8: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL8);  break; }
      case  9: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL9);  break; }
      case 10: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL10); break; }
      case 11: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL11); break; }
      case 12: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL12); break; }
      case 13: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL13); break; }
      case 14: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL14); break; }
      case 15: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL15); break; }
      case 16: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL16); break; }
      default: {__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, RCC_PLL_MUL7);  break; }
   }

   // запустим PLL
   __HAL_RCC_PLL_ENABLE();
      
   // дождёмся запуска PLL
   uint32_t tickstart = HAL_GetTick();
   while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)  == RESET)
   {
     if((HAL_GetTick() - tickstart ) > 100) // 100 ms
     {
       return 3;
     }
   }
   
   // установим PLL в качестве источника системного тактирования
   __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);

   // Настроим SysTick в соответствии с новой системной частотой   
   SystemCoreClock = HAL_RCC_GetSysClockFreq();
   HAL_InitTick(uwTickPrio);
   return 0;
}


/*----------------------------------------------------------------------------*\
Настройка SPI
\*----------------------------------------------------------------------------*/
void MX_SPI2_Init(void)
{
   hspi2.Instance = SPI2;
   hspi2.Init.Mode = SPI_MODE_MASTER;
   hspi2.Init.Direction = SPI_DIRECTION_2LINES;
   hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
   hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
   hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
   hspi2.Init.NSS = SPI_NSS_SOFT;
   hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
   hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
   hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
   hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
   hspi2.Init.CRCPolynomial = 10;
   HAL_SPI_Init(&hspi2);
   
   __HAL_RCC_SPI2_CLK_ENABLE();
   __HAL_SPI_ENABLE(&hspi2);
}


/*----------------------------------------------------------------------------*\
Настройка основных выводов
\*----------------------------------------------------------------------------*/
void MX_GPIO_Init(void)
{
   
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   /* GPIO Ports Clock Enable */
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOF_CLK_ENABLE();

   /*Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(GPIOB, LCD_DC_Pin|LCD_CS_Pin|LCD_RST_Pin, GPIO_PIN_RESET);
 
   /*Configure GPIO pins : LCD_DC_Pin LCD_CS_Pin LCD_RST_Pin */
   GPIO_InitStruct.Pin = LCD_DC_Pin|LCD_CS_Pin|LCD_RST_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
   /*Configure GPIO pin : PA8 --> MCO */
   GPIO_InitStruct.Pin = GPIO_PIN_8;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   /**SPI2 GPIO Configuration
   PB13     ------> SPI2_SCK
   PB15     ------> SPI2_MOSI
   */
   GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   HAL_GPIO_WritePin(GPIOE, LD4_Pin|LD3_Pin|LD5_Pin|LD7_Pin|LD9_Pin|LD10_Pin|/*LD8_Pin|*/LD6_Pin, GPIO_PIN_RESET);
   GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD7_Pin|LD9_Pin|LD10_Pin|/*LD8_Pin|*/LD6_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

   /*Configure GPIO pins : KEY1_Pin KEY4_Pin */
   GPIO_InitStruct.Pin = KEY1_Pin|KEY4_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 
   /*Configure GPIO pins : KEY2_Pin KEY3_Pin */
   GPIO_InitStruct.Pin = KEY2_Pin|KEY3_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
   HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}


/*----------------------------------------------------------------------------*\
Настройка UART
\*----------------------------------------------------------------------------*/
void MX_USART1_UART_Init(void)
{
   huart1.Instance = USART1;
   huart1.Init.BaudRate = 115200;
   huart1.Init.WordLength = UART_WORDLENGTH_8B;
   huart1.Init.StopBits = UART_STOPBITS_1;
   huart1.Init.Parity = UART_PARITY_NONE;
   huart1.Init.Mode = UART_MODE_TX_RX;
   huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   huart1.Init.OverSampling = UART_OVERSAMPLING_16;
   huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
   huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
   HAL_UART_Init(&huart1);
   __HAL_RCC_USART1_CLK_ENABLE();
   __HAL_USART_ENABLE(&huart1);
}


/*----------------------------------------------------------------------------*\
Продолжение настройки UART
Функция вызывается из HAL в ответ на вызов HAL_UART_Init
\*----------------------------------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(huart->Instance==USART1)
   {
      __HAL_RCC_USART1_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
      GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
   }
}


/*----------------------------------------------------------------------------*\
обработчик прерывания 5-9
\*----------------------------------------------------------------------------*/
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY2_Pin);
  HAL_GPIO_EXTI_IRQHandler(KEY3_Pin);
  // после прерывания HAL вызовет HAL_GPIO_EXTI_Callback(GPIO_Pin)
  // который находится в menu.c
}


/*----------------------------------------------------------------------------*\
обработчик прерывания 10-15
\*----------------------------------------------------------------------------*/
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
  HAL_GPIO_EXTI_IRQHandler(KEY4_Pin);
  // после прерывания HAL вызовет HAL_GPIO_EXTI_Callback(GPIO_Pin)
  // который находится в menu.c
}


/*----------------------------------------------------------------------------*\
Функция показывает содержимое регистра RCC_CFGR2
используется для отладки
\*----------------------------------------------------------------------------*/
void DebugExplainRCC_CFGR2(uint32_t CFGR2)
{
   uint8_t PREDIV = (CFGR2 & 0b1111);   

   logstr("CFGR2:", NEWLINE);
   logint("PREDIV", PREDIV+1, NEWLINE);
   logstr("", NEWLINE);
}


/*----------------------------------------------------------------------------*\
Функция показывает содержимое регистра RCC_CFGR
используется для отладки
\*----------------------------------------------------------------------------*/
void DebugExplainRCC_CFGR(uint32_t CFGR)
{
   uint8_t MCO      = (CFGR & 0b00000111000000000000000000000000) >> 24;   
   uint8_t PLLMUL   = (CFGR & 0b00000000001111000000000000000000) >> 18;
   uint8_t PLLXTPRE = (CFGR & 0b00000000000000100000000000000000) >> 17;
   uint8_t PLLSRC   = (CFGR & 0b00000000000000010000000000000000) >> 16;
   uint8_t ADCPRE   = (CFGR & 0b00000000000000001100000000000000) >> 14;
   uint8_t PPRE2    = (CFGR & 0b00000000000000000011100000000000) >> 11;
   uint8_t PPRE1    = (CFGR & 0b00000000000000000000011100000000) >> 8;
   uint8_t HPRE     = (CFGR & 0b00000000000000000000000011110000) >> 4;
   uint8_t SWS      = (CFGR & 0b00000000000000000000000000001100) >> 2;
   uint8_t SW       = (CFGR & 0b00000000000000000000000000000011);

   logstr("CFGR:", NEWLINE);
   logint("MCO       ", MCO, NONEWLINE);
   switch (MCO) {
      case 7:  logstr(" (PLL/2)",     NEWLINE); break;
      case 6:  logstr(" (HSE)",       NEWLINE); break;
      case 5:  logstr(" (HSI)",       NEWLINE); break;
      case 4:  logstr(" (SYSCLK)",    NEWLINE); break;
      default: logstr(" (No output)", NEWLINE); break;
   }

   logint("PLLMUL    ", PLLMUL+2, NEWLINE);

   logint("PLLXTPRE  ", PLLXTPRE, NONEWLINE);
   if (PLLXTPRE == 0)
      logstr(" (PREDIV=1)", NEWLINE);
   else
      logstr("(PREDIV>1)", NEWLINE);

   logint("PLLSRC    ", PLLSRC, NONEWLINE);
   if (PLLSRC == 0)
      logstr(" (HSI/2)", NEWLINE);
   else
      logstr(" (PREDIV)", NEWLINE);

   logint("ADCPRE    ", 2*(ADCPRE+1), NEWLINE);
   
   logint("PPRE2     ", PPRE2, NONEWLINE);
   switch (PPRE2) {
      case 7:  logstr(" (HCLK/16)", NEWLINE); break;
      case 6:  logstr(" (HCLK/8)", NEWLINE); break;
      case 5:  logstr(" (HCLK/4)", NEWLINE); break;
      case 4:  logstr(" (HCLK/2)", NEWLINE); break;
      default: logstr(" (HCLK)", NEWLINE); break;
   }
   
   logint("PPRE1     ", PPRE1, NONEWLINE);
   switch (PPRE1) {
      case 7:  logstr(" (HCLK/16)", NEWLINE); break;
      case 6:  logstr(" (HCLK/8)", NEWLINE); break;
      case 5:  logstr(" (HCLK/4)", NEWLINE); break;
      case 4:  logstr(" (HCLK/2)", NEWLINE); break;
      default: logstr(" (HCLK)", NEWLINE); break;
   }
   
   logint("HPRE      ", HPRE, NONEWLINE);
   switch (HPRE) {
      case  8: logstr(" (SYSCLK/2)", NEWLINE); break;
      case  9: logstr(" (SYSCLK/4)", NEWLINE); break;
      case 10: logstr(" (SYSCLK/8)", NEWLINE); break;
      case 11: logstr(" (SYSCLK/16)", NEWLINE); break;
      case 12: logstr(" (SYSCLK/64)", NEWLINE); break;
      case 13: logstr(" (SYSCLK/128)", NEWLINE); break;
      case 14: logstr(" (SYSCLK/256)", NEWLINE); break;
      case 15: logstr(" (SYSCLK/512)", NEWLINE); break;
      default: logstr(" (SYSCLK)", NEWLINE); break;
   }
   
   logint("SWS       ", SWS, NONEWLINE);
   switch (SWS) {
      case 0: logstr(" (HSI)", NEWLINE); break;
      case 1: logstr(" (HSE)", NEWLINE); break;
      case 2: logstr(" (PLL)", NEWLINE); break;
      case 3: logstr(" (ERR)", NEWLINE); break;
   }
   
   logint("SW        ", SW, NONEWLINE);
   switch (SW) {
      case 0: logstr(" (HSI)", NEWLINE); break;
      case 1: logstr(" (HSE)", NEWLINE); break;
      case 2: logstr(" (PLL)", NEWLINE); break;
      case 3: logstr(" (ERR)", NEWLINE); break;
   }
   
   logstr("", NEWLINE);
}


/*----------------------------------------------------------------------------*\
Функция показывает содержимое регистра RCC_CR
используется для отладки
\*----------------------------------------------------------------------------*/
void DebugExplainRCC_RC(uint32_t CR)
{
   uint8_t PLLRDY  = (CR & 0b00000010000000000000000000000000) >> 25;   
   uint8_t PLLON   = (CR & 0b00000001000000000000000000000000) >> 24;
   uint8_t CSSON   = (CR & 0b00000000000010000000000000000000) >> 19;
   uint8_t HSEBYP  = (CR & 0b00000000000001000000000000000000) >> 18;
   uint8_t HSERDY  = (CR & 0b00000000000000100000000000000000) >> 17;
   uint8_t HSEON   = (CR & 0b00000000000000010000000000000000) >> 16;
   uint8_t HSITRIM = (CR & 0b00000000000000000000000011111000) >> 3;
   uint8_t HSIRDY  = (CR & 0b00000000000000000000000000000010) >> 1;
   uint8_t HSION   = (CR & 0b00000000000000000000000000000001);
   
   logstr("CR:", NEWLINE);
   
   logint("PLLRDY     ", PLLRDY, NEWLINE);
   logint("PLLON      ", PLLON, NEWLINE);
   logint("CSSON      ", CSSON, NEWLINE);
   logint("HSEBYP     ", HSEBYP, NEWLINE);
   logint("HSERDY     ", HSERDY, NEWLINE);
   logint("HSEON      ", HSEON, NEWLINE);
   logint("HSITRIM    ", HSITRIM, NEWLINE);
   logint("HSIRDY     ", HSIRDY, NEWLINE);
   logint("HSION      ", HSION, NEWLINE);
}


/*----------------------------------------------------------------------------*\
Функция кратко показывает ситуацию системного тактирования
\*----------------------------------------------------------------------------*/
void DebugShowRCC(uint32_t CR, uint32_t CFGR, uint32_t CFGR2)
{
   uint32_t dGEN = 1;
   uint32_t dDIV = 1;
   uint32_t dMUL = 1;
   uint8_t SWS      = (CFGR & 0b00000000000000000000000000001100) >> 2; 
   uint8_t PREDIV   = (CFGR2 & 0b1111);   
   uint8_t PLLRDY   = (CR & 0b00000010000000000000000000000000) >> 25;   
   uint8_t PLLON    = (CR & 0b00000001000000000000000000000000) >> 24;
   uint8_t HSERDY   = (CR & 0b00000000000000100000000000000000) >> 17;
   uint8_t HSEON    = (CR & 0b00000000000000010000000000000000) >> 16;
   //uint8_t HSITRIM  = (CR & 0b00000000000000000000000011111000) >> 3;
   uint8_t HSIRDY   = (CR & 0b00000000000000000000000000000010) >> 1;
   uint8_t HSION    = (CR & 0b00000000000000000000000000000001);
   uint8_t PLLXTPRE = (CFGR & 0b00000000000000100000000000000000) >> 17;
   uint8_t PLLSRC   = (CFGR & 0b00000000000000010000000000000000) >> 16;
   uint8_t PLLMUL   = (CFGR & 0b00000000001111000000000000000000) >> 18;
   
   logstr("System clock source: ", NONEWLINE);
   
   if (SWS == 0) 
   {
      // system clock from HSI
      dGEN = 8000000;
      dDIV = 1;
      dMUL = 1;
      logstr("HSI", NEWLINE);
      logint("HSIRDY     ", HSIRDY, NEWLINE);
      logint("HSION      ", HSION, NEWLINE);
   }
   else if (SWS == 1)
   {
      // system clock from HSE
      dGEN = 16000000;
      dDIV = 1;
      dMUL = 1;
      logstr("HSE", NEWLINE);
      logint("HSERDY     ", HSERDY, NEWLINE);
      logint("HSEON      ", HSEON, NEWLINE);
   }
   else if (SWS == 2)
   {
      // system clock from PLL
      logstr("PLL", NEWLINE);
      logint("PLLRDY     ", PLLRDY, NEWLINE);
      logint("PLLON      ", PLLON, NEWLINE);
      logint("PLLMUL     ", PLLMUL+2, NEWLINE);
      logint("PLLSRC     ", PLLSRC, NONEWLINE);
      
      dMUL = PLLMUL+2;
      HAL_UART_Transmit(&huart1, (uint8_t*)tmp_str, strlen(tmp_str), 1000);
      if (PLLSRC == 0){
         dGEN = 4000000;
         dDIV = 1;
         logstr(" (HSI/2)", NEWLINE);
         logint("HSIRDY     ", HSIRDY, NEWLINE);
         logint("HSION      ", HSION, NEWLINE);
      } else {
         dGEN = 16000000;
         if (PLLXTPRE == 0) {
            logstr(" (PREDIV=1)", NEWLINE);
            dDIV = 1;
         } else {
            dDIV = PREDIV+1;
            logint(" (PREDIV=", PREDIV+1, NONEWLINE);
            logstr(")", NEWLINE);
         }
      }
   }
   logint("SYSCLK = ", (dGEN * dMUL) / dDIV, NONEWLINE);
   logint("SYSCLK = ", HAL_RCC_GetSysClockFreq(), NONEWLINE);
}

   //DebugExplainRCC_RC((uint32_t)(RCC->CR));
   //DebugExplainRCC_CFGR((uint32_t)(RCC->CFGR));
   //DebugExplainRCC_CFGR2((uint32_t)(RCC->CFGR2));
   //DebugShowRCC((uint32_t)(RCC->CR), (uint32_t)(RCC->CFGR), (uint32_t)(RCC->CFGR2));
   

/*----------------------------------------------------------------------------*\
Запуск системного тактирования
\*----------------------------------------------------------------------------*/
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}


/*----------------------------------------------------------------------------*\
Функция продолжения настройки SPI
функция вызывается из HAL в ответ на вызов HAL_SPI_Init
\*----------------------------------------------------------------------------*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI2)
  {
    __HAL_RCC_SPI2_CLK_ENABLE();
  }
}


/*----------------------------------------------------------------------------*\
Функция продолжения настройки таймеров
функция вызывается из HAL после вызова HAL_TIM_Base_Init
\*----------------------------------------------------------------------------*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
   if(tim_baseHandle->Instance==TIM1)
   {
      __HAL_RCC_TIM1_CLK_ENABLE();
   }
  
   if(tim_baseHandle->Instance==TIM16)
   {
      __HAL_RCC_TIM16_CLK_ENABLE();
      hdma_tim16_ch1_up.Instance = DMA1_Channel3;
      hdma_tim16_ch1_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
      hdma_tim16_ch1_up.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_tim16_ch1_up.Init.MemInc = DMA_MINC_ENABLE;
      hdma_tim16_ch1_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      hdma_tim16_ch1_up.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      hdma_tim16_ch1_up.Init.Mode = DMA_CIRCULAR;
      hdma_tim16_ch1_up.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      HAL_DMA_Init(&hdma_tim16_ch1_up);
      __HAL_LINKDMA(tim_baseHandle, hdma[TIM_DMA_ID_UPDATE], hdma_tim16_ch1_up);
   }
  
   if(tim_baseHandle->Instance==TIM17)
   {
      __HAL_RCC_TIM17_CLK_ENABLE();
   }
  
   if(tim_baseHandle->Instance==TIM3)
   {
      __HAL_RCC_TIM3_CLK_ENABLE();
   }
}


/*----------------------------------------------------------------------------*\
Настройка DAC
Функция вызывается из HAL.
\*----------------------------------------------------------------------------*/
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{ 
   __HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac_ch1); 
}


/*----------------------------------------------------------------------------*\
Обработчик прерывания DMA
Функция вызывается из HAL.
\*----------------------------------------------------------------------------*/
void DMA1_Channel3_IRQHandler(void)
{
   if (DMA1->ISR & DMA_ISR_HTIF3)
      dma_buf_first_half_init_pointer();
   else
   if (DMA1->ISR & DMA_ISR_TCIF3)
      dma_buf_second_half_init_pointer();
   HAL_DMA_IRQHandler(&hdma_dac_ch1);
   HAL_DMA_IRQHandler(&hdma_tim16_ch1_up);
}


/*----------------------------------------------------------------------------*\
Инициализация массива рабочих частот
\*----------------------------------------------------------------------------*/
void init_freqs_array(void)
{
   for (uint16_t i = 0; i < WORK_FREQS_LEN; i++)
      work_freqs[i].sysclk = (double)Fosc * work_freqs[i].pllmul / work_freqs[i].prediv;
}



/*----------------------------------------------------------------------------*\
Находит самую актуальную конфигуацию в FLASH
\*----------------------------------------------------------------------------*/
void find_last_config_in_FLASH(void)
{

}

/*----------------------------------------------------------------------------*\
Читает конфигурацию из FLASH
\*----------------------------------------------------------------------------*/
void read_config_from_flash(void)
{

}


/*----------------------------------------------------------------------------*\
Записывает конфигурацию в FLASH
\*----------------------------------------------------------------------------*/
void write_config_to_flash(void)
{

}


/*----------------------------------------------------------------------------*\
Установка дефолтных значений конфигурации
\*----------------------------------------------------------------------------*/
void set_defaults_to_config(void)
{
   control_data.Fos = Fosc;
   //control_data.Control;
   //control_data.Menu = &menu1;
   //control_data.Frect;    // последняя частота генератора прямоугольных импульсов
   //control_data.Frect2;   // последняя частота генератора прямоугольных импульсов 2
   //control_data.Fsin;     // последняя частота генератора синусоиды
   //control_data.Timp;     // последняя длительность генератора одиночных импульсов
}

/*----------------------------------------------------------------------------*\

\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

// подстройка частоты HSI
// https://studfile.net/preview/16485874/page:75/

// http://dimoon.ru/obuchalka/stm32f1/uroki-stm32f103-chast-4-nastroyka-rcc.html

// http://microsin.net/programming/arm/stm32f4xx-advanced-control-timers-tim1-tim8.html
// https://istarik.ru/blog/stm32/138.html  HAL_TIM_DMABurst_MultiWriteStart
// https://istarik.ru/blog/stm32/121.html
// https://radiohlam.ru/stm32_15_4/#5.9
