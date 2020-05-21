/* Includes ------------------------------------------------------------------*/
#include "UI.h"
#include "LCD.h"
#include "interface.h"
#include "hw_hal.h"
#include "rfhelp.h"
#include "stm32f10x_pwr.h"
#include "misc.h"
#include "sound.h"


/* Private typedef -----------------------------------------------------------*/
#define LAST_MENU_ID MenuDispCtrlSett

#define abs(x)  (( (x)<0 ) ? (-(x)) : (x))


mc_control_mode controlMode = CONTROL_MODE_CURRENT;

CModeType cModeNow = CMODE_NO;

MenuItem Null_Menu =
{ (void*) 0, (void*) 0, (void*) 0, (void*) 0, /*NULL_FUNC,*/NULL_FUNC /*,NULL_FUNC*/, 	""};

M_M(MenuDisplay,    NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, menuScreen, 	"Menu");
M_M(SoundDisplay,	NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, soundScreen, 	"Sound");
M_M(CModeDisplay, 	NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, cModeScreen, 	"CMode");
M_M(PassDisplay, 	NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, passScreen, 	"Pass");
M_M(SelCtrlDisplay, NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, selCtrlScreen,	"Ctrl");

M_M(MainDisplay,  	AuxDisplay,		GraphDisplay,	NULL_MENU,NULL_MENU, mainScreen, 	"");
M_M(AuxDisplay,   	GraphDisplay,	MainDisplay,	NULL_MENU,NULL_MENU, auxScreen, 	"AUX");
M_M(GraphDisplay,  	MainDisplay,	AuxDisplay,		NULL_MENU,NULL_MENU, graphScreen, 	"");

M_M(ChargeDisplay, 	NULL_MENU,		NULL_MENU,		NULL_MENU,NULL_MENU, chargeScreen, 	"");


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//MENU_TYPE screenMode = MenuMain;

MenuItem*	CurrWorkItem = &MainDisplay;
MenuItem*   CurrScreen = &MainDisplay;

uint16_t btnDelay[2] = {0,0};

uint32_t nrfResets = 0;

uint16_t limiterTime = 60, limiterDist = 2, limiterEnergy = 100;
uint16_t limiterTimeCounter, limiterDistCounter, limiterEnergyCounter;



/* Extern variables ----------------------------------------------------------*/
extern int16_t thrLevel, brkLevel;
extern InterfaceStatusTypedef currentState;

extern const unsigned char font3x5[], font5x8[];
extern uint32_t sysTickerMs;

extern uint32_t thrStart, thrEnd, thrCorrel;
extern uint32_t brkStart, brkEnd, brkCorrel;

extern uint8_t cntcnt;
extern uint8_t moveDirection;

extern uint32_t movingTimeCounterS;


extern float limSpeed, limDuty;

extern uint32_t shutdownDelay;
extern uint32_t soundDelay;

extern uint32_t upTimeS;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MenuDispatcher
*******************************************************************************/
void MenuDispatcher(uint16_t code)
{
	if(IsCharging() && CurrWorkItem != &ChargeDisplay)
	{
		MenuChange(&ChargeDisplay);
	}
	if((IsCharging() == false) && (CurrWorkItem == &ChargeDisplay))
	{
		MenuChangeHome();
	}

	if(code)
	{
		shutdownDelay = SHDN_DELAY_MS;
	}

	LCD_Clear();

	if(*CurrWorkItem->caption != '\n')
	{
		LCD_setFont(font5x8);
		LCD_StringCentered(0, 101-9, CurrWorkItem->caption, 1);
	}

	if(CurrWorkItem->Callback!=NULL_FUNC)
	{
		CurrWorkItem->Callback(code);
	}

	LCD_Update();
}

