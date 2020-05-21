/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _UI_H
#define _UI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_bkp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
enum{
	REG_ON_OFF_TIMES = BKP_DR1,
	REG_UPTIME_TOTAL_30secQuant = BKP_DR2,
	REG_UPTIME_SINCE_CHRG_30secQuant = BKP_DR3,
	REG_POWERACCU_CAPACITY = BKP_DR4,

	REG_RIDE_TIME_TOTAL = BKP_DR5,
	REG_ODO_TOTAL = BKP_DR6

};

enum {
	NULL_COM = 0,
	START_COM,
	STOP_COM,

	ENC_UP,
	ENC_DOWN,
	ENC_BTN,
	ENC_BTN_LONG
};

typedef void (*funcPtr)(void);
typedef uint8_t (*callPtr)(uint16_t);
//typedef void (*WriteFuncPtr)(const char*);

typedef struct
{
	void *Next;
	void *Previous;
	void *Parent;
	void *Child;
//	funcPtr     EnterFunc;
	callPtr Callback;
//    funcPtr     LeaveFunc;
	char* caption;
} MenuItem;

#define M_M(Name,   Next, Previous, Parent, Child,  /*EnterFunc,*/ Callback/*, LeaveFunc*/,Caption) \
    extern MenuItem Next;     \
	extern MenuItem Previous; \
	extern MenuItem Parent;   \
	extern MenuItem Child;  \
	MenuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child,  /*(funcPtr)EnterFunc,*/(callPtr)Callback/*,(funcPtr)LeaveFunc*/, Caption}


#define NULL_MENU 	Null_Menu
#define NULL_FUNC  (void*)0

#define PREVIOUS   ( CurrWorkItem->Previous )
#define NEXT       ( CurrWorkItem->Next )
#define PARENT     ( CurrWorkItem->Parent )
#define CHILD      ( CurrWorkItem->Child )
#define ENTERFUNC  ( CurrWorkItem->EnterFunc )
//#define SELECTFUNC ( CurrWorkItem->SelectFunc )


/* Exported functions ------------------------------------------------------- */
void MenuDispatcher(uint16_t);
void ProcessEncButton();
void Menu_processBackupData();
void ShutDown();
void ProcessEncoder();

void MenuChange(MenuItem*);

/* Menus */
uint8_t menuScreen(uint16_t);

uint8_t MainScreen(uint16_t);
uint8_t AuxScreen(uint16_t);
uint8_t ChargeScreen(uint16_t);


#endif //_UI_H
