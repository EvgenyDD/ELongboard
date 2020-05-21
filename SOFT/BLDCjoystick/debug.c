/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "string.h"
#include "misc.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t DebugDecrease;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#ifdef DEBUG_ENABLE

/*******************************************************************************
 * Function Name  : DebugInit
 * Description    : Initialize debug (via DEBUG_USART)
 *******************************************************************************/
void DebugInit()
{

}

/*******************************************************************************
 * Function Name  : DebugSendString
 * Description    : Sends debug information (via DEBUG_USART)
 * Input          : pointer to text massive
 *******************************************************************************/
void DebugSendString(char *pText)
{
#ifdef USE_SEMIHOSTING
	SH_SendString(pText);
	SH_SendChar('\n');
	return;
#endif
	for(; *pText != '\0'; pText++)
	{
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_USART, *pText);
	}
	while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_USART, '\n');

}


void DebugSendStringLen(uint8_t *pText, uint8_t num)
{
#ifdef USE_SEMIHOSTING
	SH_SendString(pText);
	SH_SendChar('\n');
	return;
#endif
	for(; *pText != '\0' && num!=0; pText++, num--)
	{
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_USART, *pText);
	}
	while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_USART, '\n');

}



void DebugSendStringNoNL(char *pText)
{
#ifdef USE_SEMIHOSTING
	return;
#endif
	for(; *pText != '\0'; pText++)
	{
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_USART, *pText);
	}
}

/*******************************************************************************
 * Function Name  : DebugSendChar
 * Description    : Sends debug information (via DEBUG_USART)
 * Input          : char to send
 *******************************************************************************/
void DebugSendChar(uint8_t charTx)
{
#ifdef USE_SEMIHOSTING
	return;
#endif
	while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_USART, charTx);
}

/*******************************************************************************
 * Function Name  : DebugSysTimeEnd
 * Description    : send time from DebugSysTimeStart()
 *******************************************************************************/
/*void DebugSysTimeEnd()
 {
 char f[15];
 //if(Debug)
 ftoa_(DebugDecrease, f);
 DebugSendString(f);
 }*/

void DebugSendNum(uint64_t Num)
{
	char str[50];
	itoa_(Num,  (uint8_t*)str);
	DebugSendString(str);
}

void DebugSendNumHex(uint32_t Num)
{
	char str[50];
	itoa_hex(Num,  (uint8_t*)str);
	DebugSendString(str);
}

void DebugSendNumWSpace(uint64_t Num)
{
#ifdef USE_SEMIHOSTING
	return;
#endif
	char str[50];
	itoa_(Num,  (uint8_t*)str);
	for(uint8_t i = 0; str[i] != '\0'; i++)
	{
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_USART, str[i]);
	}

	while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_USART, '\t');
}

void DebugSendNumWSpaceHex(uint8_t Num)
{
#ifdef USE_SEMIHOSTING
	return;
#endif
	char str[50];

	itoa_hex((int64_t)Num,  (uint8_t*)str);
	for(uint8_t i = 0; str[i] != '\0'; i++)
	{
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_USART, str[i]);
	}

	while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_USART, ' ');
}

void DebugSendNumWDesc(char *string, uint32_t Num)
{
	char str[50] =
	{ '\0' }, number[20];
	strcat_( (uint8_t*)str, (uint8_t*)string);
	itoa_(Num,  (uint8_t*)number);
	strcat_( (uint8_t*)str, (uint8_t*)number);
	DebugSendString(str);
}

void DebugSendNumWDescOneLine(char *string, uint16_t Num)
{
	char str[50] =
	{ '\0' }, number[20];
	strcat_( (uint8_t*)str, (uint8_t*)string);
	itoa_(Num,  (uint8_t*)number);
	strcat_( (uint8_t*)str, (uint8_t*)number);
	DebugSendStringNoNL(str);
}


void DebugSendNumWDescHex(char *string, uint64_t Num)
{
	char str[50] =
	{ '\0' }, number[20];
	strcat_( (uint8_t*)str, (uint8_t*)string);
	itoa_hex(Num,  (uint8_t*)number);
	strcat_( (uint8_t*)str, (uint8_t*)number);
	DebugSendString(str);
}

void DebugSendFloat(float Num)
{
	char str[50];
	ftoa_(Num, (uint8_t*)str, 3);
	DebugSendString(str);
}

void DebugSendTD(RTC_TimeDateTypeDef *X)
{
	char str[50];
	RTCStringDateDisplay((uint8_t*)str, X);
	str[10] = ' ';
	str[11] = ' ';
	RTCStringTimeWSecOffset((uint8_t*)(str+12), X);
	DebugSendString(str);
}

///*******************************************************************************
// * Function Name  : DEBUG_USART_IRQHandler
// * Description    :
// *******************************************************************************/
//void DEBUG_USART_IRQHandler()
//{
//	// Check if the DEBUG_USART receive interrupt flag was set
//	if(USART_GetITStatus(DEBUG_USART, USART_IT_RXNE) != RESET)
//	{
//		USART_ClearITPendingBit(DEBUG_USART, USART_IT_RXNE);
//		//uint8_t t = USART_ReceiveData(DEBUG_USART);
//
//	}
//}
#else
	void DebugInit(){}
	void DebugSendString(char *pText){}
	void DebugSendStringLen(uint8_t *pText, uint8_t num){}
	void DebugSendStringNoNL(char *pText){}
	void DebugSendChar(uint8_t charTx){}
	void DebugSendNum(uint64_t Num){}
	void DebugSendNumHex(uint32_t Num){}
	void DebugSendNumWSpace(uint64_t Num){}
	void DebugSendNumWSpaceHex(uint8_t Num){}
	void DebugSendNumWDesc(char *string, uint32_t Num){}
	void DebugSendNumWDescOneLine(char *string, uint16_t Num){}
	void DebugSendNumWDescHex(char *string, uint64_t Num){}
	void DebugSendFloat(float Num){}

#endif
