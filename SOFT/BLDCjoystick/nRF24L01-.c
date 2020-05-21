/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

#include "nRF24l01.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t TxBuf[Buffer_Size] = { 0 };
uint8_t RxBuf[Buffer_Size] = { 0 };

uint8_t nRF24L01_channel = 115;
uint8_t nRF24L01_power = P0dBm;

//define the initial Address
uint8_t TX_ADDRESS[ADR_WIDTH] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };
uint8_t RX_ADDRESS[ADR_WIDTH] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void nRF24L01_Init()
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);
	/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //SPI_CPOL_High=模式3，时钟空闲为高 //SPI_CPOL_Low=模式0，时钟空闲为低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //SPI_CPHA_2Edge;//SPI_CPHA_1Edge, SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //SPI_NSS_Soft;//SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; //SPI_BaudRatePrescaler_2=18M;//SPI_BaudRatePrescaler_4=9MHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //数据从高位开始发送
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
}

/*******************************************************************************
 * Function Name  : nRF24L01_Delay_us
 * Description    : Delay without timers, in us
 *******************************************************************************/
static void nRF24L01_Delay_us(uint32_t n)
{
	uint32_t i;

	while(n--)
	{
		i = 2;
		while(i--);
	}
}


/*******************************************************************************
 * Function Name  : nRF24L01_SPI_NSS_H
 * Description    : Disable SPI Communication
 *******************************************************************************/
static void nRF24L01_SPI_NSS_H()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_10);
}

/*******************************************************************************
 * Function Name  : nRF24L01_SPI_NSS_L
 * Description    : Enable SPI Communication
 *******************************************************************************/
static void nRF24L01_SPI_NSS_L()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}




/*******************************************************************************
 * Function Name  : nRF24L01_SPISendByte
 *******************************************************************************/
static uint8_t nRF24L01_SPISendByte(uint8_t dat)
{
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI1, dat);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI_I2S_ReceiveData(SPI1);
}

/*******************************************************************************
 * Function Name  : SPI_WriteReg
 *******************************************************************************/
static uint8_t SPI_WriteReg(uint8_t reg, uint8_t value)
{
	nRF24L01_SPI_NSS_L();                  // CSN low, init SPI transaction
	uint8_t status = nRF24L01_SPISendByte(reg);                  // select register
	nRF24L01_SPISendByte(value);             // ..and write value to it..
	nRF24L01_SPI_NSS_H();                   // CSN high again

	return status;            // return nRF24L01 status uint8_t
}

/*******************************************************************************
 * Function Name  : SPI_ReadReg
 *******************************************************************************/
uint8_t SPI_ReadReg(uint8_t reg)
{
	nRF24L01_SPI_NSS_L();                // CSN low, initialize SPI communication...
	nRF24L01_SPISendByte(reg);            // Select register to read from..
	uint8_t value = nRF24L01_SPISendByte(0);    // ..then read register value
	nRF24L01_SPI_NSS_H();                // CSN high, terminate SPI communication

	return value;       				 // return register value
}



/*******************************************************************************
 * Function Name  : SPI_ReadBuf
 *******************************************************************************/
uint8_t SPI_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t Len)
{
	nRF24L01_SPI_NSS_L();                    		// Set CSN low, init SPI tranaction
	uint8_t status = nRF24L01_SPISendByte(reg);   // Select register to write to and read status uint8_t

	for(uint16_t i = 0; i < Len; i++)
		pBuf[i] = nRF24L01_SPISendByte(0);

	nRF24L01_SPI_NSS_H();

	return status;                    // return nRF24L01 status uint8_t
}

/*******************************************************************************
 * Function Name  : SPI_WriteBuf
 *******************************************************************************/
static uint8_t SPI_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t Len)
{
	nRF24L01_SPI_NSS_L();

	uint8_t status = nRF24L01_SPISendByte(reg);
	for(uint16_t i = 0; i < Len; i++, pBuf++)
		nRF24L01_SPISendByte(*pBuf);

	nRF24L01_SPI_NSS_H();
	return status;
}


/*******************************************************************************
 * Function Name  : nRF24L01_Set_TX_Address
 *******************************************************************************/
void nRF24L01_Set_TX_Address(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E)
{
	TX_ADDRESS[0] = A;
	TX_ADDRESS[1] = B;
	TX_ADDRESS[2] = C;
	TX_ADDRESS[3] = D;
	TX_ADDRESS[4] = E;
}

/*******************************************************************************
 * Function Name  : nRF24L01_Set_RX_Address
 *******************************************************************************/
