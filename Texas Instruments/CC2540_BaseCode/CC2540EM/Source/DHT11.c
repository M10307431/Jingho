/*
CC2530 
ghostyu
201304019
*/

/*
说明：
如果使用SmartRF平台开发板，需要将P10的倒数第二排用跳线帽短路，
用于将P0_7连接到DS18B20插座上，DHT11和DS18B20公用一个插座

使用SmartRF-BB平台开发板，将DHT11的数据端口连接至P0_7
*/
#include<ioCC2540.h>
#include"Common.h"
#include"Hal_lcd.h"

/**************************************************
  接口定义，移植此程序只需修改下列宏定义和延时函数
**************************************************/
#define IN_DQ		P0_7
#define DQ_PIN		7
#define DQ_PORT		P0DIR


#define SET_OUT DQ_PORT|=BV(DQ_PIN);asm("NOP");asm("NOP")
#define SET_IN  DQ_PORT&=~(BV(DQ_PIN));asm("NOP");asm("NOP")

#define CL_DQ  IN_DQ=0;asm("NOP");asm("NOP")
#define SET_DQ IN_DQ=1;asm("NOP");asm("NOP") 


uint8  CheckSum;
uint8  tmp8BitValue;

static void Delay_nus(uint16 s) 
{
  while (s--)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }
}

void Read8Bit(void)
{
  static uint8  OverTimeCnt = 0;
  uint8 i,tmpBit;
  
  for(i=0;i<8;i++)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);
    //while(IN_DQ == 0);
    Delay_nus(19);//12
    if(IN_DQ == 1)
      tmpBit = 1;
    else
      tmpBit = 0;
    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //while(IN_DQ == 1);
    //超时则跳出for循环		  
    if(OverTimeCnt==1)
      break;
    
    tmp8BitValue<<=1;
    tmp8BitValue|=tmpBit;        //0
  
  }

}

/*
//how to use ReadValue
uint8 Sensor[4];
ReadValue(Sensor);

Sensor[0]
-->湿度整数值
Sensor[1]
-->湿度小数值

Sensor[2]
-->温度整数值
Sensor[3]
-->温度小数值
*/
void ReadValue(uint8 *sv)
{
  static uint8  OverTimeCnt = 0;
  SET_OUT; 
  CL_DQ; 
  Delay_nus(20000);//主机拉低至少18ms  
  SET_DQ;
  Delay_nus(20);//总线由上拉电阻拉高 主机延时20us-40us
  SET_IN;
  if(IN_DQ == 0)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);

    while(IN_DQ == 1);
    //数据接收状态	
    Read8Bit();
    sv[0]=tmp8BitValue;
    Read8Bit();
    sv[1]=tmp8BitValue;
    Read8Bit();
    sv[2]=tmp8BitValue;
    Read8Bit();
    sv[3]=tmp8BitValue;
	
    Read8Bit();
    CheckSum = tmp8BitValue;
    
    if(CheckSum == sv[0]+sv[1]+sv[2]+sv[3])
    {
      CheckSum = 0xff;
    
    }
	
    
  }

}

int main()
{
  HAL_BOARD_INIT();
  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2530EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->DHT11_TEST"); 
  HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "Current hum & temp:");
  
  uint8 hum[10];
  uint8 temp[10];
  uint8 Sensor[4];
  while(1){
    ReadValue(Sensor);
    IntToStr(hum,Sensor[0]);
    IntToStr(temp,Sensor[2]);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, hum);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_6, temp);
    HalHW_WaitMS(500);
  }
  return 0;
}