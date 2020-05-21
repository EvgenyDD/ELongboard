/* Includes ------------------------------------------------------------------*/
#include "UI.h"
#include "LCD.h"
#include "interface.h"
#include "hw_hal.h"
#include "nRF24L01.h"
#include "stm32f10x_pwr.h"
#include "misc.h"
#include "sound.h"


/* Private typedef -----------------------------------------------------------*/
//typedef enum{
//	MenuMenuMain = 0,
//	MenuMain,
//	MenuHUD,
//	//MenuParams,
//	MenuSetPID,
//	MenuDispCtrlSett
//}MENU_TYPE;

#define LAST_MENU_ID MenuDispCtrlSett


MenuItem Null_Menu =
{ (void*) 0, (void*) 0, (void*) 0, (void*) 0, /*NULL_FUNC,*/NULL_FUNC /*,NULL_FUNC*/, ""};

M_M(Menu,       	NULL_MENU,	NULL_MENU,	NULL_MENU,NULL_MENU, menuScreen, "Menu");
M_M(Settings,       NULL_MENU,	NULL_MENU,	NULL_MENU,NULL_MENU, menuScreen, "Stngs");


M_M(MainDisplay,  	AuxDisplay,	AuxDisplay,	NULL_MENU,NULL_MENU, MainScreen, "");
M_M(AuxDisplay,   	MainDisplay,MainDisplay,NULL_MENU,NULL_MENU, AuxScreen, "");
M_M(ChargeDisplay, 	NULL_MENU,	NULL_MENU,	NULL_MENU,NULL_MENU, ChargeScreen, "");


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//MENU_TYPE screenMode = MenuMain;

MenuItem*	CurrWorkItem = &MainDisplay;
MenuItem*   CurrScreen;

uint16_t btnDelay[2] = {0,0};



/* Extern variables ----------------------------------------------------------*/
extern int16_t thrLevel, brkLevel;
extern InterfaceStatusTypedef currentState;

extern const unsigned char font3x5[], font5x8[];
extern uint32_t sysTickerMs;

extern uint32_t thrStart, thrEnd, thrCorrel;
extern uint32_t brkStart, brkEnd, brkCorrel;

extern uint8_t cntcnt;
extern uint8_t invertMode;

extern mc_control_mode controlMode;

extern float limDuty, limCurr, limSpeed, limSmartSpeed;

extern uint32_t shutdownDelay;
extern uint32_t soundDelay;

extern uint32_t upTimeMs;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MenuDispatcher
*******************************************************************************/
void MenuDispatcher(uint16_t code)
{
	if(code)
		shutdownDelay = SHDN_DELAY_MS;

	LCD_Clear();

	if(*CurrWorkItem->caption != '\n')
	{
		LCD_setFont(font5x8);
		LCD_StringCentered(0, 101-9, CurrWorkItem->caption, 1);
	}

	if(CurrWorkItem->Callback!=NULL_FUNC)
		CurrWorkItem->Callback(code);

	LCD_Update();
}


void MenuChange(MenuItem *NextWorkItem)
{
//	if(CurrWorkItem->Callback!=NULL_FUNC)
//		CurrWorkItem->Callback(STOP_COM);

	CurrWorkItem = NextWorkItem;

//	if(CurrWorkItem->Callback!=NULL_FUNC)
//		CurrWorkItem->Callback(START_COM);
}


/*******************************************************************************
* Function Name  : Inv
*******************************************************************************/
void Inv()
{
    if(invertMode)
        invertMode = 0;
    else
        invertMode = 1;
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
		BKP_WriteBackupRegister(REG_UPTIME_TOTAL_30secQuant, BKP_ReadBackupRegister(REG_UPTIME_TOTAL_30secQuant) + 1);
		BKP_WriteBackupRegister(REG_UPTIME_SINCE_CHRG_30secQuant, BKP_ReadBackupRegister(REG_UPTIME_SINCE_CHRG_30secQuant) + 1);
	}
}


