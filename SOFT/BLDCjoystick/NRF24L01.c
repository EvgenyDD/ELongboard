/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "NRF24L01.h"
#include "macros.h"
#include "misc.h"
#include "interface.h"
#include <stdbool.h>
//#include "spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define NRF_CLEAR_INTERRUPTS		NRF_WriteRegisterBit(NRF_STATUS, 4, 1); \
									NRF_WriteRegisterBit(NRF_STATUS, 5, 1); \
									NRF_WriteRegisterBit(NRF_STATUS, 6, 1)

/* Private variables ---------------------------------------------------------*/
volatile uint8_t TxMode;		// Flag which denotes transmitting mode
uint8_t Payload;
uint8_t RxMsgSize = 0;

static bool nrfNeedToReadIRQ = false;

uint8_t RxArray[32];

uint8_t RxAddress[] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };
uint8_t TxAddress[] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };
#define ADDR_LEN (3)

// CRC Table
const unsigned short crc16_tab[] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad,
	0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a,
	0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,
	0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861,
	0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
	0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87,
	0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
	0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3,
	0x5004, 0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290,
	0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e,
	0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f,
	0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
	0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83,
	0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
	0x2e93, 0x3eb2, 0x0ed1, 0x1ef0 };

/* Extern variables ----------------------------------------------------------*/
extern uint32_t failPKT, succPKT;
extern uint32_t timeRx, timeRxCount;

extern uint32_t sysTickerMs;
extern InterfaceStatusTypedef currentState;


/* Private function prototypes -----------------------------------------------*/
void delay_ms(uint32_t nTime);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : NRF_WriteRegisterBit
 * Description    : Write Bit in nRF24L01+ register
 *******************************************************************************/
void NRF_WriteRegisterBit(uint8_t reg, uint8_t bit, uint8_t value)
{
	uint8_t tmp = NRF_ReadRegister(reg);
	if(value != 0)
		tmp |= 1 << bit;
	else
		tmp &= ~(1 << bit);

	NRF_WriteRegister(reg, tmp);
}

/*******************************************************************************
 * Function Name  : crc16
 *******************************************************************************/
static unsigned short crc16(unsigned char *buf, unsigned int len)
{
	unsigned short cksum = 0;
	for(uint16_t i = 0; i < len; i++)
		cksum = crc16_tab[(((cksum >> 8) ^ *buf++) & 0xFF)] ^ (cksum << 8);

	return cksum;
}

/*******************************************************************************
 * Function Name  : NRF_Init
 *******************************************************************************/
void NRF_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Configure SPI1 pins: SCK, MISO and MOSI */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI1 configuration */
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //SPI_BaudRatePrescaler_2=18M;//SPI_BaudRatePrescaler_4=9MHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);

	//Config the NSS pin
	//SPI_SSOutputCmd(SPI1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);

//	EXTI_InitTypeDef EXTI_InitStructure;
//	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);

//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x07;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	NRF_CE_L;
	NRF_CSN_H;
}

void NRF_Sleep()
{
	NRF_CLEAR_INTERRUPTS;
	NRF_CE_L;
	NRF_WriteRegister(NRF_CONFIG, 0);
	NRF_CE_H;
}

/*******************************************************************************
 * Function Name  : NRF_ConfigChnlPld
 * Description    : Sets the channel & payload size + switch to Rx mode
 *******************************************************************************/
void NRF_ConfigChnlPld(uint8_t channel, uint8_t payload)
{
	// Set RF channel
	NRF_SetChannel(channel);

	// Set length of incoming payload
	Payload = payload > 32 ? 32 : payload;
	NRF_WriteRegister(NRF_RX_PW_P0, Payload);

	NRF_WriteRegister(NRF_SETUP_AW, 0x01); //0x01 - 3 byte address

	// Set RADDR and TADDR as the transmit address since we also enable auto acknowledgement
	NRF_SetRxAdr(RxAddress);
	NRF_SetTxAdr(TxAddress);

	// Enable RX_ADDR_P0 address matching
	NRF_WriteRegister(NRF_EN_RXADDR, 1 << ERX_P0);

	NRF_PowerUpRx();     	// Power up in receiving mode
	NRF_CE_H;     			// Listening for pakets
	delay_ms(1);
}

/*******************************************************************************
 * Function Name  : NRF_ConfigRatePwr
 * Description    : Configure data rate and output power
 *******************************************************************************/
