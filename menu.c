/*
##############################################################################
##############################################################################
##
## Модуль меню
## Обеспечивает управление всей программой и запуск функциональных модулей
##
##############################################################################
##
## Небольшое тело меню крутится в loop цикле.
## Его задача - отрисовывать меню и запускать функции.
## Обработка кнопок - по прерываниям.
## Запускаемая функция переопределяет обработчики кнопок и loop цикл на свой.
## При возврате - запускает функцию инициализации меню,
## она возвращает управление кнопками и ставит свой loop цикл
##
##############################################################################
##############################################################################
*/

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "lcd.h"
#include <string.h>
#include "menu.h"
#include "screen.h"

/* Variables -----------------------------------------------------------------*/

t_keys_handlers keys_handlers; // указатели на обработчики кнопок
button_data  buttons[4];       // текущая информация по состоянию кнопок
uint16_t tick_counter = 0;
menu_item_t* CurrentMenuItem = &NULL_MENU; // Указатель на текущий пункт меню
menu_item_t* NewMenuItem = &NULL_MENU;     // Указатель на новый пункт меню
uint8_t L10 = 0;

/* Functions -----------------------------------------------------------------*/
   
   
/*----------------------------------------------------------------------------*\
функция отображения пункта меню
\*----------------------------------------------------------------------------*/
void menu_draw_item(const char* Text, const uint16_t* Icon) {
   // указатель на родительское меню - его имя нужно отобразить в заголовке окна
   char *header;
   menu_item_t* ParentItem = (*CurrentMenuItem).parent;
   if ((ParentItem != &NULL_MENU) && (ParentItem != NULL)) {
      header = (char*)(*ParentItem).text;
   } else
      header = "MENU";
   ST7735_FillScreen(SCREEN_COLOR);
   // рисуем заголовок меню
   uint8_t len = strlen(header) * 7;
   ST7735_WriteString((160-len)/2, 0, header, Font_7x10, SCREEN_TITLE_COLOR, SCREEN_COLOR, 0);
   ST7735_DrawHorisontalLine(0, 11, 160, SCREEN_TITLE_COLOR); 
   // рисуем сам текст пункта меню
   len = strlen(Text) * 11;
   ST7735_WriteString((160-len)/2, 13, Text, Font_11x18, SCREEN_TITLE_COLOR, SCREEN_COLOR, 0);
   // c 27й строки можно рисовать иконку высотой 32п
   if (Icon != NULL)
      ST7735_DrawIcon(64, 40, 32, 32, (const uint8_t*)Icon, SCREEN_TITLE_COLOR, SCREEN_COLOR, 0x00);
}

   
/*----------------------------------------------------------------------------*\
Запуск отображения меню с заданного пункта
\*----------------------------------------------------------------------------*/
void Menu_Navigate(menu_item_t* const NewMenu)
{
   if ((NewMenu == &NULL_MENU) || (NewMenu == NULL))
      return;
   NewMenuItem = NewMenu;
   CurrentMenuItem = NewMenu;
   menu_draw_item(CurrentMenuItem->text, CurrentMenuItem->icon);
}


/*----------------------------------------------------------------------------*\
обработчик кнопки вверх / предыдущий пункт меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void menu_up_button_handler(void) {
   menu_item_t* NewItem = (*CurrentMenuItem).prev;
   if ((NewItem != &NULL_MENU) && (NewItem != NULL)) {
      NewMenuItem = NewItem;
   }
}

/*----------------------------------------------------------------------------*\
обработчик кнопки вниз / следующий пункт меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void menu_down_button_handler(void) {
   menu_item_t* NewItem = (*CurrentMenuItem).next;
   if ((NewItem != &NULL_MENU) && (NewItem != NULL)) {
      NewMenuItem = NewItem;
   }
}


/*----------------------------------------------------------------------------*\
деинициализация управления перед вызовом функции выбранного пункта меню
\*----------------------------------------------------------------------------*/
void menu_deinit(void) {
   keys_handlers.pFunc_key1 = NULL; // UP
   keys_handlers.pFunc_key2 = NULL; // DOWN
   keys_handlers.pFunc_key3 = NULL; // MENU/ESC
   keys_handlers.pFunc_key4 = NULL; // PAUSE/OK
   pLoop_Func = NULL;    
}
   
