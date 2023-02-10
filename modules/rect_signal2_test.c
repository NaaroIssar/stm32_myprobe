#include <stdio.h>
#include <math.h>
#include <string.h>

#define uint32_t long
#define uint16_t int
#define Fosc                     16002899
#define abs(x)  ((x)>=0 ? (x) : (-x))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define RECT_SIGNAL2_TWO_FREQ_GENERATION      0   // разрешена ли двухчастотная генерация сигнала
#define RECT_SIGNAL2_MAX_ALLOWED_DIFFERENCE   0.1 // максимально допустимое отклонение от заданной частоты для одночастотной генерации сигнала
#define RECT_SIGNAL2_BUFFER_SIZE 2000 // размер буфера DMA для двухчастотной генерации

typedef struct sysclk_gen
{
   uint16_t prediv;
   uint16_t pllmul;
   double   sysclk;
} sysclk_gen_t;

sysclk_gen_t work_freqs[] = {{8, 2, 0.0}, {7, 2, 0.0}, {6, 2, 0.0}, {8, 3, 0.0}, {5, 2, 0.0}, {7, 3, 0.0},
{4, 2, 0.0}, {7, 4, 0.0}, {5, 3, 0.0}, {8, 5, 0.0}, {3, 2, 0.0}, {7, 5, 0.0}, {4, 3, 0.0}, {5, 4, 0.0},
{6, 5, 0.0}, {7, 6, 0.0}, {8, 7, 0.0}, {2, 2, 0.0}, {8, 9, 0.0}, {7, 8, 0.0}, {6, 7, 0.0}, {5, 6, 0.0},
{4, 5, 0.0}, {7, 9, 0.0}, {3, 4, 0.0}, {8, 11, 0.0}, {5, 7, 0.0}, {7, 10, 0.0}, {2, 3, 0.0}, {7, 11, 0.0},
{5, 8, 0.0}, {8, 13, 0.0}, {3, 5, 0.0}, {7, 12, 0.0}, {4, 7, 0.0}, {5, 9, 0.0}, {6, 11, 0.0}, {7, 13, 0.0},
{8, 15, 0.0}, {1, 2, 0.0}, {7, 15, 0.0}, {6, 13, 0.0}, {5, 11, 0.0}, {4, 9, 0.0}, {7, 16, 0.0}, {3, 7, 0.0},
{5, 12, 0.0}, {2, 5, 0.0}, {5, 13, 0.0}, {3, 8, 0.0}, {4, 11, 0.0}, {5, 14, 0.0}, {1, 3, 0.0}, {5, 16, 0.0},
{4, 13, 0.0}, {3, 10, 0.0}, {2, 7, 0.0}, {3, 11, 0.0}, {4, 15, 0.0}, {1, 4, 0.0}, {3, 13, 0.0}, {2, 9, 0.0},
{3, 14, 0.0}, {1, 5, 0.0}, {3, 16, 0.0}, {2, 11, 0.0}, {1, 6, 0.0}, {2, 13, 0.0}, {1, 7, 0.0},
{2, 15, 0.0}, {1, 8, 0.0}, {1, 9, 0.0} };
#define WORK_FREQS_LEN  72 // размер массива системных частот

void init_freqs_array(void)
{
   for (uint16_t i=0; i<WORK_FREQS_LEN; i++)
      work_freqs[i].sysclk = (double)Fosc * work_freqs[i].pllmul / work_freqs[i].prediv;
}

uint32_t freq = 14964;//793658;//333658;//788290;//395743;

double bestpwm, bestsysclk;
uint16_t bestpllmul, bestprediv;
uint16_t minK16;
uint16_t minK16_2;
uint32_t K3;
uint16_t K3psc, K3cnt;
double rpwm, rpwm2; // реальные получившиеся PWM
double rfreq, rfreq2; // реальные получающиеся частоты
double difffreq;
uint16_t pwm1, pwm2, bestpwm1, bestpwm2;
double   fpwm2, diff, bestdiff=9;
double rfreq3;
double adjitter; // амплитуда джиттера, нс
double fdjitter; // частота джиттера, Гц
double deviation; // девиация частоты, Гц

