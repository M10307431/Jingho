/*
CC2540 
ghostyu
20130204
*/

/*
说明：继电器模块测试

*/
#include<ioCC2540.h>
#include"Common.h"
#include"Hal_lcd.h"

/*继电器所用的引脚定义*/
#define HAL_RELAY_PIN       4
#define HAL_RELAY_PIN_CTL   P0_4
#define HAL_RELAY_PORT_DIR      P0DIR
#define HAL_RELAY_PORT_SEL      P0SEL
/*继电器模块初始化
  初始化继电器用到的GPIO
*/
void Relay_Init()
{
  /*防止跳变*/
  HAL_RELAY_PIN_CTL = 0;
  HAL_RELAY_PORT_DIR |= BV(HAL_RELAY_PIN);
  HAL_RELAY_PORT_SEL &= ~BV(HAL_RELAY_PIN);

}


int main()
{
  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->RELAY_TEST");
  
  Relay_Init();
  while(1){
    //循环置高和置低继电器，让继电器不停的开关
    HAL_RELAY_PIN_CTL = 1;
    HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "HEIGH");
    HalHW_WaitUs(1000000);
    HAL_RELAY_PIN_CTL = 0;
    HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "LOW");
    HalHW_WaitUs(1000000);
  }
  
}