void NRF_ConfigRatePwr(NRF_DataRate_t DataRate, NRF_OutputPower_t OutPwr)
{
	uint8_t tmp = 1;

	if(DataRate == NRF_DataRate_2M)
		tmp |= 1 << NRF_RF_DR_HIGH;
	else if(DataRate == NRF_DataRate_250k)
		tmp |= 1 << NRF_RF_DR_LOW;
	//If 1Mbps, all bits set to 0

	if(OutPwr == NRF_OutputPower_0dBm)
		tmp |= 3 << NRF_RF_PWR;
	else if(OutPwr == NRF_OutputPower_M6dBm)
		tmp |= 2 << NRF_RF_PWR;
	else if(OutPwr == NRF_OutputPower_M12dBm)
		tmp |= 1 << NRF_RF_PWR;

	NRF_WriteRegister(NRF_RF_SETUP, tmp);
}

/*******************************************************************************
 * Function Name  : NRF_SetRxAdr
 * Description    : Set the receiving address
 *******************************************************************************/
void NRF_SetRxAdr(uint8_t *adr)
{
	NRF_CE_L;
	NRF_WriteRegisters(NRF_RX_ADDR_P0, adr, ADDR_LEN);
	NRF_CE_H;
}

/*******************************************************************************
 * Function Name  : NRF_SetTxAdr
 * Description    : Set the transmitting address
 *******************************************************************************/
void NRF_SetTxAdr(uint8_t *adr)
{
	NRF_CE_L;
	NRF_WriteRegisters(NRF_TX_ADDR, adr, ADDR_LEN);
	NRF_CE_H;
}

/*******************************************************************************
 * Function Name  : NRF_ReadRegister
 *******************************************************************************/
uint8_t NRF_ReadRegister(uint8_t reg)
{
	NRF_CSN_L;
	SPI_Send(R_REGISTER | (REGISTER_MASK & reg));
	uint8_t value = SPI_Send(NOP);
	NRF_CSN_H;

	return value;
}

/*******************************************************************************
 * Function Name  : NRF_WriteRegister
 *******************************************************************************/
void NRF_WriteRegister(uint8_t reg, uint8_t value)
{
	NRF_CSN_L;
	SPI_Send(W_REGISTER | (REGISTER_MASK & reg));
	SPI_Send(value);
	NRF_CSN_H;
}

/*******************************************************************************
 * Function Name  : NRF_WriteRegisters
 * Description    : Writes an array of bytes
 *******************************************************************************/
void NRF_WriteRegisters(uint8_t reg, uint8_t *value, uint8_t len)
{
	NRF_CSN_L;
	SPI_Send(W_REGISTER | (REGISTER_MASK & reg));
	SPI_WriteMass(value, len);
	NRF_CSN_H;
}

/*******************************************************************************
 * Function Name  : NRF_ReadRegisters
 * Description    : Read an array of bytes
 *******************************************************************************/
void NRF_ReadRegisters(uint8_t reg, uint8_t *value, uint8_t len)
{
	NRF_CSN_L;
	SPI_Send(R_REGISTER | (REGISTER_MASK & reg));
	SPI_ReadMass(value, len);
	NRF_CSN_H;
}

/*******************************************************************************
 * Function Name  : NRF_SetChannel
 *******************************************************************************/
void NRF_SetChannel(uint8_t channel)
{
	NRF_WriteRegister(NRF_RF_CH, channel > 125 ? 125 : channel);
}

/*******************************************************************************
 * Function Name  : EXTI4_IRQHandler
 * Description    : NRF24L01 Handler
 *******************************************************************************/
void EXTI4_IRQHandler()
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line4);

        nrfNeedToReadIRQ = true;
    }
}

void NRF_processIRQData()
{
	//if(nrfNeedToReadIRQ)
//	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
	//	nrfNeedToReadIRQ = false;
		currentState.numxxx = NRF_ReadRegister(0x07);

		if(BitIsSet(currentState.numxxx, RX_DR))
		{
			RxMsgSize = NRF_GetData(RxArray);

			if(RxMsgSize < 2)
				return;
			if(crc16(RxArray, RxMsgSize - 2) != ((RxArray[RxMsgSize - 2] << 8) | RxArray[RxMsgSize - 1]))
				return;

			InterfaceProcessInputPacket(RxArray, RxMsgSize);

			static uint32_t prev = 0;
			currentState.RxLatency = (sysTickerMs - prev) > 999 ? 999 : (sysTickerMs - prev);
			prev = sysTickerMs;
		}
	}
}


/*******************************************************************************
 * Function Name  : NRF_IsRxDataReady
 * Description    : If data is available for reading
 *******************************************************************************/
uint8_t NRF_IsDataReady()
{
	NRF_CSN_L;
	uint8_t status = SPI_Send(NOP);          // Read status register
	NRF_CSN_H;
	return status & (1 << RX_DR);
}

