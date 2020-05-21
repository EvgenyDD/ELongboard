/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEBUG_H
#define DEBUG_H


/* Includes ------------------------------------------------------------------*/
#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>




/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void DebugInit();
void DebugSendString(char*);
void DebugSendStringLen(uint8_t *pText, uint8_t num);
void DebugSendChar(uint8_t);
void DebugSysTimeEnd();
void DebugSysTimeStart();
void DebugSendNum(uint64_t Num);
void DebugSendNumHex(uint32_t Num);
void DebugSendNumWSpace(uint64_t Num);
void DebugSendNumWSpaceHex(uint8_t Num);
void DebugSendNumWDesc(char *string, uint32_t Num);
void DebugSendNumWDescOneLine(char *string, uint16_t Num);
void DebugSendNumWDescHex(char *string, uint64_t Num);


void DebugSendFloat(float Num);


#endif //DEBUG_H
