/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_crc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_pwr.h"
#include "misc.h"
#include "stm32f10x_bkp.h"

#include "hw_hal.h"
#include "LCD.h"
#include "rfhelp.h"
#include "interface.h"
#include "UI.h"
#include "nrf_driver.h"

#include <stdbool.h>

#include "sound.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t timerDelay = 0;

//debug + statistics vars
uint32_t failPKT = 0, succPKT = 1;
uint32_t timeRx = 0, timeRxCount = 0;

uint32_t thrStart = 2400, thrEnd = 2800, thrCorrel = 10;
uint32_t brkStart = 2400, brkEnd = 2800, brkCorrel = 10;
int16_t thrLevel = 0, brkLevel = 0;


uint32_t msHeartbeat1000 = 0;

uint32_t sysTickerMs = 0;
uint32_t buzzDelay = 0, soundDelay = 0;

uint32_t shutdownDelay = SHDN_DELAY_MS;

uint32_t upTimeS = 0;
uint32_t movingTimeCounterS = 0;
bool isMovingNow = false;

uint16_t updateCounter = 0;

bool nrfNeedRestart = false;


/* Extern variables ----------------------------------------------------------*/
extern const unsigned char font5x8[];
extern uint16_t btnDelay[2];

extern InterfaceStatusTypedef currentState;

extern int nrf_restart_rx_time;
extern int nrf_restart_tx_time;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : delay_ms
 * Description    : Delay the code on the N cycles of SysTick Timer.
 * Input          : N Delay Cycles.
 *******************************************************************************/
void delay_ms(uint32_t nTime)
{
	timerDelay = nTime;
	while(timerDelay != 0);
}


/*******************************************************************************
 * Function Name  : SysTick_Handler
 * Description    : Handles SysTick Timer Interrupts every 1ms/2.
 *******************************************************************************/
void SysTick_Handler()
{
	if(timerDelay > 0)
	{
		timerDelay--;
	}

	if(updateCounter != 0)
	{
		updateCounter--;
	}

	if(nrfNeedRestart == false)
	{
		if(nrf_restart_rx_time > 0 && nrf_restart_tx_time > 0)
		{
			nrf_restart_rx_time--;
			nrf_restart_tx_time--;
		}
		else
		{
			nrfNeedRestart = true;
		}
	}

	if(++msHeartbeat1000 >= 1000)
	{
		msHeartbeat1000 = 0;
		upTimeS++;

		if(isMovingNow)
		{
			movingTimeCounterS++;
		}
	}

	sysTickerMs++;

	if(btnDelay[0] != 0)
	    btnDelay[0]++;

	if(btnDelay[1] != 0)
	    btnDelay[1]++;

	if(buzzDelay)
	{
		if(--buzzDelay == 0)
			HapticOff();
	}

	if(soundDelay)
	{
		if(--soundDelay == 0)
			Sound(0);
	}

	SoundDispatcher();

	if(shutdownDelay)
	{
		shutdownDelay--;
	}
}


/*******************************************************************************
 * Function Name  : map
 *******************************************************************************/
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/*******************************************************************************
 * Function Name  : CalibrateSticks
 *******************************************************************************/