/*******************************************************************************
 * Function Name  : NRF_GetData
 * Description    : Read Payload bytes
 *******************************************************************************/
uint8_t NRF_GetData(uint8_t *data)
{
	NRF_CSN_L;
	SPI_Send(R_PAYLOAD_W);
	uint8_t Payload = SPI_Send(0);
	NRF_CSN_H;

	NRF_CSN_L;
	SPI_Send(R_RX_PAYLOAD);            	// Send cmd to read rx payload
	SPI_ReadMass(data, Payload); 		// Read payload
	NRF_CSN_H;

	NRF_WriteRegister(NRF_STATUS, (1 << RX_DR));   // Reset status register

	//CRC Check
//	if (res >= 0 && *len > 2) {
//		unsigned short crc = crc16((unsigned char*)data, *len - 2);
//
//		if (crc	!= ((unsigned short) data[*len - 2] << 8 | (unsigned short) data[*len - 1])) {
//			res = -3;
//		}
//	}
//
//	*len -= 2;

	return Payload;
}

/*******************************************************************************
 * Function Name  : NRF_PowerUpRx
 *******************************************************************************/
void NRF_PowerUpRx()
{
	NRF_CSN_L;
	SPI_Send(FLUSH_RX);     // Write cmd to flush tx fifo
	NRF_CSN_H;

	NRF_CLEAR_INTERRUPTS;
	NRF_CE_L;
	NRF_WriteRegister(NRF_CONFIG, NRF_CONFIG_DEFAULT | ((1 << PRIM_RX) | (1 << PWR_UP)));
	NRF_CE_H;
	//delay_ms(2);
}

/*******************************************************************************
 * Function Name  : NRF_PowerUpTx
 *******************************************************************************/
void NRF_PowerUpTx()
{
	NRF_CLEAR_INTERRUPTS;
	NRF_WriteRegister(NRF_CONFIG, NRF_CONFIG_DEFAULT | (0 << PRIM_RX) | (1 << PWR_UP));
//	delay_ms(2);
}

/*******************************************************************************
 * Function Name  : NRF_Send
 * Description    : Sends a data package
 *******************************************************************************/
void NRF_Send(uint8_t *value, uint8_t len)
{
	unsigned short crc = crc16((unsigned char*) value, len);

	value[len] = (char) (crc >> 8);
	value[len + 1] = (char) (crc & 0xFF);

	NRF_CE_L;

	NRF_PowerUpTx();

	NRF_CSN_L;
	SPI_Send(FLUSH_TX);     			// Write cmd to flush tx fifo
	NRF_CSN_H;

	NRF_CSN_L;
	SPI_Send(W_TX_PAYLOAD/* | (1 << 3)*/); 	// Write cmd to write payload
	SPI_WriteMass(value, len + 2);   	// Write payload
	NRF_CSN_H;

	NRF_CE_H;               			// Start transmission
	for(volatile uint32_t i=0; i<500; i++);
	NRF_CE_L;
}

/*******************************************************************************
 * Function Name  : NRF_GetTransmissionStatus
 *******************************************************************************/
NRF_Transmit_Status_t NRF_GetTransmissionStatus()
{
	NRF_CSN_L;
	uint8_t status = SPI_Send(NOP);
	NRF_CSN_H;

	if(BitIsSet(status, TX_DS))
		return NRF_Transmit_Status_Ok;
	else if(BitIsSet(status, MAX_RT))
		return NRF_Transmit_Status_Lost;

	return NRF_Transmit_Status_Sending;
}

/*******************************************************************************
 * Function Name  : SPI_WriteReadMass
 *******************************************************************************/
void SPI_WriteReadMass(uint8_t *dataout, uint8_t *datain, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		datain[i] = SPI_Send(dataout[i]);
}

/*******************************************************************************
 * Function Name  : SPI_ReadMass
 *******************************************************************************/
void SPI_ReadMass(uint8_t *data, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		data[i] = SPI_Send(0);
}

/*******************************************************************************
 * Function Name  : SPI_WriteMass
 *******************************************************************************/
void SPI_WriteMass(uint8_t *dataout, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		SPI_Send(dataout[i]);
}

/*******************************************************************************
 * Function Name  : SPI_Send
 *******************************************************************************/
uint8_t SPI_Send(uint8_t data)
{
	SPI1->DR = data;

	int32_t failCnt = 100;
	while((!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) && (failCnt-- > 0));

//	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); ///###

	failCnt = 100;
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) && (failCnt-- > 0));

	return SPI1->DR;
}
