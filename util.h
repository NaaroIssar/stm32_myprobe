/*
##############################################################################
##############################################################################
##
## Используемые вспомогательные функции
##
##############################################################################
##############################################################################
*/

#ifndef __UTIL_H
#define __UTIL_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

#ifdef __cplusplus
}
#endif

/* Definitions ---------------------------------------------------------------*/

#define NEWLINE    1
#define NONEWLINE  0

#define ABS(x) (x>0 ? x : -x)
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* Functions -----------------------------------------------------------------*/

void logstr(char *str, uint8_t newline);
void logint(char *str, uint32_t number, uint8_t newline);
void logfloat(char *str, double number, uint8_t nfract, uint8_t newline);
double fabs(double X);

/* External functions --------------------------------------------------------*/


/* External variables --------------------------------------------------------*/

extern UART_HandleTypeDef huart1;
extern char tmp_str[30];

#endif /* __UTIL_H */
/*----------------------------------------------------------------------------*/


