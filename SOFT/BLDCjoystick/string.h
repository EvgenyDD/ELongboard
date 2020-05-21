/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(char *);
int strlenNum(char *pText, int begin);
void itoa_(int32_t, char s[]);
void itoa_zeros(uint32_t n, char s[], uint8_t end);
void ftoaPseudo_(int32_t n, char s[], uint8_t pointNumbers);

void itoa_hex(int64_t n, char s[]);
void itoa_zeros2(uint32_t n, char s[], uint8_t end);

void dtoa_(int64_t n, char s[]);
void dtoaPositive_(int64_t n, char s[]);
void ftoa_(float, char str[], uint8_t precision);
void reverse(char s[]);
void strcat_(char first[], char second[]);
void strcat_NO_EOL(char first[], char second[]);
void strcat_white(char first[], uint8_t first_limit, char second[], uint8_t second_limit);
void strcpy_(char first[], char second[]);
void strcatNum2(char first[], char second[], int begin);

void strcatNum(char first[], char second[], int begin, int end);
void strcatnum(char first[], char second[], uint8_t num);

uint64_t pow10_(uint8_t x);
float log10_(int v);
float pow_(float x, float y);

float atof(uint8_t* num);
uint64_t stoi_(char *s);
uint64_t atoi_BCD(char *s, uint8_t start, uint8_t end);
uint64_t atoi_special(char *s, uint8_t start, uint8_t end);
uint64_t atoi_special2(char *s, uint8_t start, uint8_t end);
uint64_t atoi_special_BCD(char *s, uint8_t start, uint8_t end);
uint64_t atoi_special_BCDinv(char *s, uint8_t start, uint8_t end);

#endif //STRING_H
