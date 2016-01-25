/*
CC2540 
ghostyu
20130204
*/

/*
说明：
五向按键实验，五向按键使用adc采样，
当5个按键中的一个按下时会在P2_0引脚产生一个中断，
然后读取ADC通道6的电压值，
通过不同的电压值来区分是哪个按键
*/
#include<ioCC2540.h>
#include"Common.h"
#include"Hal_lcd.h"

void JOYSTICK_ADC_Init();

/*五向按键仅使用两个GPIO引脚
  P2_0 中断
  P0_6 ad转换
*/
/* Joy stick move at P2.0 */
#define HAL_KEY_JOY_MOVE_PORT   P2
#define HAL_KEY_JOY_MOVE_PIN    0

//5向按键 采样通道为ad-6
#define HAL_KEY_JOY_CHN   6


uint16 JOYSTICK_Value = 0;
uint8 flag=0;

/*五向按键GPIO初始化， 
  仅P2_0
*/
void JOYSTICK_Init()
{
  P2SEL |= BV(HAL_KEY_JOY_MOVE_PIN);  //P2_0 设为普通GPIO
  P2DIR &= ~BV(HAL_KEY_JOY_MOVE_PIN); //P2_0 设为输入
  
  PICTL &= ~(BV(3));//设置PORT2 上升沿中断
  P2IFG = 0;  //清除Port2端口中断标志寄存器
}
/*使能五向按键的GPIO中断*/
void JOYSTICK_Enable()
{
  EA = 1;
  P2IEN |= BV(HAL_KEY_JOY_MOVE_PIN);
  IEN2 |= BV(1); //设置PORT2 中断使能
}
/*五向按键用到的ADC部分初始化*/
void JOYSTICK_ADC_Init()
{
  ADCCON3 |= 0x80;//设置参考电压，这里为选择AVDD5
  ADCCON3 |= 0x00;//设置分辨率为8位
  ADCCON1 |= 0x30;//设置ADC启动方式，这里为软件启动，当st为1时 开始ADC转换
}
//adc采用通道选择，从0~7，分别对应AIN0 ~ AIN7
//采样通道 每次采样前均需重新设置！
void JOYSTICK_ADC_SelectChanel(uint8 ch)
{
    ADCCON3 |= ch;//select ainx as input
    //APCFG |= BV(ch); //不是必须的配置，又名ADCCFG
}
/*ADC启动和停止函数
  s为true时开始采样
  否则停止采样
*/
void JOYSTICK_ADC_Start(bool s)
{
  if(s){
    ADCCON1 |= BV(6);
  }else{
    ADCCON1 &= ~BV(6);
  }
}
/*ADC是否采样结束
  结束后返回非0数
*/
uint8 JOYSTICK_ADC_Busy()
{
  return (ADCCON1&0x80);
}
/*读取ADC采样值*/
uint16 JOYSTICK_ADC_Get()
{
  uint16 t = ((uint16)(ADCH)<<8)+ADCL;
  t>>=8;  //8为分辨率，得到的值右移8位，即除以256
  return t;
}

int main()
{
  //HAL_BOARD_INIT();
  HalLcd_HW_Init();
  
  JOYSTICK_Init();
  JOYSTICK_Enable();
  
  JOYSTICK_ADC_Init();
  JOYSTICK_ADC_Get();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->JOYSTICK_TEST"); 
  
  while(1){
    if(flag ==1){
      /*不同的采样电压对应不同的按键*/
      if ((JOYSTICK_Value >= 2) && (JOYSTICK_Value <= 38))
      {
         HalLcd_HW_WriteLine(HAL_LCD_LINE_5,"UP");
      }
      else if ((JOYSTICK_Value >= 74) && (JOYSTICK_Value <= 88))
      {
        HalLcd_HW_WriteLine(HAL_LCD_LINE_5,"RIGHT");
      }
      else if ((JOYSTICK_Value >= 60) && (JOYSTICK_Value <= 73))
      {
        HalLcd_HW_WriteLine(HAL_LCD_LINE_5,"LEFT");
      }
      else if ((JOYSTICK_Value >= 39) && (JOYSTICK_Value <= 59))
      {
        HalLcd_HW_WriteLine(HAL_LCD_LINE_5,"DOWN");
      }
      else if ((JOYSTICK_Value >= 89) && (JOYSTICK_Value <= 100))
      {
        HalLcd_HW_WriteLine(HAL_LCD_LINE_5,"CENTER");
      }
      flag = 0;
     
    }


  }
}


/*GPIO中断函数*/
#pragma vector = P2INT_VECTOR
__interrupt void P2_ISR(void) 
{ 
  if(P2IFG&BV(HAL_KEY_JOY_MOVE_PIN)){         //按键中断 
    P2IFG &= ~BV(HAL_KEY_JOY_MOVE_PIN); 
    
    //设置本次的采样通道
    JOYSTICK_ADC_SelectChanel(6);
    //开始采样
    JOYSTICK_ADC_Start(1);
    //等待转换结束
    while(!JOYSTICK_ADC_Busy());
    //读取采样结果
    JOYSTICK_Value = JOYSTICK_ADC_Get();
    
    flag = 1;
  } 
}