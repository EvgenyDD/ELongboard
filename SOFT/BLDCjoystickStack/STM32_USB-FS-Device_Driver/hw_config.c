/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_it.h"
#include "misc.h"
#include "stm32f10x_flash.h"

#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];
extern uint32_t count_in;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
extern void assert_failed(const char* file, uint32_t line);


/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Initialization.
* Description    : Initialize the chip peripheral.
*******************************************************************************/
void Initialization()
{
//RCC Init
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)
	{
		// Enable Prefetch Buffer
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		// Flash 2 wait state
		FLASH_SetLatency(FLASH_Latency_2);

		// HCLK = SYSCLK
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		// PCLK2 = HCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);
		// PCLK1 = HCLK /2
		RCC_PCLK1Config(RCC_HCLK_Div2);
		// ADC Clock
		RCC_ADCCLKConfig(RCC_PCLK2_Div8);
		// PLL Clock
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);//9

		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08);
	}

//System Clock = (72Mhz) / 72000 = 1000Hz = 1ms reload
	SysTick_Config(72000);

//Peripheral clocking
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 |
			RCC_APB2Periph_USART1 | RCC_APB2Periph_ADC1 |RCC_APB2Periph_AFIO, ENABLE );

//	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

//I/O
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 |
//								GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_15 | GPIO_Pin_5 | GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//USB Init
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

}


/*******************************************************************************
* Function Name : CUBECheckState.
* Description   : Decodes the CUBE state.
* Return value  : The state value.
*******************************************************************************/
//void CUBECheckState(void)
//{
//	static uint8_t packetType = 4;
//
//#if 1
//	switch(packetType)
//	{
//	case 4:
//		packetType = 5;
//		Buffer[0] = 4;
//		for(uint8_t i=0; i<32; i++) {Buffer[i+1] = image[i];}
//		USB_SIL_Write(EP1_IN, Buffer, RPT4_COUNT+1);
//		break;
//	case 5:
//		packetType = 6;
//		Buffer[0] = 5;
//		for(uint8_t i=0; i<32; i++) {Buffer[i+1] = image[i+32];}
//		USB_SIL_Write(EP1_IN, Buffer, RPT5_COUNT+1);
//		break;
//	case 6:
//		packetType = 4;
//		Buffer[0] = 6;
//		Buffer[1] = buttonsState;
//		Buffer[2] = Buffer[3] = 0;
//		USB_SIL_Write(EP1_IN, Buffer, RPT6_COUNT+1);
//		break;
//	}
//
//
//#else
//	Buffer[0] = 4;
//	for(uint8_t i=0; i<32; i++)
//	{
//		if(i<15)	Buffer[i+1] = 0;
//		else Buffer[i+1] = 255;
//	}
//			USB_SIL_Write(EP1_IN, Buffer, RPT4_COUNT+1);
//#endif
//   /* Buffer[0] = 4;//4;
//    Buffer[1] = 255;//btn1;
//    Buffer[2] = 13;//btn2;
//    Buffer[3] = 150;//Matrix[0];
//    Buffer[4] = 49;
//    Buffer[5] = 48;
//    for(uint8_t i = 1; i<64; i++) Buffer[i] = i;*/
//
//    //for(uint8_t k=0; k<5; k++)Buffer[k] = k+10;
//
//
//	//USB_SIL_Write(EP1_IN, Buffer, RPT4_COUNT+1); /* Copy mouse position info in ENDP1 Tx Packet Memory Area*/
//	SetEPTxValid(ENDP1);	/* Enable endpoint for transmission */
//
//}
//
//void CUBECheckState2(void)
//{
//	uint8_t buff[21] = {05,56,57,58,100,101,102};
//
//	/* Copy mouse position info in ENDP1 Tx Packet Memory Area*/
//	USB_SIL_Write(EP1_IN, buff, 32+1);
//	/* Enable endpoint for transmission */
//	SetEPTxValid(ENDP1);
//
//}

/*******************************************************************************
 * Description    : Configures Main system clocks & power
 *******************************************************************************/
void Set_System(void){}


