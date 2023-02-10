/*
##############################################################################
##############################################################################
##
## Функции управления дисплеем, функции отображения информации
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "lcd.h"
#include "screen.h"
#include <stdio.h>
#include <string.h>

/* Variables -----------------------------------------------------------------*/


/* Functions -----------------------------------------------------------------*/


//void ST7735_SendByte(uint8_t data)
//{
//  while((ST7735_SPI_PORT->SR & SPI_SR_TXE) == RESET);
//  ST7735_SPI_PORT->DR = data;
//}


/*----------------------------------------------------------------------------*\
Ожидание полной отправки всех данных
\*----------------------------------------------------------------------------*/
void ST7735_WaitLastData()
{
  while((ST7735_SPI_PORT->SR & SPI_SR_TXE) == RESET);
  while((ST7735_SPI_PORT->SR & SPI_SR_BSY) != RESET);
}


/*----------------------------------------------------------------------------*\
Посылка команды в дисплей
\*----------------------------------------------------------------------------*/
void ST7735_SendCommand(uint8_t data)
{
  HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_RESET);
  ST7735_SendByte(data);
  ST7735_WaitLastData();
}


/*----------------------------------------------------------------------------*\
Посылка данных в дисплей
\*----------------------------------------------------------------------------*/
void ST7735_SendData(uint8_t data)
{
  HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);
  ST7735_SendByte(data);
  ST7735_WaitLastData();
}


/*----------------------------------------------------------------------------*\
Посылка данных в дисплей из буфера
\*----------------------------------------------------------------------------*/
void ST7735_SendDataMultiple(uint8_t *data, uint32_t num)
{
  HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);

  for (uint32_t i = 0; i < num; i++)
  {
    ST7735_SendByte(*data);
    data++;
  }

  ST7735_WaitLastData();
}


//##########################################################
//## Инициализация дисплея 
//##########################################################
void ST7735_Init()
{
   ST7735_SPI_PORT->CR1 |= SPI_CR1_SPE;
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_RESET);
   HAL_Delay(5);
   
   HAL_GPIO_WritePin(ST7735_RESET_PORT, ST7735_RESET_PIN, GPIO_PIN_RESET);
   HAL_Delay(5);
   HAL_GPIO_WritePin(ST7735_RESET_PORT, ST7735_RESET_PIN, GPIO_PIN_SET);
   HAL_Delay(5);
   
   ST7735_SendCommand(ST7735_SWRESET);
   HAL_Delay(120);

   ST7735_SendCommand(ST7735_SLPOUT);//Sleep exit
   HAL_Delay(120);

   //ST7735R Frame Rate
   ST7735_SendCommand(ST7735_FRMCTR1);
   ST7735_SendData(0x01);
   ST7735_SendData(0x2C);
   ST7735_SendData(0x2D);
   ST7735_SendCommand(ST7735_FRMCTR2);
   ST7735_SendData(0x01);
   ST7735_SendData(0x2C);
   ST7735_SendData(0x2D);
   ST7735_SendCommand(ST7735_FRMCTR3);
   ST7735_SendData(0x01);
   ST7735_SendData(0x2C);
   ST7735_SendData(0x2D);
   ST7735_SendData(0x01);
   ST7735_SendData(0x2C);
   ST7735_SendData(0x2D);
   
   ST7735_SendCommand(ST7735_INVCTR); //Column inversion
   ST7735_SendData(0x07);
   
   //ST7735R Power Sequence
   ST7735_SendCommand(ST7735_PWCTR1);
   ST7735_SendData(0xA2);
   ST7735_SendData(0x02);
   ST7735_SendData(0x84);
   ST7735_SendCommand(ST7735_PWCTR2);
   ST7735_SendData(0xC5);
   ST7735_SendCommand(ST7735_PWCTR3);
   ST7735_SendData(0x0A);
   ST7735_SendData(0x00);
   ST7735_SendCommand(ST7735_PWCTR4);
   ST7735_SendData(0x8A);
   ST7735_SendData(0x2A);
   ST7735_SendCommand(ST7735_PWCTR5);
   ST7735_SendData(0x8A);
   ST7735_SendData(0xEE);
   
   ST7735_SendCommand(ST7735_VMCTR1); //VCOM
   ST7735_SendData(0x0E);
   
   //ST7735R Gamma Sequence
   ST7735_SendCommand(ST7735_GMCTRP1);
   ST7735_SendData(0x0f);
   ST7735_SendData(0x1a);
   ST7735_SendData(0x0f);
   ST7735_SendData(0x18);
   ST7735_SendData(0x2f);
   ST7735_SendData(0x28);
   ST7735_SendData(0x20);
   ST7735_SendData(0x22);
   ST7735_SendData(0x1f);
   ST7735_SendData(0x1b);
   ST7735_SendData(0x23);
   ST7735_SendData(0x37);
   ST7735_SendData(0x00);
   ST7735_SendData(0x07);
   ST7735_SendData(0x02);
   ST7735_SendData(0x10);
   
   ST7735_SendCommand(ST7735_GMCTRN1);
   ST7735_SendData(0x0f);
   ST7735_SendData(0x1b);
   ST7735_SendData(0x0f);
   ST7735_SendData(0x17);
   ST7735_SendData(0x33);
   ST7735_SendData(0x2c);
   ST7735_SendData(0x29);
   ST7735_SendData(0x2e);
   ST7735_SendData(0x30);
   ST7735_SendData(0x30);
   ST7735_SendData(0x39);
   ST7735_SendData(0x3f);
   ST7735_SendData(0x00);
   ST7735_SendData(0x07);
   ST7735_SendData(0x03);
   ST7735_SendData(0x10);

	ST7735_SendCommand(ST7735_INVON);
	ST7735_SendCommand(ST7735_MADCTL); //MX, MY, RGB mode
	ST7735_SendData(ST7735_ORIENTATION);
	ST7735_SendCommand(ST7735_COLMOD); //65k mode
	ST7735_SendData(0x05);
	ST7735_SendCommand(ST7735_DISPON);//Display on

   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_SET);
}


