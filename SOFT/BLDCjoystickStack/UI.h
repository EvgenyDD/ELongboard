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
	// total time of movement
	// energy (Wh) counter from the charge
	// total odometer (100m)

	// mute
	// control mode value

	// password value

	// joystick total power-on time since charge

	REG_ODO_100m = BKP_DR1,
	REG_MOVE_TIME_S = BKP_DR2,
	REG_ENERGY_mah = BKP_DR3,

	REG_SOUND_MUTE = BKP_DR4,
	REG_SET_LIM_CURR = BKP_DR5,
	REG_PASSWD = BKP_DR6,
	REG_JOYSTICK_POWERON_TIME_30secQuant = BKP_DR7,

	REG_LIMITER_SPEED = BKP_DR8
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

typedef enum
{
	CMODE_NO=0,
	CMODE_TIME_LIM,
	CMODE_ENERGY_LIM,
	CMODE_DIST_LIM
}CModeType;

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
void MenuChangeHome();

/* Menus */
uint8_t menuScreen(uint16_t);
uint8_t soundScreen(uint16_t code);
uint8_t cModeScreen(uint16_t code);
uint8_t passScreen(uint16_t code);
uint8_t selCtrlScreen(uint16_t code);

uint8_t mainScreen(uint16_t);
uint8_t auxScreen(uint16_t code);
uint8_t graphScreen(uint16_t code);
uint8_t chargeScreen(uint16_t);

void checkBKPRegs();
long map(long x, long in_min, long in_max, long out_min, long out_max);


#endif //_UI_H
