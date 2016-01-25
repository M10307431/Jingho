/*
CC2540 
ghostyu
20130204
*/

/*
说明：

*/
#include<ioCC2540.h>
#include"Common.h"
#include"Hal_lcd.h"

uint8 temp;

void UART_Init()
{
  /*1、配置U0CSR寄存器*/
  U0CSR |= BV(7); //UART mode
  U0CSR |= BV(6); //Receiver enabled

  /*2、配置U0UCR寄存器*/
  U0UCR &= ~(BV(6)); //disabled Flow control
  U0UCR &= ~(BV(4)); //8-bit
  U0UCR &= ~(BV(3)); //parity disabled
  U0UCR &= ~(BV(2)); // 1 stop bit
  U0UCR |= BV(1);    //high stop bit
  U0UCR &= ~(BV(0)); //low start bit
  
  /*3、设置波特率*/
  //115200
  U0BAUD = 0xD8;
  U0GCR |= 0x0B;
  
  /*4、FLUSH*/
  U0UCR |= BV(7);
}

void UART_GPIO_Init()
{
  PERCFG &= ~(BV(0)); //选择P0端口uart
  P0SEL |=BV(5)|BV(4)|BV(3)|BV(2);
  P2DIR &=~(BV(7)|BV(6));
}
void UART_EnableRxInt()
{
  EA = 1;
  URX0IE = 1;
}
/*想UART0 发送数据*/
void UART_SendString(uint8 *data,int len)
{
  int i;
  for(i=0;i<len;i++){
    U0DBUF = *data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

int main()
{
  HAL_BOARD_INIT();
  HalLcd_HW_Init();

  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2540EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->UART_TEST");
  
  UART_GPIO_Init();
  UART_Init();
  UART_EnableRxInt();
  
  while(1){
    UART_SendString("123456",6);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, "Send: 123456/second");

    HalHW_WaitMS(1000);
  }
}

#pragma vector = URX0_VECTOR 
__interrupt void UART0_ISR(void) 
{ 
  URX0IF = 0;    //清中断标志 
  temp = U0DBUF; 
} 