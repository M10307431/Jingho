/*
CC2540EM GPIO Test
ghostyu
20130204
*/

/*
使用CC2540的GPIO口驱动底板上的LED灯，使四个led循环点亮和熄灭
*/
#include <ioCC2540.h>
#include "Common.h"
#include "hal_lcd.h"
/*
MAP
LED1--->P1_0
LED2--->P1_1
LED3--->P1_4
LED4--->P0_1
*/
/*LEDs端口定义*/
#define LED1_SBIT   P1_0
#define LED2_SBIT   P1_1
#define LED3_SBIT   P1_4
#define LED4_SBIT   P0_1
/*LEDs极性定义，高电平有效*/
#define LEDs_POLARITY   ACTIVE_HIGH

/*LEDs开关宏定义*/
#define HAL_TURN_OFF_LED1()       st( LED1_SBIT = LEDs_POLARITY (0); )
#define HAL_TURN_OFF_LED2()       st( LED2_SBIT = LEDs_POLARITY (0); )
#define HAL_TURN_OFF_LED3()       st( LED3_SBIT = LEDs_POLARITY (0); )
#define HAL_TURN_OFF_LED4()       st( LED4_SBIT = LEDs_POLARITY (0); )

#define HAL_TURN_ON_LED1()        st( LED1_SBIT = LEDs_POLARITY (1); )
#define HAL_TURN_ON_LED2()        st( LED2_SBIT = LEDs_POLARITY (1); )
#define HAL_TURN_ON_LED3()        st( LED3_SBIT = LEDs_POLARITY (1); )
#define HAL_TURN_ON_LED4()        st( LED4_SBIT = LEDs_POLARITY (1); )

void GPIO_LedsInit()
{
  /*先关所有led，防止初始化时led闪烁*/
  HAL_TURN_OFF_LED1();
  HAL_TURN_OFF_LED2();
  HAL_TURN_OFF_LED3();
  HAL_TURN_OFF_LED4();
  
  /*设置LEDs引脚为输出*/
  P1DIR |= BV(0)|BV(1)|BV(4);
  P0DIR |= BV(1);
  
  /*设置LEDs引脚为GPIO引脚*/
  P1SEL &= ~(BV(0)|BV(1)|BV(4));
  P0SEL &= ~(BV(1));
}
void delay_nus(uint16 timeout)
{
    while (timeout--)
    {
        asm("NOP");
        asm("NOP");
        asm("NOP");
    }
}

int main()
{
  HAL_BOARD_INIT();
  GPIO_LedsInit();
  HalLcd_HW_Init();
  int i;
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->GPIO_TEST");
  /*注意，软件延时，不准确，只能大概的延时一段时间*/
  while(1){
    HAL_TURN_ON_LED1();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Green LED:ON");
    //HalHW_WaitUs(200000);//延时
    HalHW_WaitMS(1000);
        //for(i=20; i>0; i--)
     // delay_nus(50000);
    HAL_TURN_OFF_LED1();
    
    HAL_TURN_ON_LED2();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Red LED:ON");
    //HalHW_WaitUs(200000);//延时
    HalHW_WaitMS(1000);
       // for(i=20; i>0; i--)
      //delay_nus(50000);
    HAL_TURN_OFF_LED2();
    
    HAL_TURN_ON_LED3();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Yellow LED:ON");
    //HalHW_WaitUs(200000);//延时
    HalHW_WaitMS(1000);
        //for(i=20; i>0; i--)
      //delay_nus(50000);
    HAL_TURN_OFF_LED3();

    HAL_TURN_ON_LED4();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Orange LED:ON");
    //HalHW_WaitUs(200000);//延时
    HalHW_WaitMS(1000);
        //for(i=20; i>0; i--)
      //delay_nus(50000);
    HAL_TURN_OFF_LED4();
    
  }
}