/*
CC2540EM Interrupt Test
ghostyu
20130204
*/

/*
使用底板的按键S1来控制led灯的亮灭，按键S1使用中断触发信号
*/
#include<ioCC2540.h>
#include"Common.h"
#include"hal_lcd.h"
/*
MAP
S1--->P0_1
*/
#define HAL_S1_PIN  1

uint8 S1_Flag = 0;


/*按键S1端口初始化*/
void S1_INT_Init()
{
  P0SEL |= BV(HAL_S1_PIN);//外设
  P0DIR &= ~(BV(HAL_S1_PIN));//输入
  
  P0INP &= ~(BV(HAL_S1_PIN));//上拉或者下拉模式
  P2INP |= BV(5);//P0端口设置成下拉
  
  PICTL &= ~(BV(0));//上升沿中断
  P0IFG = 0;
}

/*使能S1中断*/
void S1_INT_Enable()
{
  EA = 1;//开启CPU中断
  P0IEN |= BV(HAL_S1_PIN);//开启HAL_S1_PIN引脚中断使能
  IEN1|= BV(5); //此项设置非常重要，不然进不了中断函数
}
/*失能S1中断*/
void S1_INT_Disable()
{
  P0IEN &= ~(BV(HAL_S1_PIN));
}

/*********************************LEDs*****************************************/
/*LEDs端口定义*/
#define LED1_SBIT   P1_0
#define LED2_SBIT   P1_1
#define LED3_SBIT   P1_4
/*LEDs极性定义，高电平有效*/
#define LEDs_POLARITY   ACTIVE_HIGH

/*LEDs开关宏定义*/
#define HAL_TURN_OFF_LED1()       st( LED1_SBIT = LEDs_POLARITY (0); )
#define HAL_TURN_OFF_LED2()       st( LED2_SBIT = LEDs_POLARITY (0); )
#define HAL_TURN_OFF_LED3()       st( LED3_SBIT = LEDs_POLARITY (0); )

#define HAL_TURN_ON_LED1()        st( LED1_SBIT = LEDs_POLARITY (1); )
#define HAL_TURN_ON_LED2()        st( LED2_SBIT = LEDs_POLARITY (1); )
#define HAL_TURN_ON_LED3()        st( LED3_SBIT = LEDs_POLARITY (1); )

void GPIO_LedsInit()
{
  /*先关所有led，防止初始化时led闪烁*/
  HAL_TURN_OFF_LED1();
  HAL_TURN_OFF_LED2();
  HAL_TURN_OFF_LED3();
  
  P1DIR |= BV(0)|BV(1)|BV(4);
  
  P1SEL &= ~(BV(0)|BV(1)|BV(4));
}
/*********************************LEDs*****************************************/

int main()
{
  //HAL_BOARD_INIT();
  GPIO_LedsInit();

  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->INTERRUPT_TEST");
  
  /*S1按键初始化*/
  S1_INT_Init();
  S1_INT_Enable();
  
  while(1){
    if(S1_Flag==0){
      HAL_TURN_ON_LED1();
      HAL_TURN_ON_LED2();
      HAL_TURN_ON_LED3();
      HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "ALL LED:ON");
    }else{
      HAL_TURN_OFF_LED1();
      HAL_TURN_OFF_LED2();
      HAL_TURN_OFF_LED3();
      HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "ALL LED:OFF");
    }
  }
}


/*中断函数*/
#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void) 
{ 
  if(P0IFG&BV(HAL_S1_PIN)){         //按键中断 
    P0IFG &= ~BV(HAL_S1_PIN); 
    S1_Flag = ~S1_Flag;
  } 
}