void checkBKPRegs()
{
	if(BKP_ReadBackupRegister(REG_ODO_100m) >= 0xFFFF)
		BKP_WriteBackupRegister(REG_ODO_100m, 0);

	if(BKP_ReadBackupRegister(REG_MOVE_TIME_S) >= 0xFFFF)
		BKP_WriteBackupRegister(REG_MOVE_TIME_S, 0);

	if(BKP_ReadBackupRegister(REG_ENERGY_mah) >= 0xFFFF)
		BKP_WriteBackupRegister(REG_ENERGY_mah, 0);

	if(BKP_ReadBackupRegister(REG_SOUND_MUTE) >= 1)
		BKP_WriteBackupRegister(REG_SOUND_MUTE, 0);

	if(BKP_ReadBackupRegister(REG_PASSWD) >= 9999)
		BKP_WriteBackupRegister(REG_PASSWD, 1234);

	if(BKP_ReadBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant) >= 0xFFFF)
		BKP_WriteBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant, 0);

	uint16_t value = BKP_ReadBackupRegister(REG_SET_LIM_CURR);
	if(value < 10 || value > 100)
		BKP_WriteBackupRegister(REG_SET_LIM_CURR, 60);
}


void MenuChange(MenuItem *NextWorkItem)
{
//	if(CurrWorkItem->Callback!=NULL_FUNC)
//		CurrWorkItem->Callback(STOP_COM);

	CurrWorkItem = NextWorkItem;

	if(CurrWorkItem->Callback!=NULL_FUNC)
		CurrWorkItem->Callback(START_COM);
}

void MenuChangeHome()
{
	if(CurrScreen != &NULL_MENU)
		CurrWorkItem = CurrScreen;
}



/*******************************************************************************
* Function Name  :
*******************************************************************************/
void Menu_processBackupData()
{
	static uint32_t prevTimer = 0;
	if((sysTickerMs - prevTimer) > 30000) //30sec quantization
	{
		prevTimer = sysTickerMs;
		BKP_WriteBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant, BKP_ReadBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant) + 1);
	}

	uint32_t timeOfMovingS = movingTimeCounterS;
	static uint32_t prevTimeS = 0;
//	if(prevTimeS == 0 && currentState.odometer > 1)
//	{
//		prevTimeS = currentState.odometer;
//	}
	if(prevTimeS != timeOfMovingS)
	{
		BKP_WriteBackupRegister(REG_MOVE_TIME_S, BKP_ReadBackupRegister(REG_MOVE_TIME_S) + timeOfMovingS - prevTimeS);
		prevTimeS = timeOfMovingS;
	}
	currentState.timeMoveS = BKP_ReadBackupRegister(REG_MOVE_TIME_S);
}


/*******************************************************************************
* Function Name  : ProcessEncButton
*******************************************************************************/
void ProcessEncButton()
{
    static uint8_t lastButtonState = 0;

    if(shutdownDelay == 0)
    	ShutDown();

    //encoder button
	if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1))
	{
		if(lastButtonState == 0)
		{
			lastButtonState = 1;
			btnDelay[0] = 1;
		}
		else if(lastButtonState == 1)
		{
			if(btnDelay[0] > 1000)
			{
            	Beep(4000, 250);

            	MenuDispatcher(ENC_BTN_LONG);

                btnDelay[0] = 0;
                lastButtonState = 2;
            }
		}
	}
	else
	{
		if(lastButtonState == 1)
		{
			Beep(3000, 250);
			MenuDispatcher(ENC_BTN);
			btnDelay[0] = 0;
		}

		lastButtonState = 0;
	}

	/* Fire button */
    static uint8_t lastStateFireButton = 0;
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
    {
        if(lastStateFireButton == 0)
        {
        	lastStateFireButton = 1;
        	btnDelay[1] = 1;
        }
        else
        {
			if(btnDelay[1] > 1000)
			{
				NVIC_InitTypeDef NVIC_InitStructure;
				NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
				NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x07;
				NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
				NVIC_Init(&NVIC_InitStructure);
				//Beep(6000, 1000);
				shutdownDelay = 0;

				SendPowerOff();
				ShutDown();

				btnDelay[0] = 0;
				lastStateFireButton = 0;
			}
        }
    }
    else
    {
    	if(lastStateFireButton == 1)
		{
    		Beep(2000, 250);//PlayStarWars();
    		HapticBuzz(200);

    		moveDirection = moveDirection == 0 ? 1 : 0; //invert moving direction

			btnDelay[1] = 0;
		}

        lastStateFireButton = 0;
    }
}


