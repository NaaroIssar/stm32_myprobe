/*##############################################################################
/*##############################################################################
/*##
/*## Общие функции для модулей
/*##
/*##############################################################################
/*##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "common_mod.h"

/* Variables -----------------------------------------------------------------*/


/* Functions -----------------------------------------------------------------*/


/*----------------------------------------------------------------------------/*
Отрисовка экрана из двух строк шрифтом 16х26
/*----------------------------------------------------------------------------*/
void two_string_draw_screen(uint32_t number1, uint32_t number2, uint8_t selector, 
                            const char *text1, const char *text2, const char *text3)
{
   ST7735_WriteNumberCentered(1, TWOLINE_TEXT_1L_Y, 158, 26, number1, 1, Font_16x26, SCREEN_TEXT_COLOR, SCREEN_BACKGROUND, 0);
   ST7735_WriteNumberCentered(1, TWOLINE_TEXT_2L_Y, 158, 26, number2, 1, Font_16x26, SCREEN_TEXT_COLOR, SCREEN_BACKGROUND, 0);

   switch (selector)
   {
      case 0: {
                 ST7735_WriteString(SCREEN_PARAM_X, SCREEN_PARAM_Y, text1, 
                 SCREEN_PARAM_FONT, SCREEN_PARAM_COLOR, SCREEN_COLOR,0);
                 break;
              }
      case 1: {
                 ST7735_WriteString(SCREEN_PARAM_X, SCREEN_PARAM_Y, text2, 
                 SCREEN_PARAM_FONT, SCREEN_PARAM_COLOR, SCREEN_COLOR,0);
                 break;
              }
      case 2: {
                 ST7735_WriteString(SCREEN_PARAM_X, SCREEN_PARAM_Y, text3, 
                 SCREEN_PARAM_FONT, SCREEN_PARAM_COLOR, SCREEN_COLOR,0);
                 break;
              }
   
   }
}

/*----------------------------------------------------------------------------/*
Button UP / Увеличение выбранного параметра
/*----------------------------------------------------------------------------*/
void plus_parameter(uint8_t selector, uint32_t parameter, uint32_t parameter_step, uint32_t parameter_max) 
{
   if (selector == 0) {
      // изменяем частоту
      if (parameter + parameter_step <= parameter_max) 
      {
         parameter += parameter_step;
      }
   } else
   if (plus_parameter == 1) {
      // изменяем шаг
      if (gen_freq_step_idx < gen_freq_step_size-1)
      {
         gen_freq_step_idx++;
         parameter_step = gen_freq_step_arr[gen_freq_step_idx];
      }
   }
}


/*----------------------------------------------------------------------------/*
Button DOWN / Уменьшение выбранного параметра
/*----------------------------------------------------------------------------*/
void minus_parameter(void) 
{
   
}


/*----------------------------------------------------------------------------/*
Button RIGHT / Выбор параметра для изменения
/*----------------------------------------------------------------------------*/
void change_parameter_focus(void)
{
   
}

/*----------------------------------------------------------------------------*/
