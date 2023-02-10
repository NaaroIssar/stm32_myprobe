/*
##############################################################################
##############################################################################
##
## Функции управления дисплеем, функции отображения информации
##
##############################################################################
##############################################################################
*/

#ifndef __LCD_H
#define __LCD_H

/* Includes ------------------------------------------------------------------*/

#include "fonts.h"
#include "hardware.h"

/* Definitions ---------------------------------------------------------------*/

#define ST7735_SPI_PORT           SPI2

#define ST7735_RED                0xF800
#define ST7735_GREEN              0x07E0
#define ST7735_BLUE               0x001F
#define ST7735_YELLOW             0xFFE0
#define ST7735_WHITE              0xFFFF
#define ST7735_BLACK              0x0000
#define ST7735_ORANGE             0xFA20
#define ST7735_NAVY               0x000F
#define ST7735_DARKGREEN          0x03E0
#define ST7735_DARKCYAN           0x03EF
#define ST7735_MAROON             0x7800
#define ST7735_PURPLE             0x780F
#define ST7735_OLIVE              0x7BE0
#define ST7735_LIGHTGREY          0xC618
#define ST7735_DARKGREY           0x7BEF
#define ST7735_CYAN               0x07FF
#define ST7735_MAGENTA            0xF81F
#define ST7735_COLOR565(r, g, b)  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
#define ST7735_TEXT_INVERSION     1
#define ST7735_NO_TEXT_INVERSION  0

#define ST7735_RESET_PORT         LCD_RST_GPIO_Port
#define ST7735_RESET_PIN          LCD_RST_Pin

#define ST7735_CS_PORT            LCD_CS_GPIO_Port
#define ST7735_CS_PIN             LCD_CS_Pin

#define ST7735_DC_PORT            LCD_DC_GPIO_Port
#define ST7735_DC_PIN             LCD_DC_Pin

#define ST7735_X_SIZE             160
#define ST7735_Y_SIZE              80
#define ST7735_WIDTH              160
#define ST7735_HEIGHT              80

#define ST7735_NOP                0x00
#define ST7735_SWRESET            0x01
#define ST7735_RDDID              0x04
#define ST7735_RDDST              0x09

#define ST7735_SLPIN              0x10
#define ST7735_SLPOUT             0x11
#define ST7735_PTLON              0x12
#define ST7735_NORON              0x13

#define ST7735_INVOFF             0x20
#define ST7735_INVON              0x21
#define ST7735_APOFF              0x22
#define ST7735_APON               0x23
#define ST7735_DISPOFF            0x28
#define ST7735_DISPON             0x29
#define ST7735_CASET              0x2A
#define ST7735_RASET              0x2B
#define ST7735_RAMWR              0x2C
#define ST7735_RAMRD              0x2E

#define ST7735_PTLAR              0x30
#define ST7735_COLMOD             0x3A
#define ST7735_MADCTL             0x36

#define ST7735_FRMCTR1            0xB1
#define ST7735_FRMCTR2            0xB2
#define ST7735_FRMCTR3            0xB3
#define ST7735_INVCTR             0xB4
#define ST7735_DISSET5            0xB6

#define ST7735_PWCTR1             0xC0
#define ST7735_PWCTR2             0xC1
#define ST7735_PWCTR3             0xC2
#define ST7735_PWCTR4             0xC3
#define ST7735_PWCTR5             0xC4
#define ST7735_VMCTR1             0xC5

#define ST7735_RDID1              0xDA
#define ST7735_RDID2              0xDB
#define ST7735_RDID3              0xDC
#define ST7735_RDID4              0xDD

#define ST7735_PWCTR6             0xFC

#define ST7735_GMCTRP1            0xE0
#define ST7735_GMCTRN1            0xE1

#define ST7735_SPI_TIMEOUT        100

#define ST7735_MADCTL_MY          0x80
#define ST7735_MADCTL_MX          0x40
#define ST7735_MADCTL_MV          0x20
#define ST7735_MADCTL_ML          0x10
#define ST7735_MADCTL_RGB         0x00
#define ST7735_MADCTL_BGR         0x08
#define ST7735_MADCTL_MH          0x04

#define ST7735_YSTART             26
#define ST7735_XSTART             1

#define ST7735_ORIENTATION        (ST7735_MADCTL_MY|ST7735_MADCTL_MV|ST7735_MADCTL_ML|ST7735_MADCTL_BGR)

#define TEXT_INVERSE              1
#define NO_TEXT_INVERSE           0
#define SPLIT_BY_THRIADES         1
#define NO_SPLIT_BY_THRIADES      0

#define SPI_DR_8bit         *(__IO uint8_t*)&(ST7735_SPI_PORT->DR)
// F303 SPI bug - https://hubstub.ru/stm32/176-stm32f3-problemy-s-spi.html

#define ST7735_SendByte(data); while((ST7735_SPI_PORT->SR & SPI_SR_TXE) == RESET); \
                               SPI_DR_8bit = data;

/* Functions -----------------------------------------------------------------*/

void ST7735_Init();
void ST7735_FillBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_FillRectangle(uint16_t cStart, uint16_t rStart, uint16_t cStop, uint16_t rStop, uint16_t color);
void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawHorisontalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
void ST7735_DrawVerticalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
void ST7735_DrawIcon(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *icon, uint16_t color, uint16_t bgcolor, uint8_t inverse);
//void ST7735_DrawTextBorder(uint16_t x,  uint16_t y,  uint16_t w,  uint16_t h, uint16_t dx, uint16_t dy, uint16_t dw, uint16_t dh, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void ST7735_WriteStringCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void ST7735_WriteNumberCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t number, uint8_t thriades, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void ST7735_WriteFloatCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, double number, uint8_t thriades, uint8_t nfract, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse);
void DrawMainScreen(char *title);
void ClearMainScreen(void);

/* External functions --------------------------------------------------------*/


/* External variables --------------------------------------------------------*/

extern char tmp_str[30];
extern char *tmp_str_ptr;

#endif /* __LCD_H */
/*----------------------------------------------------------------------------*/