/*----------------------------------------------------------------------------*\
обработчик кнопки вправо / запуск пункта меню / вход в подменю
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void menu_ok_button_handler(void) {
   menu_item_t* NewItem = (*CurrentMenuItem).child;
   if ((NewItem != &NULL_MENU) && (NewItem != NULL)) {
      // есть дочернее меню, переходим туда
      NewMenuItem = NewItem;
   } else {
      // если есть указатель на функцию пункта меню, записываем её как новый loop цикл
      void (*func)(void) = (*CurrentMenuItem).func;
      if (func != NULL) {
         menu_deinit(); // аннулируем всё управление меню // исполнитель функции это может сделать сам, но полагаться не будем
         pLoop_Func = func; // функцию инициализации обработчика пункта меню запишем в качестве loop функции
         // она отработает и настроит управление и поставит свой собственный loop цикл
         // а пока мы выйдем из прерывания
         // -----
         // при выходе из функции она должна вызвать menu_init(void)
         // управление опять перейдёт к меню
         // так вроде утечек стека не происходит
      }
   }
}


/*----------------------------------------------------------------------------*\
обработчик кнопки влево / выход в родительское меню
!!! Функция вызывается из прерывания
\*----------------------------------------------------------------------------*/
void menu_esc_button_handler(void) {
   menu_item_t* NewItem = (*CurrentMenuItem).parent;
   if ((NewItem != &NULL_MENU) && (NewItem != NULL)) {
      NewMenuItem = NewItem;
   }
}


/*----------------------------------------------------------------------------*\
Функция бесконечного цикла меню
Её задача - обнаружить изменение текущего пункта меню и отрисовать его
\*----------------------------------------------------------------------------*/
void menu_loop_code(void) {
   // если новый пункт меню не совпадает со старым, отобразим новый
   if (NewMenuItem != CurrentMenuItem) {
      CurrentMenuItem = NewMenuItem;
      Menu_Navigate(CurrentMenuItem);
   }
}


/*----------------------------------------------------------------------------*\
связывание пунктов меню
\*----------------------------------------------------------------------------*/
void menu_linking(void)
{
   //              curr   next   prev   parent     child
   MENU_ITEM_LINKS(menu1, menu2, menu4, NULL_MENU, menu1_1);
   MENU_ITEM_LINKS(menu2, menu3, menu1, NULL_MENU, menu2_1);
   MENU_ITEM_LINKS(menu3, menu4, menu2, NULL_MENU, menu3_1);
   MENU_ITEM_LINKS(menu4, menu1, menu3, NULL_MENU, menu4_1);
      
   MENU_ITEM_LINKS(menu1_1, menu1_2, menu1_8, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_2, menu1_3, menu1_1, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_3, menu1_4, menu1_2, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_4, menu1_5, menu1_3, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_5, menu1_6, menu1_4, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_6, menu1_7, menu1_5, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_7, menu1_8, menu1_6, menu1, NULL_MENU);
   MENU_ITEM_LINKS(menu1_8, menu1_1, menu1_7, menu1, NULL_MENU);
      
   MENU_ITEM_LINKS(menu2_1, menu2_2, menu2_3, menu2, NULL_MENU);
   MENU_ITEM_LINKS(menu2_2, menu2_3, menu2_1, menu2, NULL_MENU);
   MENU_ITEM_LINKS(menu2_3, menu2_1, menu2_2, menu2, NULL_MENU);
   
   MENU_ITEM_LINKS(menu3_1, menu3_2, menu3_9, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_2, menu3_3, menu3_1, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_3, menu3_4, menu3_2, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_4, menu3_5, menu3_3, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_5, menu3_6, menu3_4, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_6, menu3_7, menu3_5, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_7, menu3_8, menu3_6, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_8, menu3_9, menu3_7, menu3, NULL_MENU);
   MENU_ITEM_LINKS(menu3_9, menu3_1, menu3_8, menu3, NULL_MENU);
      
   MENU_ITEM_LINKS(menu4_1, menu4_2, menu4_3, menu4, NULL_MENU);
   MENU_ITEM_LINKS(menu4_2, menu4_3, menu4_1, menu4, NULL_MENU);
   MENU_ITEM_LINKS(menu4_3, menu4_1, menu4_2, menu4, NULL_MENU);
}