/*******************************************************************************
 * Description    : Configures USB Clock input (48MHz)
 *******************************************************************************/
void Set_USBClock(void)
{
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);

	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}


/*******************************************************************************
 * Description    : Power-off system clocks and power while entering suspend mode
 *******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}


/*******************************************************************************
 * Description    : Restores system clocks and power while exiting suspend mode
 *******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}
}


/*******************************************************************************
 * Description    : Configures the USB interrupts
 *******************************************************************************/
void USB_Interrupts_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
 * Description    : Software Connection/Disconnection of USB Cable
 *******************************************************************************/
void USB_Cable_Config(FunctionalState NewState){}


/*******************************************************************************
 * Description    : Convert Hex 32Bits value into char.
 *******************************************************************************/
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++)
	{
		if (((value >> 28)) < 0xA)
		{
			pbuf[2 * idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}


/*******************************************************************************
 * Description    : Create the serial number string descriptor.
 *******************************************************************************/
void Get_SerialNum(void)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(volatile uint32_t*) (0x1FFFF7E8);
	Device_Serial1 = *(volatile uint32_t*) (0x1FFFF7EC);
	Device_Serial2 = *(volatile uint32_t*) (0x1FFFF7F0);

	Device_Serial0 += Device_Serial2;

	if (Device_Serial0 != 0)
	{
		IntToUnicode(Device_Serial0, &Virtual_Com_Port_StringSerial[2], 8);
		IntToUnicode(Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
	}
}









//=============================================================================


/*******************************************************************************
 * Function Name  : UART_To_USB_Send_Data.
 * Description    : send the received data from UART 0 to USB.
 * Input          : None.
 * Return         : none.
 *******************************************************************************/
void USART_To_USB_Send_Data(void)
{
	buffer_in[count_in] = 0;//USART_ReceiveData(USART1);

	count_in++;
	UserToPMABufferCopy(buffer_in, ENDP1_TXADDR, count_in);
	SetEPTxCount(ENDP1, count_in);
	SetEPTxValid(ENDP1);
}



void To_USB_Send_Str(unsigned char* str, unsigned char size)
{
	unsigned count = 0, i;

	do
	{
		if(size <= 60)
			count = size;
		else
		{
			count = 60;
		}

		UserToPMABufferCopy(str, ENDP1_TXADDR, count);
		SetEPTxCount(ENDP1, count);
		SetEPTxValid(ENDP1);

		size -= count;
		str += count;

		for(i = 0; i < 10000; i++);
	} while(size);

}


extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
extern __IO uint32_t count_out;
unsigned char flag_usb = 0;
extern void delay(__IO uint32_t nCount);
extern uint8_t RxCounter4;
uint8_t NumByt;
//extern unsigned char* yy;
unsigned char RS_data_in;

void handler_usb_rs(void)
{
	if((count_out != 0) && (bDeviceState == CONFIGURED))
	{
		//if(count_out>=64)
		//delay(1);
		for(uint8_t i=0; i<count_out; i++)
			buffer_out[i] ++;

		To_USB_Send_Str(buffer_out,count_out);
		//To_USB_Send_Data(buffer_out[0]);
//      if(buffer_out[0]==10)  GPIO_SetBits(GPIOD, GPIO_Pin_13); //SD
//      else 	 GPIO_ResetBits(GPIOD, GPIO_Pin_13); //SD
		//  USB_To_USART_Send_Data(&buffer_out[0], count_out);
		NumByt = count_out;

		//    if(NumByt<(BufUSB[SIZ_RS]+4))return; //delay(1);	   //!

		RS_data_in = 1;

		flag_usb = 1;
		count_out = 0;
	}
}


///*******************************************************************************
// * Function Name  : USB_To_USART_Send_Data.
// * Description    : send the received data from USB to the UART 0.
// * Input          : data_buffer: data address.
// *******************************************************************************/
//void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes)
//{
//	for(uint32_t i = 0; i < Nb_bytes; i++)
//	{
////    USART_SendData(USART1, *(data_buffer + i));
////    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//	}
//}

