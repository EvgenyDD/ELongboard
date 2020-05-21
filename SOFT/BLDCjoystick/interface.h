/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACE_H
#define INTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/* Exported types ------------------------------------------------------------*/
typedef enum{
	COM_STATUS = 0,
	COM_SHDN,
	COM_TELEMETRY
}COM_ID_TYPE;

typedef enum {
	FAULT_CODE_NONE = 0,
	FAULT_CODE_OVER_VOLTAGE,
	FAULT_CODE_UNDER_VOLTAGE,
	FAULT_CODE_DRV8302,
	FAULT_CODE_ABS_OVER_CURRENT,
	FAULT_CODE_OVER_TEMP_FET,
	FAULT_CODE_OVER_TEMP_MOTOR
} mc_fault_code;

typedef enum {
    CONTROL_MODE_DUTY = 0,
    CONTROL_MODE_SPEED,
    CONTROL_MODE_CURRENT,
    CONTROL_MODE_CURRENT_BRAKE,
    CONTROL_MODE_POS,
    CONTROL_MODE_NONE,
    CONTROL_MODE_SMART_SPEED
} mc_control_mode;

typedef struct{
	float speed;
	float speedAvg10sec;

	float power;
	float powerAvg10sec;

	float currentBat;
	float currentMtr;

	mc_fault_code faultCode;

	float voltBat;

	uint8_t voltCell[6];

	int8_t tempMos;
	int8_t tempMotor;

	int16_t mahBattery;
	uint16_t capacityDisc;
	uint16_t capacityChrg;

	uint16_t odometer;
	uint16_t odometerTotal;


	//nrf24l01
	uint8_t packetTransmitStatus;
	uint32_t packetTxSucc;
	uint32_t packetTxFail;

	uint32_t TxLatency;
	uint32_t RxLatency;

	uint8_t datas[32];
	uint8_t len;
	uint8_t numxxx;


}InterfaceStatusTypedef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void InterfaceInit();
void InterfaceProcessInputPacket(uint8_t *data, uint16_t len);
void SendPowerOff();

void InterfaceCallback();


#endif //INTERFACE_H