/*----------------------------------------------------------------------------*\
Восстановление управления меню
Управление сюда попадает после выхода из какого-либо модуля
\*----------------------------------------------------------------------------*/
void menu_control(void)
{
   keys_handlers.pFunc_key1 = menu_up_button_handler;    // UP
   keys_handlers.pFunc_key2 = menu_down_button_handler; // DOWN
   keys_handlers.pFunc_key3 = menu_esc_button_handler; // MENU/ESC
   keys_handlers.pFunc_key4 = menu_ok_button_handler; // PAUSE/OK/SELECT
   main_loop_time = MENU_LOOP_CYCLE;
   pLoop_Func = menu_loop_code;
   CurrentMenuItem = &NULL_MENU;
   esc_func = menu_control;
   NewMenuItem = &menu3_2;//&menu1;
}


//##########################################################
//## Инициализация управления меню
//## Управление сюда попадает из main
//## Далее мы проваливаемся в главный loop цикл
//##########################################################
void menu_init(void) {
   menu_linking();
   menu_control();
}


/*----------------------------------------------------------------------------*\
эту функцию вызовет HAL после отработки прерывания от кнопки
\*----------------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if (GPIO_Pin == BUTTON_UP_Pin)
   {
      HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
      if (HAL_GPIO_ReadPin(BUTTON_UP_GPIO_Port, BUTTON_UP_Pin) == 1)
      {
         buttons[0].pressed = 0;
      } else {
         buttons[0].pressed = 1;
         buttons[0].time_pressed = HAL_GetTick();
      }
   }
   
   if (GPIO_Pin == BUTTON_DOWN_Pin)
   {
      HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
      if (HAL_GPIO_ReadPin(BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin) == 1)
      {
         buttons[1].pressed = 0;
      } else {
         buttons[1].pressed = 1;
         buttons[1].time_pressed = HAL_GetTick();
      }
   }
   
   if (GPIO_Pin == BUTTON_LEFT_Pin)
   {
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
      if (HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin) == 1)
      {
         buttons[2].pressed = 0;
      } else {
         buttons[2].pressed = 1;
         buttons[2].time_pressed = HAL_GetTick();
      }
   }
   
   if (GPIO_Pin == BUTTON_RIGHT_Pin)
   {
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
      if (HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin) == 1)
      {
         buttons[3].pressed = 0;
      } else {
         buttons[3].pressed = 1;
         buttons[3].time_pressed = HAL_GetTick();
      }
   }
}


/*----------------------------------------------------------------------------*\
Вызов обработчика кнопки по её номеру
Модули устанавливают свои обработчики кнопок
\*----------------------------------------------------------------------------*/
void key_press_handler(uint8_t key)
{
   switch (key) {
      case 0: if (keys_handlers.pFunc_key1 != NULL) keys_handlers.pFunc_key1(); break;
      case 1: if (keys_handlers.pFunc_key2 != NULL) keys_handlers.pFunc_key2(); break;
      case 2: if (keys_handlers.pFunc_key3 != NULL) keys_handlers.pFunc_key3(); break;
      case 3: if (keys_handlers.pFunc_key4 != NULL) keys_handlers.pFunc_key4(); break;
      default: return;
   }
}


/*----------------------------------------------------------------------------*\
Тики таймера
Тут мы отрабатываем поведение кнопок
\*----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{
   uint32_t PressTime = 0;
   HAL_IncTick();
   uint32_t CurrentTick = HAL_GetTick();
   for (uint8_t i=0; i<4; i++)
   {
      PressTime = CurrentTick - buttons[i].time_pressed;
   
      if (buttons[i].pressed == 1)
      {
         if ((PressTime > Button_Short_Click_Time) &  (PressTime < Button_Long_Press_Time))
         {
            buttons[i].pressed = 2;
            // одиночное нажатие
            key_press_handler(i);
         }
      } // first pressed button
   
      if (buttons[i].pressed == 2)
      {
         if (PressTime > Button_Long_Press_Time)
         {
            buttons[i].pressed = 3;
            buttons[i].repetition_cnt = Button_Long_Press_Repetition_Rate;
            // долгое нажатие
            key_press_handler(i);
         }
      } // long pressed button
   
      if (buttons[i].pressed == 3)
      {
         if (buttons[i].repetition_cnt > 0)
         {
            buttons[i].repetition_cnt--;
         } else {
            buttons[i].repetition_cnt = Button_Long_Press_Repetition_Rate;
            // нажатие
            key_press_handler(i);
         }
      } // long pressed button repetition
   }
  
   // тестовый светодиод, чтобы понимать, что система не зависла
   tick_counter++;
   if (tick_counter >= 500) 
   {
      if (L10 != 0) {
         L10 = 0;
         HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
      } else {
         L10++;
         HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
      }
      tick_counter = 0;  
   }
}

/*----------------------------------------------------------------------------*/