/*----------------------------------------------------------------------------*/
//void ST7735_SetColAddr(uint16_t cStart, uint16_t cStop)
//{
//  uint8_t data[4];
//
//  data[0] = ((cStart+ST7735_XSTART) & 0xFF00) >> 8;
//  data[1] = (cStart+ST7735_XSTART) & 0x00FF;
//  data[2] = ((cStop+ST7735_XSTART) & 0xFF00) >> 8;
//  data[3] = (cStop+ST7735_XSTART) & 0x00FF;
//
//  ST7735_SendCommand(ST7735_CASET);
//  ST7735_SendDataMultiple(data, 4);
//}


/*----------------------------------------------------------------------------*/
//void ST7735_SetRowAddr(uint16_t rStart, uint16_t rStop)
//{
//  uint8_t data[4];
//
//  data[0] = ((rStart+ST7735_YSTART) & 0xFF00) >> 8;
//  data[1] = (rStart+ST7735_YSTART) & 0x00FF;
//  data[2] = ((rStop+ST7735_YSTART) & 0xFF00) >> 8;
//  data[3] = (rStop+ST7735_YSTART) & 0x00FF;
//
//  ST7735_SendCommand(ST7735_RASET);
//  ST7735_SendDataMultiple(data, 4);
//}


/*----------------------------------------------------------------------------*\
Установка адресов рабочего окна дисплея
\*----------------------------------------------------------------------------*/
void ST7735_SetAddressWindow(uint16_t cStart, uint16_t rStart, uint16_t cStop, uint16_t rStop)
{
  uint8_t data[4];

  data[0] = ((cStart+ST7735_XSTART) & 0xFF00) >> 8;
  data[1] = (cStart+ST7735_XSTART) & 0x00FF;
  data[2] = ((cStop+ST7735_XSTART) & 0xFF00) >> 8;
  data[3] = (cStop+ST7735_XSTART) & 0x00FF;

  ST7735_SendCommand(ST7735_CASET);
  ST7735_SendDataMultiple(data, 4);

  data[0] = ((rStart+ST7735_YSTART) & 0xFF00) >> 8;
  data[1] = (rStart+ST7735_YSTART) & 0x00FF;
  data[2] = ((rStop+ST7735_YSTART) & 0xFF00) >> 8;
  data[3] = (rStop+ST7735_YSTART) & 0x00FF;

  ST7735_SendCommand(ST7735_RASET);
  ST7735_SendDataMultiple(data, 4);
}


/*----------------------------------------------------------------------------*\
Функция вывода симвода на экран
Внутренняя функция
\*----------------------------------------------------------------------------*/
void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse) {
    uint32_t i, b, j;

    ST7735_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
    ST7735_SendCommand(ST7735_RAMWR);
    uint8_t fdata[] = { color >> 8, color & 0xFF };
    uint8_t bdata[] = { bgcolor >> 8, bgcolor & 0xFF };
    HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);
    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - font.offset) * font.height + i];
        if (inverse>0) b = b ^ 0xFFFF;
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                ST7735_SendByte(fdata[0]);
                ST7735_SendByte(fdata[1]);
            } else {
                ST7735_SendByte(bdata[0]);
                ST7735_SendByte(bdata[1]);
            }
        }
    }
}