// подбор лучшей рабочей системной частоты
// лучшая частота та, при которой необходимое значение ШИМ максимально
// но значение ШИМ не должно превышать 0,99998 - иначе мы не сможем его
// реализовать на 16-битном таймере. Максимально возможное значение
// ШИМ = (65535-1)/65535 = 0,9999847409781033
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
      if ((sysclk >= 8000000) &&(sysclk <= 36010000.0))
      {
         K3 = (uint32_t)((sysclk / 2) / (double)freq / 2);
         K3psc = K3 / 65536 + 1;
         K3cnt = K3 % 65536;
         K3 = K3psc * K3cnt;
         pwm = (double)freq * (K3) / (sysclk / 2) * 2;
         if (pwm<0.9999847)
         {
            //printf("pwm=%8.6f sysclk=%10.1f prediv=%d pllmul=%d\n", pwm, sysclk, prediv, pllmul);
            if (((bestpwm < pwm) && (pwm < 0.9999847)) || ((bestpwm == pwm) && (bestsysclk < sysclk)))
            {
               //printf("pwm=%8.6f sysclk=%10.1f prediv=%d pllmul=%d\n", pwm, sysclk, prediv, pllmul);
               bestpwm = pwm;
               bestsysclk = sysclk;
               bestpllmul = work_freqs[i].pllmul;
               bestprediv = work_freqs[i].prediv;
            } // if (((bestpwm  
         } // if (pwm<0.99998)
      } // if (sysclk <=
   } // for (uint16_t i=0;
}

void rect_signal2_calculate_pwm(double pwm, uint16_t djitter)
{
   double mindiff = 999.0;
   double diff, realpwm;
   minK16 = 1;
   for (uint16_t K16=10; K16<65535; K16++)
   {
      realpwm = ((double)K16 - (double)djitter) / (double)K16;
      if (pwm > realpwm) diff = pwm - realpwm; else diff = realpwm - pwm;
      if (mindiff > diff)
      {
         //printf("K16=%d, realpwm=%8.6f diff=%10.8f\n", K16, realpwm, diff);
         mindiff = diff;
         minK16 = K16;
      }
   }

}

// рассчитать количество и параметры двух PWM для двухчастотной генерации
void rect_signal2_calculate_pwm2(double bestpwm, double rpwm, double rpwm2)
{
   bestdiff = 9;
   //double k = (bestpwm - MIN(rpwm,rpwm2)) / (MAX(rpwm,rpwm2) - bestpwm);
   double k = (bestpwm - rpwm) / (rpwm2 - bestpwm);
   for (pwm1 = 10; pwm1 < RECT_SIGNAL2_BUFFER_SIZE; pwm1++)
   {
      fpwm2 = k * pwm1;
      pwm2 = (uint16_t)fpwm2;
      diff = fpwm2 - pwm2;
      if ((bestdiff > diff) && ((pwm1 + pwm2) <= RECT_SIGNAL2_BUFFER_SIZE))
      {
         bestdiff = diff;
         bestpwm1 = pwm1;
         bestpwm2 = pwm2;
      }
   }
}

