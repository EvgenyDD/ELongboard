/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NRF_H
#define NRF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Memory Map */
#define NRF_CONFIG      0x00
#define NRF_EN_AA       0x01
#define NRF_EN_RXADDR   0x02
#define NRF_SETUP_AW    0x03
#define NRF_SETUP_RETR  0x04
#define NRF_RF_CH       0x05
#define NRF_RF_SETUP    0x06
#define NRF_STATUS      0x07
#define NRF_OBSERVE_TX  0x08
#define NRF_CD          0x09
#define NRF_RX_ADDR_P0  0x0A
#define NRF_RX_ADDR_P1  0x0B
#define NRF_RX_ADDR_P2  0x0C
#define NRF_RX_ADDR_P3  0x0D
#define NRF_RX_ADDR_P4  0x0E
#define NRF_RX_ADDR_P5  0x0F
#define NRF_TX_ADDR     0x10
#define NRF_RX_PW_P0    0x11
#define NRF_RX_PW_P1    0x12
#define NRF_RX_PW_P2    0x13
#define NRF_RX_PW_P3    0x14
#define NRF_RX_PW_P4    0x15
#define NRF_RX_PW_P5    0x16
#define NRF_FIFO_STATUS 0x17
#define NRF_DYNPD 		0x1C
#define NRF_FEATURE		0x1D

/* Bit Mnemonics */
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0
#define AW          0
#define ARD         4
#define ARC         0
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      1
#define LNA_HCURR   0        
#define RX_DR       6
#define TX_DS       5
#define MAX_RT      4
#define RX_P_NO     1
#define TX_FULL     0
#define PLOS_CNT    4
#define ARC_CNT     0
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0

//RF setup register
#define NRF_PLL_LOCK		4
#define NRF_RF_DR_LOW		5
#define NRF_RF_DR_HIGH		3
#define NRF_RF_DR			3
#define NRF_RF_PWR			1 //2 bits
/* Instruction Mnemonics */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define R_PAYLOAD_W	  0x60
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

/* NRF Settings */
#define NRF_CONFIG_DEFAULT  ((0 << MASK_RX_DR) | (1 << MASK_TX_DS) | (1 << MASK_MAX_RT) | (1 << EN_CRC) | (0 << CRCO))

typedef enum
{
	NRF_Transmit_Status_Lost = 0,		//Message is lost, reached max retransmissions
	NRF_Transmit_Status_Ok = 1,		//Message sent successfully
	NRF_Transmit_Status_Sending = 0xFF	//Message is still sending
} NRF_Transmit_Status_t;

typedef enum
{
	NRF_DataRate_2M,		// 2Mbps
	NRF_DataRate_1M,		// 1Mbps
	NRF_DataRate_250k		// 250kbps
} NRF_DataRate_t;

typedef enum
{
	NRF_OutputPower_M18dBm,		// -18dBm
	NRF_OutputPower_M12dBm,		// -12dBm
	NRF_OutputPower_M6dBm,	// -6dBm
	NRF_OutputPower_0dBm	// 0dBm
} NRF_OutputPower_t;

/* Exported macro ------------------------------------------------------------*/
#define NRF_CSN_H	GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define NRF_CSN_L	GPIO_ResetBits(GPIOB, GPIO_Pin_10)
#define NRF_CE_H   GPIO_SetBits(GPIOB, GPIO_Pin_2)
#define NRF_CE_L   GPIO_ResetBits(GPIOB, GPIO_Pin_2)

/* Exported functions ------------------------------------------------------- */
void NRF_Init();
void NRF_Sleep();

void NRF_processIRQData();

void NRF_ConfigChnlPld( uint8_t, uint8_t);
void NRF_ConfigRatePwr(NRF_DataRate_t, NRF_OutputPower_t);
void NRF_SetChannel(uint8_t channel);

void NRF_SetRxAdr(uint8_t*);
void NRF_SetTxAdr(uint8_t*);

uint8_t NRF_ReadRegister( uint8_t);
void NRF_WriteRegisters( uint8_t, uint8_t*, uint8_t);
void NRF_WriteRegister( uint8_t, uint8_t);

void NRF_PowerUpRx();
void NRF_PowerUpTx();

void NRF_Send(uint8_t*, uint8_t);
NRF_Transmit_Status_t NRF_GetTransmissionStatus();

uint8_t NRF_IsRxDataReady();
uint8_t NRF_GetData(uint8_t*);

void SPI_WriteReadMass(uint8_t*, uint8_t*, uint8_t);
void SPI_WriteMass(uint8_t*, uint8_t);
uint8_t SPI_Send( uint8_t);
void SPI_ReadMass(uint8_t *data, uint8_t len);

#endif //NRF_H
