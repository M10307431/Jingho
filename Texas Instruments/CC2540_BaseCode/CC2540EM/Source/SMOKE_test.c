/*
CC2540 
ghostyu
20130204
*/

/*
说明：烟雾报警器模块测试程序
烟雾报警器，检测到烟雾时，GPIO输出高电平，报警消失后，变低电平
*/
#include<ioCC2540.h>
#include"Common.h"
#include"Hal_lcd.h"

//烟雾报警器模块所用到的GPIO
#define HAL_RELAY_PIN       4
#define HAL_RELAY_PIN_CTL   P0_4

/*烟雾报警器模块初始化
  初始化烟雾报警器模块用到的GPIO
*/
void Smoke_Init()
{
  P0DIR &= ~BV(HAL_RELAY_PIN);  //输入，外设
  P0SEL &= ~BV(HAL_RELAY_PIN);

}

int main()
{
  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->SMOKE_TEST");
  
  Smoke_Init();
  while(1){
    //当检测到烟雾时，模块输出高电平，开始报警
    if(HAL_RELAY_PIN_CTL==1){
      HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "HEIGH");
    }else{
      HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "LOW");
    }
    //HalHW_WaitUs(500000);
  }
  
}