void nRF24L01_Set_RX_Address(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E)
{
	RX_ADDRESS[0] = A;
	RX_ADDRESS[1] = B;
	RX_ADDRESS[2] = C;
	RX_ADDRESS[3] = D;
	RX_ADDRESS[4] = E;
}

/*******************************************************************************
 * Function Name  : nRF24L01_Config
 * Description    : Configurate nRF24L01 module
 *******************************************************************************/
uint8_t nRF24L01_Config(uint8_t channel, uint8_t power, uint8_t speed)
{
	nRF24L01_channel = 0;
	nRF24L01_power = 0;

	if((channel > 125) && (channel < 0))
		return 0;
	else
		nRF24L01_channel = channel;

	if(P0dBm == power)
		nRF24L01_power |= 0x06;
	else if(Pm6dBm == power)
		nRF24L01_power |= 0x04;
	else if(Pm12dBm == power)
		nRF24L01_power |= 0x02;
	else if(Pm18dBm == power)
		nRF24L01_power |= 0x00;
	else
		return 0;

	if(speed == R2mbps)
		nRF24L01_power |= 0x08;
	else if(speed == R1mbps)
		nRF24L01_power |= 0x00;
	else if(speed == R250kbps)
		nRF24L01_power |= 0x20;
	else
		return 0;

	return 1;
	
}

/*******************************************************************************
 * Function Name  : nRF24L01_SetRxMode
 *******************************************************************************/
void nRF24L01_SetRxMode()
{
	uint8_t buf[5] = { 0 };

	SPI_ReadBuf(TX_ADDR, buf, ADR_WIDTH);

	SPI_WriteBuf(WRITE_nRF_REG + RX_ADDR_P0, RX_ADDRESS, ADR_WIDTH);

	SPI_WriteReg(WRITE_nRF_REG + EN_AA, 0);
	SPI_WriteReg(WRITE_nRF_REG + EN_RXADDR, 0x01);
	SPI_WriteReg(WRITE_nRF_REG + SETUP_RETR, 0x1a);
	SPI_WriteReg(WRITE_nRF_REG + RF_CH, nRF24L01_channel);
	SPI_WriteReg(WRITE_nRF_REG + RX_PW_P0, RX_PLOAD_WIDTH);
	SPI_WriteReg(WRITE_nRF_REG + RF_SETUP, nRF24L01_power);

	SPI_WriteReg(WRITE_nRF_REG + CONFIG, 0x03);

	nRF24L01_Delay_us(200);
}

/*******************************************************************************
 * Function Name  : nRF24L01_SetTxMode
 *******************************************************************************/
void nRF24L01_SetTxMode()
{
	SPI_WriteBuf(WRITE_nRF_REG + TX_ADDR, TX_ADDRESS, ADR_WIDTH);
	SPI_WriteBuf(WRITE_nRF_REG + RX_ADDR_P0, RX_ADDRESS, ADR_WIDTH);

	SPI_WriteReg(WRITE_nRF_REG + EN_AA, 0);
	SPI_WriteReg(WRITE_nRF_REG + EN_RXADDR, 0x01);
	SPI_WriteReg(WRITE_nRF_REG + SETUP_RETR, 0x1a);
	SPI_WriteReg(WRITE_nRF_REG + RF_CH, nRF24L01_channel);
	SPI_WriteReg(WRITE_nRF_REG + RF_SETUP, nRF24L01_power);
	SPI_WriteReg(WRITE_nRF_REG + CONFIG, 0x02);

}

/*******************************************************************************
 * Function Name  : nRF24L01_TxPacket
 * Description    : Transmit packet over radio
 *******************************************************************************/
void nRF24L01_TxPacket(uint8_t* tx_buf)
{
	SPI_WriteBuf(WRITE_nRF_REG + RX_ADDR_P0, TX_ADDRESS, ADR_WIDTH);
	SPI_WriteBuf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);
}

/*******************************************************************************
 * Function Name  : nRF24L01_RxPacket
 * Description    : Receive packet over radio
 *******************************************************************************/
uint8_t nRF24L01_RxPacket(uint8_t* rx_buf)
{
	uint8_t flag = 0;
	uint8_t status = SPI_ReadReg(NRFRegSTATUS);

	if(status & 0x40)
	{
		SPI_ReadBuf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);
		flag = 1;
	}

	SPI_WriteReg(WRITE_nRF_REG + NRFRegSTATUS, status);
	return flag;
}