/*******************************************************************************
* Function Name  : ProcessEncButton
*******************************************************************************/
void ProcessEncButton()
{
    static uint8_t lastButtonState = 0;

    if(shutdownDelay == 0)
    	ShutDown();

	if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1))
	{
		if(lastButtonState == 0)
		{
			lastButtonState = 1;
			btnDelay[0] = 1;
		}
		else
		{
			if(btnDelay[0] > 1000)
			{
            	Beep(4000, 250);

            	MenuDispatcher(ENC_BTN_LONG);

                btnDelay[0] = 0;
                lastButtonState = 0;
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

    static uint8_t lastStateRed = 0;
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
    {
        if(lastStateRed == 0)
        {
        	lastStateRed = 1;
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

//				NVIC_InitTypeDef NVIC_InitStructure;
//				NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//				NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
//				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
//				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x07;
//				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//				NVIC_Init(&NVIC_InitStructure);

				ShutDown();

				btnDelay[0] = 0;
				lastStateRed = 0;
			}
        }
    }
    else
    {
    	if(lastStateRed == 1)
		{
    		Beep(2000, 250);//PlayStarWars();//
            Inv();
			btnDelay[1] = 0;
		}

        lastStateRed = 0;
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
	NRF_Sleep();
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

//        switch(controlMode)
//        {
//        case CONTROL_MODE_DUTY:
//            limDuty += 0.05;
//            if(limDuty > 0.95)
//                limDuty = 0.95;
//            break;
//
//        case CONTROL_MODE_SPEED:
//            limSpeed += 0.2;
//            if(limSpeed > 30.0)
//                limSpeed = 30.0;
//            break;
//
//        case CONTROL_MODE_CURRENT:
//            limCurr += 2;
//            if(limCurr > 60.0)
//                limCurr = 60.0;
//            break;
//
//        case CONTROL_MODE_SMART_SPEED:
//        	smartIncrementValue(&limSmartSpeed);
//        	break;
//
//        default:
//            break;
//        }
    }
    else if(prevEnc > TIM4->CNT/2)
    {
    	Beep(4000, 20);

    	MenuDispatcher(ENC_DOWN);
//        switch(controlMode)
//        {
//        case CONTROL_MODE_DUTY:
//            limDuty -= 0.05;
//            if(limDuty < 0.10)
//                limDuty = 0.1;
//            break;
//
//        case CONTROL_MODE_SPEED:
//            limSpeed -= 0.2;
//            if(limSpeed < 2.0)
//                limSpeed = 2.0;
//            break;
//
//        case CONTROL_MODE_CURRENT:
//            limCurr -= 2;
//            if(limCurr < 1.0)
//                limCurr = 0.1;
//            break;
//
//        case CONTROL_MODE_SMART_SPEED:
//        	smartDecrementValue(&limSmartSpeed);
//        	break;
//
//        default:
//            break;
//        }
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


void MenuDispatcher2()
{
	LCD_setFont(font5x8);

	static mc_fault_code prevCode;
	if(prevCode == FAULT_CODE_NONE && currentState.faultCode != FAULT_CODE_NONE)
		HapticBuzz(500);
	prevCode = currentState.faultCode;

	if(IsCharging())
	{
		shutdownDelay = SHDN_DELAY_MS;

		LCD_StringCentered(0, 64, "CHRG", 1);

		LCD_FloatPseudoWString(2, 40, GetVBatMV()/10,  "v", 2, 1);

		int16_t vBat = GetVBatMV() - 3450;

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

		BKP_WriteBackupRegister(REG_UPTIME_SINCE_CHRG_30secQuant, 0);
		upTimeMs = 0;
	}
	else
	{
		//switch(screenMode)
		{
	//		break;


			//case MenuHUD:
				LCD_setFont(font3x5);
				LCD_Rectangle(8, 94, 32 - 8, 6, 1);
				LCD_RectangleFill(8, 94, (thrLevel > 99 ? 99 : thrLevel) * (33 - 8) / 100, 6, 1);
				LCD_Num(0, 95, (thrLevel > 99 ? 99 : thrLevel), 1);

				LCD_Rectangle(8, 94 - 8, 32 - 8, 6, 1);
				LCD_RectangleFill(8, 94 - 8, (brkLevel > 99 ? 99 : brkLevel) * (33 - 8) / 100, 6, 1);
				LCD_Num(0, 95 - 8, (brkLevel > 99 ? 99 : brkLevel), 1);

				if(currentState.speed < 0)
					currentState.speed *= -1;

				LCD_setFont(font5x8);
				LCD_Num(2 + (currentState.speed / 10 < 10 ? 11 : 0), 69, currentState.speed / 10, 2);
				LCD_Num(2 + 24, 71, (int32_t)currentState.speed % 10, 1);

				if(invertMode)
					LCD_RectangleInvert(0, 70, 33, 16);

				//currents
				LCD_setFont(font3x5);
				LCD_FloatPseudoWString(1, 64, currentState.currentMtr, " A", 1, 1);
				LCD_FloatPseudoWString(1, 57, currentState.currentBat, " A", 1, 1);
				LCD_RectangleInvert(0, 56, 33, 5 + 2);

				//battery voltage
				LCD_setFont(font5x8);
				LCD_FloatPseudoWString(0, 47, currentState.voltBat, "V", 1, 1);

				//up/down
				LCD_setFont(font3x5);
				LCD_NumWDesc(0, 42, "\xB1", /*currentState.downCapacity*/GetVBatMV());	//mah
				LCD_NumWDesc(0, 36, "\xB0", currentState.capacityChrg);	//mah

				//rx/tx quality
				float lostRatio = (float) (1000 * currentState.packetTxFail / currentState.packetTxSucc) / 10.0;
				if(lostRatio > 99.9)
					lostRatio = 99.9;

				LCD_Float(0, 30, lostRatio, 1);

				LCD_Num(19, 30, currentState.RxLatency, 1);
				LCD_RectangleInvert(16, 29, 33 - 16, 5 + 2);

				//			LCD_NumWDesc(0, 50, "L", currentState.TxLatency);

				if(currentState.packetTransmitStatus == NRF_Transmit_Status_Lost)
					LCD_RectangleInvert(0, 29, 16, 5 + 2);

				LCD_setFont(font5x8);
				LCD_FloatPseudoWString(1, 20, currentState.power, "W", 0, 1);
				LCD_RectangleInvert(0, 20, 33, 9);

				LCD_setFont(font3x5);

				switch(controlMode)
				{
				case CONTROL_MODE_DUTY:
					LCD_String(0, 14, "Duty", 1);
					LCD_Float(0, 8, limDuty, 2);
					break;

				case CONTROL_MODE_SPEED:
					LCD_String(0, 14, "Speed", 1);
					LCD_Float(0, 8, limSpeed, 1);
					break;

				case CONTROL_MODE_CURRENT:
					LCD_String(0, 14, "Current", 1);
					LCD_Float(0, 8, limCurr, 2);
					break;

				case CONTROL_MODE_SMART_SPEED:
					LCD_String(0, 14, "SMART", 1);
					LCD_Float(0, 8, limSmartSpeed, 3);
					break;

				default:
					break;
				}

				switch(currentState.faultCode)
				{
				case FAULT_CODE_NONE:
					LCD_String(25, 0, "OK", 1);
					break;
				case FAULT_CODE_ABS_OVER_CURRENT:
					LCD_String(25, 0, "OC", 1);
					break;
				case FAULT_CODE_DRV8302:
					LCD_String(25, 0, "DR", 1);
					break;
				case FAULT_CODE_OVER_TEMP_FET:
					LCD_String(25, 0, "MF", 1);
					break;
				case FAULT_CODE_OVER_TEMP_MOTOR:
					LCD_String(25, 0, "MR", 1);
					break;
				case FAULT_CODE_OVER_VOLTAGE:
					LCD_String(25, 0, "OV", 1);
					break;
				case FAULT_CODE_UNDER_VOLTAGE:
					LCD_String(25, 0, "UV", 1);
					break;
				}
				//if(currentState.faultCode != FAULT_CODE_NONE && ((sysTickerMs % 600) > 300))
				if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0) != 0)
					LCD_RectangleInvert(24, 0, 33 - 24, 6);

				LCD_Celsium(0, 0,  "", currentState.tempMos, 1);
				LCD_Celsium(12, 0,  "", currentState.tempMotor, 1);
		//	break;
//
//		case MenuParams:
//			LCD_setFont(font5x8);
//			LCD_FloatPseudoWString(0, 60, currentState.voltCell[5],  "V5", 2, 1);
//			LCD_FloatPseudoWString(0, 50, currentState.voltCell[4],  "V4", 2, 1);
//			LCD_FloatPseudoWString(0, 40, currentState.voltCell[3],  "V3", 2, 1);
//			LCD_FloatPseudoWString(0, 30, currentState.voltCell[2],  "V2", 2, 1);
//			LCD_FloatPseudoWString(0, 20, currentState.voltCell[1],  "V1", 2, 1);
//			LCD_FloatPseudoWString(0, 10, currentState.voltCell[0],  "V0", 2, 1);
//			LCD_Num(0, 0, cntcnt, 1);
//			break;

		//case MenuSetPID:
			LCD_setFont(font3x5);
			for(uint8_t i = 0; i < 9 || i < currentState.len; i++)
			{
				int ii = i / 2;
				LCD_Num(0, ii * 6, currentState.datas[i], 1);
				LCD_Num(16, ii * 6, currentState.datas[++i], 1);
			}

			LCD_setFont(font5x8);
			LCD_Num(0, 90, currentState.len, 1);

			LCD_Num(16, 90, currentState.numxxx, 1);

			//break;

		//case MenuDispCtrlSett:
			LCD_Num(1, 80, thrStart, 1);
			LCD_Num(1, 70, thrEnd, 1);
			LCD_Num(1, 60, thrCorrel, 1);
			LCD_Num(1, 50, thrLevel, 1);
			LCD_Num(20, 50, thrLevel * brkCorrel / 100, 1);

			LCD_Celsium(2, 40,  "t=", GetTemperatureChip(), 1);

			LCD_Num(1, 30, brkStart, 1);
			LCD_Num(1, 20, brkEnd, 1);
			LCD_Num(1, 10, brkCorrel, 1);
			LCD_Num(1, 0, brkLevel, 1);
			LCD_Num(20, 0, brkLevel * thrCorrel / 100, 1);
		//	break;
		}
	}
}

uint8_t menuScreen(uint16_t code)
{
	static uint8_t curPos = 0;

	LCD_setFont(font5x8);

	uint8_t i = 0;
	LCD_String(0, 80-i++*9, "Reset", 1);
	LCD_String(0, 80-i++*9, "RsSpd", 1);
	LCD_String(0, 80-i++*9, "Cntrl", 1);
	LCD_String(0, 80-i++*9, "Msc1", 1);
	LCD_String(0, 80-i++*9, "Set", 1);


	switch(code)
	{
	case START_COM:
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		switch(curPos)
		{
		case 0: //reset all
			break;

		case 1: //reset speed
			break;

		case 2: //change ctrl type
			break;

		case 3: //play music 1
			break;

		case 4: //settings
			MenuChange(&Settings);
			break;

		}

		break;

	case ENC_BTN_LONG:
		MenuChange(CurrScreen);
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

uint8_t MainScreen(uint16_t code)
{
	switch(code)
	{
	case START_COM:
		break;

	case STOP_COM:
		break;

	case ENC_BTN:
		MenuChange(&Menu);
		CurrScreen = &MainDisplay;
		return 0;
		break;

	case ENC_BTN_LONG:
		break;

	case ENC_UP:
		break;

	case ENC_DOWN:
		break;
	}


	LCD_setFont(font5x8);

	// battery
	int16_t vBat = GetVBatMV() - 3450;

	if(vBat < 150/*mV*/)
	{
		LCD_Rectangle(0, 80, 31, 20, 1);
		LCD_Rectangle(31, 81, 1, 18, 1);
		if(((sysTickerMs % 600) > 300))
			LCD_RectangleInvert(0,80,33,21);
	}
	else
	{
		LCD_Rectangle(0, 97, 31, 3, 1);
		LCD_Rectangle(31, 98, 1, 1, 1);
		LCD_RectangleFill(1, 98, (vBat < 0) ? 1 : (vBat / 24), 2, 1);
	}

	// speed
	if(currentState.speed < 0)
		currentState.speed *= -1;

	LCD_textNumText(1 + ((currentState.speed / 10) < 10 ? 11 : 0), 80, "", currentState.speed / 10, "", 2);

	// avg speed 10 sec
	LCD_textNumText(22 + ((currentState.speed / 10) < 10 ? 6 : 0), 88, "", currentState.speedAvg10sec / 10, "", 1);

	// mah total
	LCD_textNumText(1, 73, "", currentState.mahBattery, "\xB2", 1);

	// Watt //avg 10 sec?
	LCD_textNumText(1, 64, "", currentState.power, "\xB3", 1);
	LCD_textNumText(20, 64, (thrLevel > 10) ? "8":"_", currentState.packetTxSucc%10, "", 1);

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
	LCD_setFont(font5x8);


	// connection quality
	//rx/tx quality
	LCD_setFont(font3x5);
	float lostRat = (float) (1000 * currentState.packetTxFail / currentState.packetTxSucc) / 10.0;
	if(lostRat > 99.9)
		lostRat = 99.9;

	LCD_Float(0, 46, lostRat, 1);

	LCD_Num(19, 46, currentState.RxLatency, 1);
	LCD_RectangleInvert(16, 45, 33 - 16, 5 + 2);
	if(currentState.packetTransmitStatus == NRF_Transmit_Status_Lost)
		LCD_RectangleInvert(0, 45, 16, 5 + 2);


	LCD_setFont(font5x8);

	//odometers
	LCD_FloatPseudoWString(1, 36, currentState.odometer/100, "  \xb6", 0, 1);
	LCD_setFont(font3x5);
	LCD_Num(15, 37, currentState.odometer%100, 1);
	LCD_setFont(font5x8);
	LCD_textNumText(1, 28, "=", currentState.odometerTotal/100, "\xb6", 1);



	// time chrg
	switch((sysTickerMs%6000)/2000)
	{
	case 0:
	{
		uint16_t num = upTimeMs / 60;
		LCD_textNumText(1, 9, "J", num % 60 + (num / 60) * 100, (upTimeMs % 60) > 30 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators
	}
		break;

	case 1:
	{
		uint16_t num = upTimeMs / 60;
		LCD_textNumText(1, 9, "C", num % 60 + (num / 60) * 100, (upTimeMs % 60) > 30 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators
	}
		break;

	case 2:
	{
		uint16_t num = upTimeMs / 60;
		LCD_textNumText(1, 9, "T", num % 60 + (num / 60) * 100, (upTimeMs % 60) > 30 ? "\xB5" : "\xB4", 1);	//HHMM format. no separators
		LCD_RectangleInvert(1,8,31,10);
	}
		break;
	}

	//modes
	LCD_setFont(font5x8);

				switch(controlMode)
				{
				case CONTROL_MODE_DUTY:
					LCD_Float2Str(0, 18, "D", limDuty, "", 2, 1);
					break;

				case CONTROL_MODE_SPEED:
					LCD_Float2Str(0, 18, "S", limSpeed, "", 1, 1);
					break;

				case CONTROL_MODE_CURRENT:
					LCD_Float2Str(0, 18, "C", limCurr, "", 2, 1);
					break;

				case CONTROL_MODE_SMART_SPEED:
					LCD_Float2Str(0, 18, "E", limSmartSpeed, "", 2, 1);
					break;

				default:
					break;
				}


	// faults and temperatures
	LCD_setFont(font3x5);
	LCD_textNumText(0, 0,  "", currentState.tempMos, "", 1);

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

	if(currentState.faultCode != FAULT_CODE_NONE && ((sysTickerMs % 600) > 300))
		LCD_RectangleInvert(W_OS-1, 0, 33 - (W_OS-1), 6);

	return 0;
}

uint8_t AuxScreen(uint16_t code)
{
	return 0;
}

uint8_t ChargeScreen(uint16_t code)
{
	return 0;
}


