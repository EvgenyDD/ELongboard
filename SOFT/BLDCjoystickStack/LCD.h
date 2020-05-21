/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_H
#define LCD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_rcc.h"

#include "macros.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LCD_InitOn();
void LCD_Off();
void LCD_Update();

void LCD_Clear();
void LCD_Fill();

void LCD_HLine(uint8_t x, uint8_t y, uint8_t width, uint8_t color);
void LCD_VLine(uint8_t x, uint8_t y, uint8_t height, uint8_t color);
void LCD_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void LCD_RectangleFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void LCD_RectangleInvert(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

void LCD_MultChar(uint8_t x, uint8_t y, char s, uint8_t scale);
void LCD_Char(uint8_t x, uint8_t y, char s);
void LCD_setFont(const uint8_t* font);
void LCD_BigString(int x, int y, char *s, uint8_t scale);
void LCD_String(uint8_t x, uint8_t y, char* s, uint8_t scale);
void LCD_StringCentered(uint8_t x, uint8_t y, char* s, uint8_t scale);
void LCD_Pixel(uint8_t x, uint8_t y, uint8_t color);

void LCD_Num(uint8_t x, uint8_t y, int32_t num, uint8_t scale);
void LCD_Float(uint8_t x, uint8_t y, float num, uint8_t prec);
void LCD_Float2Str(uint8_t x, uint8_t y, char* str1, float num, char* str2, uint8_t prec, uint8_t scale);
void LCD_FloatPseudo(uint8_t x, uint8_t y, int32_t num, uint8_t prec, uint8_t scale);
void LCD_FloatPseudoWString(uint8_t x, uint8_t y, int32_t num, char *text, uint8_t prec, uint8_t scale);
void LCD_FloatPseudoW2String(uint8_t x, uint8_t y, char *text, int32_t num, char *text2, uint8_t prec, uint8_t scale);
void LCD_NumWDesc(uint8_t x, uint8_t y, char *s, int32_t num);
void LCD_textNumText(uint8_t x, uint8_t y, char *s, int32_t num, char *s2, uint8_t scale);
void LCD_Celsium(uint8_t x, uint8_t y, char *text, uint16_t num, uint8_t scale);

void LCD_InverseModeOn();
void LCD_InverseModeOff();


#endif //LCD_H