/*----------------------------------------------------------------------------*\
Вывод строки на экран
\*----------------------------------------------------------------------------*/
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, 
                        uint16_t bgcolor, uint8_t inverse) {
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_RESET);
   while(*str) {
      if (*str == ' ')
      { 
         // ширина пробела будет вполовину меньше ширины символа
         ST7735_FillBlock(x, y, font.width/2, font.height, (inverse!=0) ? color : bgcolor);
         x += font.width/2;
      } else {
         ST7735_WriteChar(x, y, *str, font, color, bgcolor, inverse);
         x += font.width;
      }
      str++;
   }
   ST7735_WaitLastData();
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_SET);
}


/*----------------------------------------------------------------------------*\
Функция заполнения прямоугольника цветом
Внутренняя функция
\*----------------------------------------------------------------------------*/
void ST7735_FillBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
   ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);
   ST7735_SendCommand(ST7735_RAMWR);

   uint8_t colorBytes[2];
   colorBytes[0] = (color & 0xFF00) >> 8;
   colorBytes[1] = color & 0x00FF;

   HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);
   for (uint32_t i = 0; i < w * h; i++)
   {
      ST7735_SendByte(colorBytes[0]);
      ST7735_SendByte(colorBytes[1]);
   }
}


/*----------------------------------------------------------------------------*\
Заполнение прямоугольника цветом
\*----------------------------------------------------------------------------*/
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_RESET);
   ST7735_FillBlock(x, y, w, h, color);
   ST7735_WaitLastData();
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_SET);
}


/*----------------------------------------------------------------------------*\
Заполнение экрана цветом
\*----------------------------------------------------------------------------*/
void ST7735_FillScreen(uint16_t color) {
   ST7735_FillRectangle(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}


/*----------------------------------------------------------------------------*\
Нарисовать горизонтальную линию
\*----------------------------------------------------------------------------*/
void ST7735_DrawHorisontalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
   ST7735_FillRectangle(x, y, len, 1, color);
}


/*----------------------------------------------------------------------------*\
Нарисовать вертикальную линию
\*----------------------------------------------------------------------------*/
void ST7735_DrawVerticalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
   ST7735_FillRectangle(x, y, 1, len, color);
}


/*----------------------------------------------------------------------------*\
Вывести иконку 32х32 на экран
\*----------------------------------------------------------------------------*/
void ST7735_DrawIcon(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *icon, 
                     uint16_t color, uint16_t bgcolor, uint8_t inverse) 
{
   uint16_t i, b, j;
   if((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;
   if((x + w - 1) >= ST7735_WIDTH) return;
   if((y + h - 1) >= ST7735_HEIGHT) return;

   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_RESET);
   ST7735_SetAddressWindow(x, y, x+w-1, y+h-1);
   ST7735_SendCommand(ST7735_RAMWR);
   HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);
   uint8_t fdata[] = { color >> 8, color & 0xFF };
   uint8_t bdata[] = { bgcolor >> 8, bgcolor & 0xFF };
   for(i = 0; i < w*h/8; i++) {
      b = icon[i];
      if (inverse>0) b = b ^ 0xFF;
      for(j = 0; j < 8; j++) {
         if((b << j) & 0x80)  {
            ST7735_SendByte(fdata[0]);
            ST7735_SendByte(fdata[1]);
         } else {
            ST7735_SendByte(bdata[0]);
            ST7735_SendByte(bdata[1]);
         }
      }
   }
   ST7735_WaitLastData();
   HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_SET);
}


/*----------------------------------------------------------------------------*\
Нарисовать рамку, обрамляющую текст
Внутренняя функция
\*----------------------------------------------------------------------------*/
void ST7735_DrawTextBorder(uint16_t x,  uint16_t y,  uint16_t w,  uint16_t h,
                           uint16_t dx, uint16_t dy, uint16_t dw, uint16_t dh,
                           uint16_t color, uint16_t bgcolor, uint8_t inverse)
{
   if (dx-x>0) ST7735_FillRectangle(x, y, dx, h, (inverse == 0) ? color : bgcolor);
   if (w-dw-dx+1>0) ST7735_FillRectangle(dx+dw, y, w-dw-dx+1, h, (inverse == 0) ? color : bgcolor);
   if (dy-y>0) ST7735_FillRectangle(dx, y, dw, dy-y, (inverse == 0) ? color : bgcolor);
   if (h-dy-dh+1>0) ST7735_FillRectangle(dx, dy+dh, dw, h-dy-dh+1, (inverse == 0) ? color : bgcolor);
}