/*******************************************************************************
* Function Name  : ShutDown
*******************************************************************************/
void ShutDown()
{
	LCD_Clear();
	LCD_Fill();
	LCD_Update();

	HapticBuzz(100);
	Beep(4000, 200);
		delay_ms(400);
	HapticBuzz(100);
		delay_ms(400);
	HapticBuzz(300);
	Beep(3000, 200);
		delay_ms(300);

	while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));
	LCD_Off();
	//NRF_Sleep();
	rfhelp_power_down();
	__disable_irq();

	PWR_WakeUpPinCmd(ENABLE);   /* Enable wake from WKUP pin*/
	PWR_EnterSTANDBYMode();
}


/*******************************************************************************
* Function Name  : ProcessEncoder
*******************************************************************************/
void ProcessEncoder()
{
    static uint16_t prevEnc = 0;

    if(prevEnc < TIM4->CNT/2)
    {
    	Beep(4000, 20);
    	MenuDispatcher(ENC_UP);
    }
    else if(prevEnc > TIM4->CNT/2)
    {
    	Beep(4000, 20);
    	MenuDispatcher(ENC_DOWN);
    }
    prevEnc = TIM4->CNT/2;
}


void smartIncrementValue(float* value)
{
	if(*value < 0.01)
		*value += 0.001;
	else if(*value < 0.1)
		*value += 0.01;
	else if(*value < 1.0)
		*value += 0.1;
	else if(*value < 10.0)
		*value += 1.0;
	else
		*value += 5.0;
}

void smartDecrementValue(float* value)
{
	if(*value < 0.001)
	{
	}
	else if(*value < 0.01)
	{
		*value -= 0.001;
	}
	else if(*value < 0.1)
	{
		*value -= 0.01;
		if(*value < 0.001)
			*value = 0.009;
	}
	else if(*value < 1.0)
	{
		*value -= 0.1;
		if(*value < 0.001)
			*value = 0.09;
	}
	else if(*value < 10.0)
	{
		*value -= 1.0;
		if(*value < 0.001)
			*value = 0.9;
	}
	else
	{
		*value -= 5.0;
		if(*value < 0.001)
			*value = 9.0;
	}
}


uint8_t menuScreen(uint16_t code)
{
	static uint8_t curPos = 0;

	LCD_setFont(font5x8);

	uint8_t i = 0;
	LCD_String(0, 80-i++*9, "CMode", 1);
	LCD_String(0, 80-i++*9, "Reset", 1);
	LCD_String(0, 80-i++*9, "RsSpd", 1);
	LCD_String(0, 80-i++*9, "Sound", 1);
	LCD_String(0, 80-i++*9, "Pass", 1);
	LCD_String(0, 80-i++*9, "SelCt", 1);

	switch(code)
	{
	case START_COM:
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		switch(curPos)
		{
		case 0: //control modes
			MenuChange(&CModeDisplay);
			break;

		case 1: //reset all counters
			currentState.packetTxFail = 0;
			currentState.packetTxSucc = 0;
			currentState.speedMax = 0;
			BKP_WriteBackupRegister(REG_ENERGY_mah, 0);
			break;

		case 2: //reset speed
			currentState.speedMax = 0;
			BKP_WriteBackupRegister(REG_MOVE_TIME_S, 0);
			BKP_WriteBackupRegister(REG_ODO_100m, 0);
			currentState.odometer = 0;
			break;

		case 3: //sound set
			MenuChange(&SoundDisplay);
			break;

		case 4: //password set
			MenuChange(&PassDisplay);
			break;

		case 5: //select control
			MenuChange(&SelCtrlDisplay);
			break;
		}

		break;

	case ENC_BTN_LONG:
		MenuChangeHome();
		break;

	case ENC_UP:
		if(--curPos >= i)
			curPos = 0;
		break;

	case ENC_DOWN:
		if(++curPos >= i)
			curPos = i-1;
		break;
	}

	LCD_RectangleInvert(0, 80 - curPos * 9, 31, 9);

	return 0;
}


