#ifndef HAL_LCD_H
#define HAL_LCD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "Common.h"


/* These are used to specify which line the text will be printed */
#define HAL_LCD_LINE_1      0x01
#define HAL_LCD_LINE_2      0x02
/*
   This to support LCD with extended number of lines (more than 2).
   Don't use these if LCD doesn't support more than 2 lines
*/
#define HAL_LCD_LINE_3      0x03
#define HAL_LCD_LINE_4      0x04
#define HAL_LCD_LINE_5      0x05
#define HAL_LCD_LINE_6      0x06
#define HAL_LCD_LINE_7      0x07
#define HAL_LCD_LINE_8      0x08

/* Max number of chars on a single LCD line */
#define HAL_LCD_MAX_CHARS   21
#define HAL_LCD_MAX_BUFF    25


void HalLcd_HW_Init(void);
void HalLcd_HW_WriteLine(uint8 line, const char *pText);
int8* IntToStr(int8* buf, int m);


#ifdef __cplusplus
}
#endif

#endif

