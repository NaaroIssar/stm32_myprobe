/*
##############################################################################
##############################################################################
##
## Функции настройки оборудования контроллера
##
##############################################################################
##############################################################################
*/

#ifndef __HARDWARE_H
#define __HARDWARE_H

/* Includes ------------------------------------------------------------------*/

#include "main.h"
//#include "menu.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"
#include "stm32f3xx_ll_rcc.h"

#ifdef __cplusplus
}
#endif

/* Definitions ---------------------------------------------------------------*/

#define LCD_MOSI_Pin         GPIO_PIN_15
#define LCD_MOSI_GPIO_Port   GPIOB
#define LCD_SCK_Pin          GPIO_PIN_9
#define LCD_SCK_GPIO_Port    GPIOF
#define LCD_DC_Pin           GPIO_PIN_11
#define LCD_DC_GPIO_Port     GPIOB
#define LCD_CS_Pin           GPIO_PIN_12
#define LCD_CS_GPIO_Port     GPIOB
#define LCD_RST_Pin          GPIO_PIN_14
#define LCD_RST_GPIO_Port    GPIOB

#define KEY1_Pin             GPIO_PIN_13
#define KEY1_GPIO_Port       GPIOC
#define KEY2_Pin             GPIO_PIN_7
#define KEY2_GPIO_Port       GPIOB
#define KEY3_Pin             GPIO_PIN_8
#define KEY3_GPIO_Port       GPIOB
#define KEY4_Pin             GPIO_PIN_15//GPIO_PIN_14 // порт С15 испортился - статикой побился?
#define KEY4_GPIO_Port       GPIOC

#define BUTTON_UP_Pin           KEY1_Pin
#define BUTTON_UP_GPIO_Port     KEY1_GPIO_Port
#define BUTTON_DOWN_Pin         KEY2_Pin
#define BUTTON_DOWN_GPIO_Port   KEY2_GPIO_Port
#define BUTTON_LEFT_Pin         KEY3_Pin
#define BUTTON_LEFT_GPIO_Port   KEY3_GPIO_Port
#define BUTTON_RIGHT_Pin        KEY4_Pin
#define BUTTON_RIGHT_GPIO_Port  KEY4_GPIO_Port

#define LD4_Pin              GPIO_PIN_8
#define LD4_GPIO_Port        GPIOE
#define LD3_Pin              GPIO_PIN_9
#define LD3_GPIO_Port        GPIOE
#define LD5_Pin              GPIO_PIN_10
#define LD5_GPIO_Port        GPIOE
#define LD7_Pin              GPIO_PIN_11
#define LD7_GPIO_Port        GPIOE
#define LD9_Pin              GPIO_PIN_12
#define LD9_GPIO_Port        GPIOE
#define LD10_Pin             GPIO_PIN_13
#define LD10_GPIO_Port       GPIOE
#define LD8_Pin              GPIO_PIN_14
#define LD8_GPIO_Port        GPIOE
#define LD6_Pin              GPIO_PIN_15
#define LD6_GPIO_Port        GPIOE

#define Button_Long_Press_Time    2000         /* milliseconds for long time */
#define Button_Short_Click_Time   100          /* milliseconds to ignore button click */
#define Button_Long_Press_Repetition_Rate 200  /* Long press repetition rate ms */

/* Functions -----------------------------------------------------------------*/

void SystemClock_Config(void);
void MX_SPI2_Init(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void DebugExplainRCC_CFGR(uint32_t CFGR);
void DebugExplainRCC_RC(uint32_t CR);
void DebugExplainRCC_CFGR2(uint32_t CFGR2);
void DebugShowRCC(uint32_t CR, uint32_t CFGR, uint32_t CFGR2);
uint8_t ChangePLL(uint8_t PreDiv, uint8_t PLLMUL);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void HAL_MspInit(void);
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle);
void DMA1_Channel3_IRQHandler(void);
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac);
void init_freqs_array(void);

/* External functions --------------------------------------------------------*/

extern void dma_buf_first_half_init(void);
extern void dma_buf_second_half_init(void);

/* External variables --------------------------------------------------------*/

extern sysclk_gen_t work_freqs[];
extern control_data_t control_data;
extern void (* dma_buf_first_half_init_pointer)();
extern void (* dma_buf_second_half_init_pointer)();

#endif /* __HARDWARE_H */
/*----------------------------------------------------------------------------*/