uint8_t cModeScreen(uint16_t code)
{
#define SPEED_MAX_LIMIT	(40)

	LCD_setFont(font5x8);
	uint8_t i=0;
	static uint8_t curPos = 0;
	static int8_t editItem = -1;

	LCD_String(0, 80-i++*9, "Reset", 1);
	LCD_String(0, 80-i++*9, "Speed", 1);
	LCD_Num(0, 80-i++*9, BKP_ReadBackupRegister(REG_LIMITER_SPEED), 1); //2
	LCD_String(0, 80-i++*9, "Time", 1);
	LCD_Num(0, 80-i++*9, (limiterTime/60)*100+limiterTime%60, 1); //4
	LCD_String(0, 80-i++*9, "Dist", 1);
	LCD_FloatPseudo(0, 80-i++*9, limiterDist, 1, 1); //6
	LCD_String(0, 80-i++*9, "Energ", 1);
	LCD_Num(0, 80-i++*9, limiterEnergy, 1); //8

	switch(code)
	{
	case START_COM:
		editItem = -1;
		curPos = 0;
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		switch(curPos)
		{
		case 0: //reset mode
			cModeNow = CMODE_NO;
			MenuChangeHome();
			break;

		case 2:
		case 4:
		case 6:
		case 8:
			editItem = (editItem == -1) ? curPos : -1;
			break;

		case 3:
			cModeNow = CMODE_TIME_LIM;
			limiterTimeCounter = limiterTime;
			MenuChangeHome();
			break;

		case 5:
			cModeNow = CMODE_DIST_LIM;
			limiterDistCounter = limiterDist + currentState.odometer;
			MenuChangeHome();
			break;

		case 7:
			cModeNow = CMODE_ENERGY_LIM;
			limiterEnergy = limiterEnergy + currentState.capacityDisc - currentState.capacityChrg;
			MenuChangeHome();
			break;
		}
		break;

	case ENC_BTN_LONG:
		MenuChange(CurrScreen);
		break;

	case ENC_UP:
		if(editItem == 2)
		{
			if(BKP_ReadBackupRegister(REG_LIMITER_SPEED) < SPEED_MAX_LIMIT)
				BKP_WriteBackupRegister(REG_LIMITER_SPEED, BKP_ReadBackupRegister(REG_LIMITER_SPEED)+1);
			else
				BKP_WriteBackupRegister(REG_LIMITER_SPEED, SPEED_MAX_LIMIT);
		}
		if(editItem == 4)
		{
			limiterTime += 10; //secs
			if(limiterTime >= 600)
				limiterTime = 600;
		}
		else if(editItem == 6)
		{
			if(++limiterDist >= 50)//5km
			{
				limiterDist = 50;
			}
		}
		else if(editItem == 8)
		{
			limiterEnergy += 50; //50mah
			if(limiterEnergy > 5000)
				limiterEnergy = 5000;
		}
		else
		{
			if(--curPos >= i)
				curPos = 0;
		}
		break;

	case ENC_DOWN:
		if(editItem == 2)
		{
			if(BKP_ReadBackupRegister(REG_LIMITER_SPEED) > 1)
				BKP_WriteBackupRegister(REG_LIMITER_SPEED, BKP_ReadBackupRegister(REG_LIMITER_SPEED)-1);
		}
		else if(editItem == 4)
		{
			limiterTime -= 10; //secs
			if(limiterTime >= 600)
				limiterTime = 20;//secs
		}
		else if(editItem == 6)
		{
			if(--limiterDist >= 50)//5km
			{
				limiterDist = 0;
			}
		}
		else if(editItem == 8)
		{
			limiterEnergy -= 50; //50mah
			if(limiterEnergy > 5000)
				limiterEnergy = 0;
		}
		else
		{
			if(++curPos >= i)
				curPos = i-1;
		}
		break;
	}

	if(editItem != -1)
	{
		if((sysTickerMs % 1000) > 500)
			LCD_RectangleInvert(0, 80 - editItem * 9, 31, 9);
	}
	else
		LCD_RectangleInvert(0, 80 - curPos * 9, 31, 9);
	return 0;
}


uint8_t soundScreen(uint16_t code)
{
	LCD_setFont(font5x8);

	uint8_t i = 0;
	LCD_String(0, 80-i++*9, "Mute", 1);
	LCD_String(0, 80-i++*9, BKP_ReadBackupRegister(REG_SOUND_MUTE)!=0 ? ">Off" : ">On", 1);

	switch(code)
	{
	case START_COM:
		break;

	case ENC_BTN_LONG:
		MenuChangeHome();

	case ENC_BTN:
	case ENC_UP:
	case ENC_DOWN:
		BKP_WriteBackupRegister(REG_SOUND_MUTE, BKP_ReadBackupRegister(REG_SOUND_MUTE)!=0 ? 0 : 1);
		break;
	}
	return 0;
}


