/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
enum WorkState{STANDALONE,	PC_MONITORING, PC_CONTROLS};

#define BULK_MAX_PACKET_SIZE  0x00000040


/* Exported functions ------------------------------------------------------- */
void Initialization();
void Set_System(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config(FunctionalState NewState);
void Get_SerialNum(void);

void CUBECheckState(void);
void CUBECheckState2(void);


/* External variables --------------------------------------------------------*/

#endif  /*__HW_CONFIG_H*/