/*----------------------------------------------------------------------------*\
Пишет строку центрировано в прямоугольнике
x, y, w, h - координаты, ширина и высота прямоугольника
str, font - строка и шрифт
color, bgcolor - цвет шрифта и фона
inverse - инвертировать цвета
\*----------------------------------------------------------------------------*/
void ST7735_WriteStringCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str, 
                                FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse)
{
   if (font.height > h)  h = font.height;
   uint16_t dy = y + (h - font.height) / 2;
   uint16_t sw = 0;
   for (uint8_t i=0; i<strlen(str); i++)
   {
      if (str[i] == ' ')
         sw += font.width>>1;
      else
         sw += font.width;
   }
   uint16_t dx = x + (w - sw) / 2;
   ST7735_DrawTextBorder(x, y, w, h, dx, dy, sw, font.height, bgcolor, color, inverse);
   ST7735_WriteString(dx, dy, str, font, color, bgcolor, inverse);
}


/*----------------------------------------------------------------------------*\
Преобразует число в строку с разбиением на триады
\*----------------------------------------------------------------------------*/
void number_by_thriades(uint32_t number)
{
   char format[6];
   uint16_t digits3, triads = 0; uint32_t power = 1;
   tmp_str_ptr = tmp_str;
   tmp_str[0] = '\0';
   while (power <= number) {triads++; power *= 1000;}
   if (triads == 0) {(*tmp_str_ptr++) = '0';   *tmp_str_ptr = '\0';}
   for (uint16_t triada = 1; triada <= triads; triada++)
   {
      power = 1;      for (uint8_t i=0; i<3*(triads - triada); i++) power = power * 10;
      digits3 = number / power;
      number = number - digits3 * power;
      if (triada > 1) sprintf(format, " %03d", digits3);
      else sprintf(format, "%d", digits3);
      strcat(tmp_str, format);
   }
}


/*----------------------------------------------------------------------------*\
Пишет число центрировано в прямоугольнике
x, y, w, h - координаты, ширина и высота прямоугольника
number - число
thriades - разбивать число на тройки (0/1)
font - шрифт
color, bgcolor - цвет шрифта и фона
inverse - инвертировать цвета (0/1)
\*----------------------------------------------------------------------------*/
void ST7735_WriteNumberCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t number, uint8_t thriades, 
                                FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse)
{
   if (thriades == SPLIT_BY_THRIADES)
      number_by_thriades(number);
   else
      sprintf(tmp_str, "%ld", number);
   ST7735_WriteStringCentered(x, y, w, h, tmp_str, font, color, bgcolor, inverse);
}


/*----------------------------------------------------------------------------*\
Пишет число центрировано в прямоугольнике
x, y, w, h - координаты, ширина и высота прямоугольника
number - число
nfract - количество знаков дробной части числа
thriades - разбивать число на тройки
font - шрифт
color, bgcolor - цвет шрифта и фона
inverse - инвертировать цвета
\*----------------------------------------------------------------------------*/
void ST7735_WriteFloatCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, double number, uint8_t thriades, 
                               uint8_t nfract, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t inverse)
{
   char format[10];
   uint32_t power, n;
   if (thriades == SPLIT_BY_THRIADES)
      number_by_thriades((uint32_t)number);
   else
      sprintf(tmp_str, "%ld", (uint32_t)number);
   power = 1;
   for (uint8_t i = 0; i < nfract; i++) power = power * 10;
   n = (uint32_t)((number - (uint32_t)number) * power + 0.5);
   sprintf(format, ".%ld", n);
   strcat(tmp_str, format);
   ST7735_WriteStringCentered(x, y, w, h, tmp_str, font, color, bgcolor, inverse);
}


/*----------------------------------------------------------------------------*\
Очистка рабочего поля экрана
\*----------------------------------------------------------------------------*/
void ClearMainScreen(void)
{
   ST7735_FillRectangle(1, SCREEN_TITLE_HEIHG, SCREEN_WINDOW_WIDTH, SCREEN_WINDOW_HEIHG, SCREEN_BACKGROUND);
}


/*----------------------------------------------------------------------------*\
рисует основу экрана
\*----------------------------------------------------------------------------*/
void DrawMainScreen(char *title)
{
   ClearMainScreen();
   ST7735_FillRectangle(0, 0, ST7735_WIDTH, SCREEN_TITLE_HEIHG, SCREEN_COLOR);
   ST7735_DrawHorisontalLine(1, ST7735_HEIGHT-1, ST7735_WIDTH-2, SCREEN_COLOR);
   ST7735_DrawVerticalLine(0, SCREEN_TITLE_HEIHG, ST7735_HEIGHT-SCREEN_TITLE_HEIHG, SCREEN_COLOR);
   ST7735_DrawVerticalLine(ST7735_WIDTH-1, SCREEN_TITLE_HEIHG, ST7735_HEIGHT-SCREEN_TITLE_HEIHG, SCREEN_COLOR);
   ST7735_WriteString(1, 1, title, SCREEN_TITLE_FONT, SCREEN_TITLE_COLOR, SCREEN_COLOR, ST7735_NO_TEXT_INVERSION);
}

/*----------------------------------------------------------------------------*/