uint8_t passScreen(uint16_t code)
{
	uint8_t currentDigit[4] = {0,0,0,0};
	static uint8_t currentDigitPtr = 0;
	switch(code)
	{
	case START_COM:
		currentDigit[0] = BKP_ReadBackupRegister(REG_PASSWD) / 1000 % 10;
		currentDigit[1] = BKP_ReadBackupRegister(REG_PASSWD) / 100 % 10;
		currentDigit[2] = BKP_ReadBackupRegister(REG_PASSWD) / 10 % 10;
		currentDigit[3] = BKP_ReadBackupRegister(REG_PASSWD) % 10;
		break;

	case ENC_BTN_LONG:
		BKP_WriteBackupRegister(REG_PASSWD,
				currentDigit[0] * 1000 + currentDigit[1] * 100 + currentDigit[2] * 10 + currentDigit[3]);
		MenuChangeHome();
		break;

	case ENC_BTN:
		if(++currentDigitPtr >= 4)
			currentDigitPtr = 0;
		break;

	case ENC_UP:
		if(++currentDigit[currentDigitPtr] >= 10)
			currentDigit[currentDigitPtr] = 0;
		break;

	case ENC_DOWN:
		if(--currentDigit[currentDigitPtr] >= 10)
			currentDigit[currentDigitPtr] = 9;
		break;
	}

	LCD_setFont(font5x8);
	for(uint8_t i=0; i<4; i++)
	{
		if(currentDigitPtr == i)
			LCD_InverseModeOn();
		LCD_Char(6*i, 80, currentDigit[i]+'0');
		if(currentDigitPtr == i)
			LCD_InverseModeOff();
	}

	return 0;
}

uint8_t selCtrlScreen(uint16_t code)
{
	LCD_setFont(font5x8);

	switch(controlMode)
	{
	case CONTROL_MODE_DUTY:
		LCD_Float2Str(0, 80, "D", limDuty, "", 2, 1);
		break;

	case CONTROL_MODE_SPEED:
		LCD_Float2Str(0, 80, "S", limSpeed, "", 1, 1);
		break;

	case CONTROL_MODE_CURRENT:
		LCD_Float2Str(0, 80, "C", (float)BKP_ReadBackupRegister(REG_SET_LIM_CURR), "", 1, 1);
		break;

	default:
		break;
	}

	switch(code)
	{
	case START_COM:
		CurrScreen = &AuxDisplay;
		break;

	case STOP_COM:
		break;

	case ENC_UP:
        switch(controlMode)
        {
        case CONTROL_MODE_DUTY:
        	if(limDuty > 0.95)
        		limDuty = 0.95;
        	else
        		limDuty += 0.05;
            break;

        case CONTROL_MODE_CURRENT:
        	BKP_WriteBackupRegister(REG_SET_LIM_CURR,
        	        BKP_ReadBackupRegister(REG_SET_LIM_CURR) > 100 ? 100 : BKP_ReadBackupRegister(REG_SET_LIM_CURR) + 2);
            break;

        default:
            break;
        }
		break;

	case ENC_DOWN:
        switch(controlMode)
        {
        case CONTROL_MODE_DUTY:
        	if(limDuty < 0.10)
        		limDuty = 0.10;
        	else
        		limDuty -= 0.10;
            break;

        case CONTROL_MODE_CURRENT:
        	BKP_WriteBackupRegister(REG_SET_LIM_CURR,
        	        BKP_ReadBackupRegister(REG_SET_LIM_CURR) < 10 ? 10 : BKP_ReadBackupRegister(REG_SET_LIM_CURR) - 2);
            break;

        default:
            break;
        }
		break;

	case ENC_BTN:
		if(controlMode != CONTROL_MODE_CURRENT)
			controlMode = CONTROL_MODE_CURRENT;
		else
			controlMode = CONTROL_MODE_DUTY;
		break;

	case ENC_BTN_LONG:
		MenuChange(&MenuDisplay);
		break;
	}

	return 0;
}