void CalibrateSticks()
{
	LCD_setFont(font5x8);

#define M_ZERO_SAMPLES 	40
#define M_EACH_SAMPLES 	40

	//ZERO
	LCD_Clear();
	LCD_StringCentered(0, 80,  "ZERO", 1);
	LCD_StringCentered(0, 70,  "STICK", 1);
	LCD_Update();

	delay_ms(1500);

	thrStart = brkStart = 0;

	for(uint8_t i = 0; i < M_ZERO_SAMPLES; i++, delay_ms(20))
	{
		thrStart += GetThrottle();
		brkStart += GetBrake();
//		delay_ms(10);
	}

	thrStart /= M_ZERO_SAMPLES;
	brkStart /= M_ZERO_SAMPLES;

	//Max 1
	LCD_Clear();
	LCD_String(0, 0,  "MAX", 1);
	LCD_StringCentered(0, 70,  "THROT", 1);
	LCD_Update();

	while((int16_t)(GetThrottle() - thrStart) < 600/* || (GetBrake() - brkStart) > 100*/);

	LCD_Clear();
	LCD_String(0, 0,  "MEAS", 1);
	LCD_StringCentered(0, 70,  "THROT", 1);
	LCD_Update();

	delay_ms(1500);

	thrEnd = brkCorrel = 0;

	for(uint8_t i = 0; i < M_EACH_SAMPLES; i++, delay_ms(20))
	{
		thrEnd += GetThrottle();
		brkCorrel += GetBrake()-brkStart;
	}

	thrEnd /= M_EACH_SAMPLES;
	brkCorrel /= M_EACH_SAMPLES;

	delay_ms(800);

	//Max 2
	LCD_Clear();
	LCD_String(0, 0,  "MAX", 1);
	LCD_StringCentered(0, 70,  "BRAKE", 1);
	LCD_Update();

	while(/*(GetThrottle() - thrStart) > 100 || */(int16_t)(GetBrake() - brkStart) < 500);

	LCD_Clear();
	LCD_String(0, 0,  "MEAS", 1);
	LCD_StringCentered(0, 70,  "BRAKE", 1);
	LCD_Update();

	delay_ms(800);

	brkEnd = thrCorrel = 0;

	for(uint8_t i = 0; i < M_EACH_SAMPLES; i++, delay_ms(20))
	{
		brkEnd += GetBrake();
		thrCorrel += GetThrottle()-thrStart;
	}

	brkEnd /= M_EACH_SAMPLES;
	thrCorrel /= M_EACH_SAMPLES;

//	//MAX BOTH
//	LCD_StringCentered(0, 80,  "MAX", 1);
//	LCD_StringCentered(0, 70,  "STICK", 1);
//	LCD_Update();
//
//	for(uint8_t i = 0; i < 20; i++)
//	{
//		thrEnd += GetThrottle();
//		brkEnd += GetBrake();
//		delay_ms(10);
//	}
//	thrEnd /= 20;
//	brkEnd /= 20;
}


/*******************************************************************************
 * Function Name  : GetControls
 *******************************************************************************/
void GetControls()
{
	uint16_t brkCurr = GetBrake(), thrCurr = GetThrottle();

#if 1

	static uint16_t crr = 0;

	if(crr < 100)
	{
		crr++;
	}
	else
	{

	if(brkCurr > brkEnd)
		brkEnd = brkCurr;
	if(brkCurr < brkStart)
		brkStart = brkCurr;

	if(thrCurr > thrEnd)
		thrEnd = thrCurr;
	if(thrCurr < thrStart)
		thrStart = thrCurr;
	}

	brkLevel = map(brkCurr, brkStart, brkEnd, 0, 100);
	thrLevel = map(thrCurr, thrStart, thrEnd, 0, 100);

		if(thrLevel > 100)
			thrLevel = 100;
		else if(thrLevel < 0)
			thrLevel = 0;

		if(brkLevel > 100)
			brkLevel = 100;
		else if(brkLevel < 0)
			brkLevel = 0;

	int16_t brkLevelOld = brkLevel;
	int16_t thrLevelOld = thrLevel;

	brkLevel -= thrLevelOld * brkCorrel / 100;
	thrLevel -= brkLevelOld * thrCorrel / 100;

	if(thrLevel > 100)
		thrLevel = 100;
	else if(thrLevel < 0)
		thrLevel = 0;

	if(brkLevel > 100)
		brkLevel = 100;
	else if(brkLevel < 0)
		brkLevel = 0;
#else

	thrLevel = thrCurr;
	brkLevel = brkCurr;
#endif
}

/*******************************************************************************
 * Function Name  : main
 *******************************************************************************/
int main()
{
	InitPeriph();

	upTimeS = BKP_ReadBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant)*30;


	if(!IsCharging())
	{
		delay_ms(1000);
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
		{
			PWR_WakeUpPinCmd(ENABLE); /* Enable wake from WKUP pin*/
			PWR_EnterSTANDBYMode();
		}
		else
		{
			HapticBuzz(300);
			Beep(4000, 200);
				delay_ms(500);
			HapticBuzz(100);
			Beep(5000, 200);
				delay_ms(400);
			HapticBuzz(100);
			while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));
		}
	}

	GPIO_SetBits(GPIOB, GPIO_Pin_3); //boost_vdd

	LCD_InitOn();
	LCD_setFont(font5x8);

	checkBKPRegs();

	HALLEnable();

	InterfaceInit();

	//CalibrateSticks();


	while(1)
	{
		if(updateCounter == 0)
		{
			updateCounter = 50; //ms

			MenuDispatcher(NULL_COM);

			Menu_processBackupData();
		}

		NRF_processIRQData();

		if(nrfNeedRestart)
		{
			rfhelp_power_up();
			rfhelp_restart();
			nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
			nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
			nrfNeedRestart = false;
		}

		GetControls();
		InterfaceCallback();

		ProcessEncButton();

		ProcessEncoder();

		delay_ms(5);
	}
}


/*******************************************************************************
 * Function Name  : ???
 *******************************************************************************/
void EXTI1_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}
