/*
CC2540EM TIMERs Test
ghostyu
20130204
*/

/*
使用定时器，完成GPIO实验中的LED显示延时
这里仅仅使用Timer3作为演示，timer124等操作均类似
*/
#include <ioCC2540.h>
#include "Common.h"
#include "hal_lcd.h"

uint32 counter = 0;
uint8  timeout = 0;
void HalTimer3Init()
{
  //T3CTL &= ~0x03;
  T3CTL = 0xAE;	//32分频，中断使能，模模式
  T3CC0 = 0xC8;	// 1Mhz==1us  200us中断
  T3CCTL0 = 0x44;//一定要设置比较的方式，不然不会中断的!!!,T3CCTL0.IM==1,T3CCTL0.MODE==1
}
/*开始计时*/
void HalTimer3Start()
{
  T3CTL |= 0x10; 
}
/*停止计时*/
void HalTimer3Stop()
{
  T3CTL &= ~0x10; 
}
/*中断使能*/
void HalTimer3EnableInt()
{
  EA = 1;
  T3IE = 1;
  //IEN1 |= BV(3);
}
/*中断不使能*/
void HalTimer3DisableInt()
{
  T3IE = 0;
  //IEN1 &= ~ BV(3);
}


/*********************************LEDs*****************************************/
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
  
  P1DIR |= BV(0)|BV(1)|BV(4);
  P0DIR |= BV(1);
  
  P1SEL &= ~(BV(0)|BV(1)|BV(4));
  P0SEL &= ~(BV(1));
}
/*********************************LEDs*****************************************/

int main()
{
  //若不对时钟初始化，将使用内部的16M晶振作为时钟，这样本来定时的1s就编程了2s了
  HAL_BOARD_INIT();
  GPIO_LedsInit();

  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->TIMER3_TEST");
  
  /*定时器初始化*/
  HalTimer3Init();
  HalTimer3EnableInt();
  HalTimer3Start();
  
  while(1){
    HAL_TURN_ON_LED1();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Green LED:ON");
    while(!timeout);//等待标志位置位
    timeout = 0;    //清零标志位，等待定时中断函数置位
    HAL_TURN_OFF_LED1();
    
    HAL_TURN_ON_LED2();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Red LED:ON");
    while(!timeout);
    timeout = 0;
    HAL_TURN_OFF_LED2();
    
    HAL_TURN_ON_LED3();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Yellow LED:ON");
    while(!timeout);
    timeout = 0;
    HAL_TURN_OFF_LED3();
    
    HAL_TURN_ON_LED4();
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Orange LED:ON");
    while(!timeout);
    timeout = 0;
    HAL_TURN_OFF_LED4();
    
  }
}
/*中断函数*/
#pragma vector = T3_VECTOR 
 __interrupt void T3_ISR(void) 
{ 
  //中断1000次为200ms
  if(counter<5000)
    counter++; 
  else{ 
    timeout = 1;  //标志位置位
    counter = 0;  //counter清零，重新开始计数
  } 
} 