uint8_t auxScreen(uint16_t code)
{
	LCD_setFont(font5x8);

	uint8_t i = 0;

	LCD_String(0, 80-i++*9, "Vbat", 1);
	LCD_Num(0, 80-i++*9, GetVBatMV(), 1);

	LCD_String(0, 80-i++*9, "PwrOn", 1);
	uint16_t num = BKP_ReadBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant);
	uint32_t minutes = num / 2;
	LCD_textNumText(0, 80-i++*9, "", minutes % 60 + (minutes / 60) * 100, (num % 2) == 1 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators

	LCD_String(0, 80-i++*9, "Rests", 1);
	LCD_Num(0, 80-i++*9, nrfResets, 1);

	LCD_String(0, 80-i++*9, "mahKM", 1);
	LCD_Float(0, 80-i++*9, (float)BKP_ReadBackupRegister(REG_ENERGY_mah) / (float)BKP_ReadBackupRegister(REG_ODO_100m) * 10.0, 1);

	LCD_String(0, 80-i++*9, "Charg", 1);


	switch(code)
	{
	case START_COM:
		CurrScreen = &AuxDisplay;
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		MenuChange(NEXT);
		return 0;
		break;

	case ENC_BTN_LONG:
		MenuChange(&MenuDisplay);
		break;
	}

	return 0;
}


uint8_t graphScreen(uint16_t code)
{
#define GR_LIM	(90)
	LCD_setFont(font3x5);
	static uint8_t graphMode = 2;
	static int16_t graphArray[GR_LIM];
	static uint8_t graphPtrFullFlag = 0;
	static uint8_t graphPtrEnd = 0;

	if(++graphPtrEnd >= GR_LIM)
	{
		graphPtrEnd = 0;
		graphPtrFullFlag = true;
	}


	if(graphMode == 0)
		graphArray[graphPtrEnd] = currentState.currentBat;
	else if(graphMode == 1)
		graphArray[graphPtrEnd] = abs(currentState.speed);
	else if(graphMode == 2)
		graphArray[graphPtrEnd] = currentState.power;

	switch(code)
	{
	case START_COM:
		graphPtrEnd = 0;
		graphPtrFullFlag = 0;
		CurrScreen = &GraphDisplay;
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		MenuChange(NEXT);
		return 0;
		break;

	case ENC_BTN_LONG:
		MenuChange(&MenuDisplay);
		break;

	case ENC_UP:
		if(++graphMode >= 3)
			graphMode = 0;
		graphPtrEnd = 0;
		graphPtrFullFlag = 0;
		break;

	case ENC_DOWN:
		if(--graphMode >= 3)
			graphMode = 2;
		graphPtrEnd = 0;
		graphPtrFullFlag = 0;
		break;
	}

	LCD_HLine(0,1,33,1);

	for(uint8_t i=0; i<4; i++)
		for(uint8_t j=0; j<10; j++)
			LCD_Pixel(31-i*10, j*10+1, 1);

	int16_t max=-32767, min=32767;
	for(uint8_t i = 0; i < (graphPtrFullFlag ? sizeof(graphArray) / sizeof(graphArray[0]) : graphPtrEnd); i++)
	{
		if(graphArray[i] < min)
			min = graphArray[i];
		if(graphArray[i] > max)
			max = graphArray[i];

		if(max < 0 && min < 0)
			max = 0;

		if(max > 0 && min > 0)
			min = 0;
	}

	if(graphPtrFullFlag)
		for(uint8_t j = 0, i = graphPtrEnd == (GR_LIM-1) ? 0 : graphPtrEnd+1; ; j++)
		{
			if(++i >= GR_LIM)
			{
				i = 0;
			}
			if(i == graphPtrEnd)
				break;
			LCD_Pixel(31-map(graphArray[i], min, max, 0, 31), j, 1);
		}
	else
		for(uint8_t i = 0; i < graphPtrEnd; i++)
		{
			LCD_Pixel(31-map(graphArray[i], min, max, 0, 31), i, 1);
		}

	LCD_VLine(31-map(0, min, max, 0, 31) ,0,101-6,1); //zero line

	if(graphMode == 0)
		LCD_NumWDesc(0, 101 - 5, "C ", max > abs(min) ? max : -min);
	else if(graphMode == 1)
		LCD_NumWDesc(0, 101 - 5, "S ", max > abs(min) ? max : -min);
	else if(graphMode == 2)
		LCD_NumWDesc(0, 101 - 5, "P ", max > abs(min) ? max : -min);

	return 0;
}


