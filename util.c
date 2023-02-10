/*
##############################################################################
##############################################################################
##
## Используемые вспомогательные функции
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "util.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Variables -----------------------------------------------------------------*/

char format[50];

/* Functions -----------------------------------------------------------------*/


/*----------------------------------------------------------------------------*\
функция ABS для чисел с плавающей точкой
\*----------------------------------------------------------------------------*/
double fabs(double X)
{
   if (X >= 0) return X;
   else return -X;
}


/*----------------------------------------------------------------------------*\
отправка строки в лог (UART1)
\*----------------------------------------------------------------------------*/
void logstr(char *str, uint8_t newline)
{
   strcpy(format, str);
   if (newline == NEWLINE)  strncat(format, "\r\n", 2);
   else strncat(format, " ", 2);
   HAL_UART_Transmit(&huart1, (uint8_t*)format, strlen(format), 1000);
}


/*----------------------------------------------------------------------------*\
отправка целого числа в лог (UART1)
Можно задать свой формат вывода (%ld). Если он не задан, будет использован "%ld"
\*----------------------------------------------------------------------------*/
void logint(char *str, uint32_t number, uint8_t newline)
{
   strcpy(format, str);
   if (strchr(str, '%') == NULL) strncat(format, " %ld", 4);
   if (newline == NEWLINE)  strncat(format, "\r\n", 2);
   else strncat(format, " ", 2);
   sprintf(tmp_str, format, number); 
   HAL_UART_Transmit(&huart1, (uint8_t*)tmp_str, strlen(tmp_str), 1000);
}


/*----------------------------------------------------------------------------*\
отправка числа с плавающей точкой в лог (UART1)
Задание собственного формата вывода не предусмотрено.
Необходимо указать количество разрядов после точки.
\*----------------------------------------------------------------------------*/
void logfloat(char *str, double number, uint8_t nfract, uint8_t newline)
{
   uint32_t dinteger, dfraction, power;
   strcpy(format, str);
   strncat(format, " %ld", 4);
   if (nfract > 0) strncat(format, ".%ld", 4);
   printf("'%s' \n", format);
   if (newline == NEWLINE)  strncat(format, "\r\n", 2);
   else strncat(format, " ", 2);
   dinteger = (uint32_t)number;
   power = 1;
   for (uint8_t i=0; i<nfract; i++) power = power * 10;
   dfraction = (uint32_t)round((number - dinteger) * power);
   if (nfract == 0) 
      sprintf(tmp_str, format, dinteger); 
   else
      sprintf(tmp_str, format, dinteger, dfraction); 
   HAL_UART_Transmit(&huart1, (uint8_t*)tmp_str, strlen(tmp_str), 1000);
}

/*----------------------------------------------------------------------------*/
