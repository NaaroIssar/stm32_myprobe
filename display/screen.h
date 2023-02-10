/*
##############################################################################
##############################################################################
##
## Определения для отображения информации на экране
##
##############################################################################
##############################################################################
*/

#ifndef __SCREEN_H
#define __SCREEN_H

/* Includes ------------------------------------------------------------------*/

#include "lcd.h"

/* Definitions ---------------------------------------------------------------*/

// Определения основного экрана

#define SCREEN_COLOR              ST7735_BLUE
#define SCREEN_BACKGROUND         ST7735_BLACK
#define SCREEN_TITLE_COLOR        ST7735_YELLOW
#define SCREEN_TITLE_FONT         Font_7x10
#define SCREEN_TITLE_HEIHG        11
#define SCREEN_WINDOW_HEIHG       (ST7735_HEIGHT-1-SCREEN_TITLE_HEIHG) // 68
#define SCREEN_WINDOW_WIDTH       (ST7735_WIDTH-1-1)                   // 158
                                 
#define SCREEN_PARAM_COLOR        ST7735_GREEN
#define SCREEN_PARAM_FONT         Font_7x10
#define SCREEN_PARAM_X            (ST7735_WIDTH - 7*5 - 2) // Параметр из 5 символов шрифтом 7x10
#define SCREEN_PARAM_Y            1


// Определения для отображения информации

#define SCREEN_TEXT_COLOR         ST7735_RED

#define SCREEN_TEXT_3D_F16_X      ((SCREEN_WINDOW_WIDTH - 3*16)/2)
#define SCREEN_TEXT_4D_F16_X      ((SCREEN_WINDOW_WIDTH - 4*16)/2)
#define SCREEN_TEXT_5D_F16_X      ((SCREEN_WINDOW_WIDTH - 5*16)/2)
#define SCREEN_TEXT_6D_F16_X      ((SCREEN_WINDOW_WIDTH - 6*16)/2)
#define SCREEN_TEXT_7D_F16_X      ((SCREEN_WINDOW_WIDTH - 7*16)/2)
#define SCREEN_TEXT_8D_F16_X      ((SCREEN_WINDOW_WIDTH - 8*16)/2)
#define SCREEN_TEXT_9D_F16_X      ((SCREEN_WINDOW_WIDTH - 9*16)/2)

// Определения для однострочного отображения


// Определения для двухстрочного отображения

#define TWOLINE_TEXT_FONT      Font_16x26
#define TWOLINE_STRING_SPACING ((SCREEN_WINDOW_HEIHG - 2*26)/5)
#define TWOLINE_TEXT_1L_Y      (SCREEN_TITLE_HEIHG + 2*TWOLINE_STRING_SPACING)
#define TWOLINE_TEXT_2L_Y      (TWOLINE_TEXT_1L_Y + TWOLINE_STRING_SPACING + 26)

/* Functions -----------------------------------------------------------------*/


/* External functions --------------------------------------------------------*/


/* External variables --------------------------------------------------------*/


#endif /* __SCREEN_H */
/*----------------------------------------------------------------------------*/