void main(void)
{
   // заполним таблицу рабочих частот
   init_freqs_array();
   // рассчитаем наиболее подходящую системную частоту
   rect_signal2_calculate_best_sysclk(freq);
   // рассчитаем делитель выходного таймера
   K3 = bestsysclk / 2 / freq / 2;
   K3psc = ((uint32_t)K3) / 65536 + 1;
   K3cnt = ((uint32_t)K3) % 65536;
   K3 = K3psc * K3cnt;
   // рассчитаем делитель для таймера-модулятора
   rect_signal2_calculate_pwm(bestpwm, 1);
   // рассчитаем реально получившиеся параметры сигнала
   rpwm = ((double)minK16-1.0)/(double)minK16;
   rfreq = ((bestsysclk / 2) * rpwm / (double)K3 / 2);
   difffreq = rfreq - (double)freq;
   adjitter = 2000000000 / bestsysclk;
   fdjitter = bestsysclk / 2 / minK16;
   // Если отклонение частоты от заданной слишком велико, 
   // рассчитаем параметры двухчастотной генерации
   if ((abs(difffreq) > RECT_SIGNAL2_MAX_ALLOWED_DIFFERENCE) && (RECT_SIGNAL2_TWO_FREQ_GENERATION != 0))
   {
      // отклонение частоты слишком большое, применим двухчастотную генерацию
      if (bestpwm > rpwm)
      {
         // получившийся PWM меньше нужного
         minK16_2 = minK16 + 1;
      } else {
         // получившийся PWM больше нужного
         minK16_2 = minK16 - 1;
      }
      // параметры второй частоты
      rpwm2 = ((double)minK16_2-1.0)/(double)minK16_2;
      rfreq2 = ((bestsysclk / 2) * rpwm2 / (double)K3 / 2);
      difffreq = rfreq2 - (double)freq;
      // найдём два числа для двухчастотной генерации
      rect_signal2_calculate_pwm2(bestpwm, rpwm, rpwm2);
      // рассчитаем реально получившиеся параметры сигнала
      rfreq3 = rfreq * bestpwm1 / (bestpwm1+bestpwm2) + rfreq2 * bestpwm2 / (bestpwm1+bestpwm2);
      difffreq = rfreq3-freq;
      deviation = MAX(abs(rfreq - freq), abs(rfreq2 - freq));
      // параметры - bestprediv, bestpllmul, K3, buffer_size, minK16, bestpwm1, minK16_2, bestpwm2 
      // частота фактическая rfreq3, отклонение difffreq, девиация deviation, амплитуда джиттера adjitter, частота джиттера fdjitter
      printf("\nTwo freq generation\n");
      printf("sysclk=%10.1f prediv=%d pllmul=%d\n", bestsysclk, bestprediv, bestpllmul);
      printf("K3=%ld : PSC=%d, CNT=%d\n", K3, K3psc, K3cnt);
      printf("1 target PWM=%8.6f K16=%d PWM=%8.6f\n", bestpwm, minK16, rpwm);
      printf("2 target freq=%ld, real freq=%10.1f, diff=%8.6f\n", freq, rfreq, rfreq - freq);
      printf("2 target PWM=%8.6f K16=%d PWM=%8.6f\n", bestpwm, minK16_2, rpwm2);
      printf("2 target freq=%ld, real freq=%10.1f, diff=%8.6f\n", freq, rfreq2, rfreq2 - freq);
      printf("pwm1 %d  pwm2 %d\n", bestpwm1, bestpwm2);
      printf("freq %ld, awg freq %10.1f, diff %8.6f\n", freq, rfreq3, difffreq);
      printf("девиация %8.1f Hz, амплитуда джиттера %5.1f ns, частота джиттера %8.1f Hz\n", deviation, adjitter, fdjitter);
      printf("abs %f %f\n", abs(rfreq - freq), abs(rfreq2 - freq));
   } else {
      // отклонение частоты не превышает установленный максимум, одночастотная генерация
      // или двухчастотная генерация запрещена
      // параметры - bestprediv, bestpllmul, minK16, K3
      // частота фактическая rfreq, отклонение difffreq, амплитуда джиттера adjitter, частота джиттера fdjitter
      printf("\nSingle freq generation\n");
      printf("sysclk=%10.1f prediv=%d pllmul=%d\n", bestsysclk, bestprediv, bestpllmul);
      printf("K3=%ld : PSC=%d, CNT=%d\n", K3, K3psc, K3cnt);
      printf("target PWM=%8.6f K16=%d PWM=%8.6f\n", bestpwm, minK16, rpwm);
      printf("target freq=%ld, real freq=%10.1f, diff=%8.6f\n", freq, rfreq, difffreq);
      printf("амплитуда джиттера %5.1f ns, частота джиттера %8.1f Hz\n", adjitter, fdjitter);
   }
   printf("\n");
   




}