uint8_t mainScreen(uint16_t code)
{
	switch(code)
	{
	case START_COM:
		CurrScreen = &MainDisplay;
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		MenuChange(NEXT);
		return 0;
		break;

	case ENC_BTN_LONG:
		MenuChange(&MenuDisplay);
		break;

	case ENC_UP:
		break;

	case ENC_DOWN:
		break;
	}

	// Joystick battery
	LCD_setFont(font5x8);
	int16_t vBat = GetVBatMV() - 3450;

	if(vBat < 150/*mV*/)
	{
		LCD_Rectangle(0, 97, 31, 3, 1);
		LCD_Rectangle(31, 98, 1, 1, 1);
		if(((sysTickerMs % 600) > 300))
		{
			LCD_RectangleInvert(0,97,33,21);
		}
	}
	else
	{
		LCD_Rectangle(0, 97, 31, 3, 1);
		LCD_Rectangle(31, 98, 1, 1, 1);
		LCD_RectangleFill(1, 98, (vBat < 0) ? 1 : (vBat / 24), 2, 1);
	}

	// Speed
	LCD_textNumText(1 + ((abs(currentState.speed) / 10) < 10 ? 11 : 0), 80, "", abs(currentState.speed) / 10, "", 2);

	// Speed (maximum)
	LCD_textNumText(22 + ((currentState.speedMax / 10) < 10 ? 6 : 0), 88, "", currentState.speedMax / 10, "", 1);

	// Energy total (mah)
	LCD_textNumText(1, 73, "", currentState.mahBattery, "\xB2", 1);

	// Power (watts)
	LCD_textNumText(1, 64, "", currentState.power, "\xB3", 1);
	//LCD_textNumText(20, 64, (thrLevel > 10) ? "8":"_", currentState.packetTxSucc%10, "", 1);

	//battery voltage
	//252 - 198(3.3) - 210(3.5) - 222(3.7)
	//210 - 165()
	int16_t vBat2 = currentState.voltBat-165;

#define VBATMAINOS 59
	if(vBat < 10/*mV*/)
	{
		LCD_Rectangle(0, VBATMAINOS, 31, 20, 1);
		LCD_Rectangle(31, VBATMAINOS+1, 1, 18, 1);
		if(((sysTickerMs % 600) > 300))
			LCD_RectangleInvert(0,VBATMAINOS,33,21);
	}
	else
	{
		LCD_Rectangle(0, VBATMAINOS, 31, 3, 1);
		LCD_Rectangle(31, VBATMAINOS+1, 1, 1, 1);
		LCD_RectangleFill(1, VBATMAINOS + 1, (vBat2 < 0) ? 1 : (vBat2 * 17 / 25), 2, 1);
	}

	LCD_setFont(font3x5);
	LCD_FloatPseudoWString(1, 53, currentState.voltBat*2.0, "", 2, 1);
	LCD_textNumText(20, 53, "", currentState.currentBat, "A", 1);

	// connection quality
	LCD_setFont(font3x5);
	float lostRat = (float) (1000 * currentState.packetTxFail / currentState.packetTxSucc) / 10.0;
	if(lostRat > 99.9)
	{
		lostRat = 99.9;
	}

	LCD_Float(0, 46, lostRat, 1);

	LCD_Num(19, 46, currentState.RxLatency, 1);
	LCD_RectangleInvert(16, 45, 33 - 16, 5 + 2);
	if(currentState.packetTransmitStatus != 0)
	{
		LCD_RectangleInvert(0, 45, 16, 5 + 2);
	}

	// odometer
	LCD_setFont(font5x8);
	LCD_Float2Str(1, 36, "", (float)currentState.odometer*0.01, "\xb6", currentState.odometer >= 1000 ? 1 : 2, 1);

	LCD_setFont(font5x8);
	LCD_textNumText(1, 28, "=", currentState.odometerTotal/100, "\xb6", 1);

	if(cModeNow == CMODE_DIST_LIM)
	{
		LCD_FloatPseudoW2String(1, 18, "D", limiterDistCounter, "", 1, 1);
	}
	else if(cModeNow == CMODE_ENERGY_LIM)
	{
		LCD_textNumText(1, 18, "E", limiterEnergyCounter, "", 1);
	}
	else if(cModeNow == CMODE_TIME_LIM)
	{
		LCD_textNumText(1, 18, "T", (limiterTimeCounter / 60) * 100 + limiterTimeCounter % 60, "", 1);
	}

	LCD_setFont(font3x5);
	LCD_Num(1, 22, (int)(currentState.mahBattery * 100.0 / (float)currentState.odometer), 1);
	LCD_Float(8, 16, ((4500.0-currentState.mahBattery)/(currentState.mahBattery * 100.0 / (float)currentState.odometer)), 1);

	// times
	switch((sysTickerMs%6000)/3000)
	{
		// time of the moving
		case 0:
		{
		uint16_t num = upTimeS / 60;
		LCD_textNumText(1, 9, "M", num % 60 + (num / 60) * 100, (upTimeS % 60) > 30 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators
		}
		break;

		// time of the uptime (board)
		case 1:
		{
		uint16_t num = upTimeS / 60;
		LCD_textNumText(1, 9, "S", num % 60 + (num / 60) * 100, (upTimeS % 60) > 30 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators
		}
		break;
	}

	//temperature
	LCD_setFont(font3x5);
	LCD_textNumText(0, 0,  "", currentState.tempMos, "", 1);

	// fault
	LCD_setFont(font5x8);
	switch(currentState.faultCode)
	{
#define W_OS 22
	case FAULT_CODE_NONE:
		LCD_String(W_OS, 0, "OK", 1);
		break;
	case FAULT_CODE_ABS_OVER_CURRENT:
		LCD_String(W_OS, 0, "OC", 1);
		break;
	case FAULT_CODE_DRV8302:
		LCD_String(W_OS, 0, "DR", 1);
		break;
	case FAULT_CODE_OVER_TEMP_FET:
		LCD_String(W_OS, 0, "MF", 1);
		break;
	case FAULT_CODE_OVER_TEMP_MOTOR:
		LCD_String(W_OS, 0, "MR", 1);
		break;
	case FAULT_CODE_OVER_VOLTAGE:
		LCD_String(W_OS, 0, "OV", 1);
		break;
	case FAULT_CODE_UNDER_VOLTAGE:
		LCD_String(W_OS, 0, "UV", 1);
		break;
	}

	//fault code inversion
	if(currentState.faultCode != FAULT_CODE_NONE && ((sysTickerMs % 600) > 300))
		LCD_RectangleInvert(W_OS-1, 0, 33 - (W_OS-1), 6);

	return 0;
}


uint8_t chargeScreen(uint16_t code)
{
	shutdownDelay = SHDN_DELAY_MS;

	LCD_StringCentered(0, 64, "CHRG", 1);

	LCD_FloatPseudoWString(2, 40, GetVBatMV()/10,  "v", 2, 1);

	int16_t vBat = GetVBatMV() - 3450;

	//reset joystick uptime if charged
	if(GetVBatMV() > 4000)
		BKP_WriteBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant, 0);

	LCD_Rectangle(0, 10, 30, 18, 1);
	LCD_Rectangle(1, 11, 28, 16, 1);
	LCD_Rectangle(30, 15, 2, 9, 1);
	LCD_Rectangle(31, 15, 1, 9, 1);

	//[2;28]
	LCD_RectangleFill(2, 12, (vBat < 0) ? 2 : (vBat / 31), 15, 1);

	static uint8_t fillCnt = 0;
	fillCnt+=2;
	if(fillCnt > 32-5)
		fillCnt = 0;

	LCD_RectangleFill(fillCnt, 2, 5, 3, 1);
	LCD_Rectangle(0, 1, 31, 4, 1);

	BKP_WriteBackupRegister(REG_JOYSTICK_POWERON_TIME_30secQuant, 0);
	upTimeS = 0;

	return 0;
}
