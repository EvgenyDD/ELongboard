/* Includes ------------------------------------------------------------------*/
#include "interface.h"
#include "buffer.h"

#include "hw_hal.h"
#include "circular.h"
#include "UI.h"

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

#include "nrf_driver.h"
#include "rf.h"
#include "rfhelp.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define abs(x)  (( (x)<0 ) ? (-(x)) : (x))


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
InterfaceStatusTypedef currentState;


/* Extern variables ----------------------------------------------------------*/
extern int16_t thrLevel, brkLevel;
extern uint32_t sysTickerMs;
extern uint32_t shutdownDelay;
extern mc_control_mode controlMode;

float limSpeed = 20.0;
float limDuty = 0.70;
uint8_t moveDirection = 0;

uint8_t cntcnt;

// Speed[km/h] = pi*Dwheel[km]*ERPM**12/30[gear ratio]*60[rpm->hour]    no (1/7[magnets/2])
#define KMH_TO_RPM (18.946)

/* Private function prototypes -----------------------------------------------*/
void delay_ms(uint32_t nTime);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : fmap
*******************************************************************************/
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
    float out = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    if(out>out_max)
        return out_max;
    if(out < out_min)
        return out_min;

    return out;
}


/*******************************************************************************
* Function Name  : InterfaceInit
*******************************************************************************/
void InterfaceInit()
{
	currentState.packetTxSucc = 1;

	nrf_driver_init();
	rfhelp_restart();
}

/*******************************************************************************
* Function Name  : InterfaceCallback
* Description    :
*******************************************************************************/
void InterfaceCallback()
{
	uint8_t dataOut[32];
	int32_t ind = 0;

	dataOut[ind++] = 0;

	if(shutdownDelay == 0)
		dataOut[0] |= COM_NRF_OFF;

	if(brkLevel > 10)
	{
		dataOut[0] |= COM_NRF_BRAKE;
		buffer_append_float32(dataOut, fmap(brkLevel, 0.0, 100.0, 0, 20/*A*/), 1000.0, &ind);
	}
	else if(thrLevel > 10)
	{
		if(moveDirection)
			dataOut[0] |= COM_NRF_INV;

		switch(controlMode)
		{
		case CONTROL_MODE_DUTY:
			dataOut[0] |= COM_NRF_DUTY;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limDuty), 1000.0, &ind);
			break;

		case CONTROL_MODE_SPEED:
			dataOut[0] |= COM_NRF_SPEED;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limSpeed * KMH_TO_RPM * 7 * 10), 1000.0, &ind);
			break;

		case CONTROL_MODE_CURRENT:
			dataOut[0] |= COM_NRF_CURR;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, (float)BKP_ReadBackupRegister(REG_SET_LIM_CURR)), 1000.0, &ind);
			break;

		default:
			buffer_append_float32(dataOut, 0.0, 1000.0, &ind);
			break;
		}
	}
	else
	{
		buffer_append_float32(dataOut, 0.0, 1000.0, &ind);
	}

	int res;
	uint8_t failCounter = 0;

	static uint32_t prev = 0;
	currentState.TxLatency = sysTickerMs - prev;
	prev = sysTickerMs;

again:
	res = rf_tx_wrapper(dataOut, ind);
	currentState.packetTransmitStatus = res;
	(res != 0) ? currentState.packetTxFail++ : currentState.packetTxSucc++;

	if(res == 0)
	{
		return;
	}

	if(++failCounter < 4)
	{
		delay_ms(2);
		goto again;
	}
}


/*******************************************************************************
* Function Name  : SendPowerOff
*******************************************************************************/
#define RETR 40
void SendPowerOff()
{
//	return;
	uint8_t dataOut[32];
	int32_t ind = 0;

	dataOut[ind++] = 0x00;

	dataOut[0] |= COM_NRF_OFF;

	buffer_append_float32(dataOut, 0.0, 1000.0, &ind);

	uint8_t failCounter = 0;
	int res;

again:
	res = rf_tx_wrapper(dataOut, ind);
	currentState.packetTransmitStatus = res;

	if(res == 0)
	{
		return;
	}

	if(++failCounter < RETR)
	{
		delay_ms(10);
	    goto again;
	}
}


/*******************************************************************************
* Function Name  : InterfaceProcessInputPacket
*******************************************************************************/
void InterfaceProcessInputPacket(uint8_t *data, uint16_t len)
{
	shutdownDelay = SHDN_DELAY_MS;

    for(int i=0; i<len; i++)
        currentState.datas[i] = data[i];

    currentState.len = len;

	COM_ID_TYPE command = data[0];

	data++;
	len--;
	int32_t ind = 0;

	switch(command)
    {
    case COM_STATUS:

        currentState.speed = (float)buffer_get_int16(data, &ind) / KMH_TO_RPM;
        if(abs(currentState.speed) > currentState.speedMax)
        {
        	currentState.speedMax = abs(currentState.speed);
        }

        currentState.currentMtr = (float)buffer_get_int16(data, &ind)*0.1;
        UTILS_LP_FAST(currentState.currentBat, (float)buffer_get_int16(data, &ind)*0.1, 0.3);

        currentState.voltBat = data[ind++];
        currentState.tempMos = (int8_t) data[ind++];
        currentState.faultCode = data[ind++];

        UTILS_LP_FAST(currentState.power, currentState.currentBat * currentState.voltBat / 10.0, 0.9);

        currentState.capacityDisc = buffer_get_uint16(data, &ind);
        currentState.capacityChrg = buffer_get_uint16(data, &ind);

		static int32_t prevEnergy = 0;
		int32_t totalEnergy = currentState.capacityDisc - currentState.capacityChrg;
		//if we've powered joystick after board have a small ride
		if(prevEnergy == 0 && totalEnergy > 1)
		{
			prevEnergy = totalEnergy;
		}
		if(prevEnergy != totalEnergy)
		{
			int16_t diff = totalEnergy - prevEnergy;
			BKP_WriteBackupRegister(REG_ENERGY_mah,
							BKP_ReadBackupRegister(REG_ENERGY_mah) + diff);
			prevEnergy = totalEnergy;
		}
		currentState.mahBattery = BKP_ReadBackupRegister(REG_ENERGY_mah);

        currentState.odometer = buffer_get_uint16(data, &ind);//10 meters
        static int32_t prevOdo = 0;
		if(prevOdo == 0 && currentState.odometer > 1)
		{
			prevOdo = currentState.odometer;
		}
        if(prevOdo != currentState.odometer)
        {
			BKP_WriteBackupRegister(REG_ODO_100m, BKP_ReadBackupRegister(REG_ODO_100m) + currentState.odometer - prevOdo);
        	prevOdo = currentState.odometer;
        }
        currentState.odometerTotal = BKP_ReadBackupRegister(REG_ODO_100m);
        break;

    case COM_SHDN:
    	shutdownDelay = 0;
    	break;

	case COM_TELEMETRY:
		break;

	default:
		break;
	}
}
