#include "rf.h"
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

/*******************************************************************************
 * Function Name  : SPI_Send
 *******************************************************************************/
uint8_t SPI_Send(uint8_t data)
{
	SPI1->DR = data;

	int32_t failCnt = 100;
	while((!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) && (failCnt-- > 0))
		;

//	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); ///###

	failCnt = 100;
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) && (failCnt-- > 0))
		;

	return SPI1->DR;
}

void spi_sw_transfer(uint8_t *in_buf, const uint8_t *out_buf, uint32_t length)
{
	if(out_buf)
	{
		if(in_buf)
		{
			for(uint32_t i = 0; i < length; i++)
				in_buf[i] = SPI_Send(out_buf[i]);
		}
		else
		{
			for(uint32_t i = 0; i < length; i++)
				SPI_Send(out_buf[i]);
		}
	}
	else
	{
		if(in_buf)
		{
			for(uint32_t i = 0; i < length; i++)
				in_buf[i] = SPI_Send(0);
		}
		else
		{
			for(uint32_t i = 0; i < length; i++)
				SPI_Send(0);
		}
	}
}

void spi_sw_begin(void)
{
	NRF_CSN_L;
}

void spi_sw_end(void)
{
	NRF_CSN_H;
}

void rf_init(void)
{
	//spi_sw_init();
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

	delay_ms(50);
}

void rf_set_speed(NRF_SPEED speed)
{
	uint8_t reg_old = rf_read_reg_byte(NRF_REG_RF_SETUP);
	uint8_t reg_new = reg_old;

	reg_new &= ~(NRF_RF_SETUP_RF_DR_LOW | NRF_RF_SETUP_RF_DR_HIGH);

	switch(speed)
	{
	case NRF_SPEED_250K:
		reg_new |= NRF_RF_SETUP_RF_DR_LOW;
		break;

	case NRF_SPEED_1M:
		break;

	case NRF_SPEED_2M:
		reg_new |= NRF_RF_SETUP_RF_DR_HIGH;
		break;

	default:
		break;
	}

	if(reg_old != reg_new)
	{
		rf_write_reg_byte(NRF_REG_RF_SETUP, reg_new); // Update if we need
	}
}

void rf_set_power(NRF_POWER power)
{
	uint8_t reg_old = rf_read_reg_byte(NRF_REG_RF_SETUP);
	uint8_t reg_new = reg_old;

	reg_new &= ~(NRF_RF_SETUP_RF_PWR | 1);
	reg_new |= (uint8_t) power << 1;

	// In case this is a SI24R1 chip and the highest power is requested, set
	// the first bit to get 7dBm output.
	if(power == NRF_POWER_0DBM)
	{
		reg_new |= 1;
	}

	if(reg_old != reg_new)
	{
		rf_write_reg_byte(NRF_REG_RF_SETUP, reg_new); // Update if we need
	}
}

void rf_set_address_width(NRF_AW aw)
{
	rf_write_reg_byte(NRF_REG_SETUP_AW, (uint8_t) aw + 1);
}

void rf_set_crc_type(NRF_CRC crc_type)
{
	uint8_t reg_old = rf_read_reg_byte(NRF_REG_CONFIG);
	uint8_t reg_new = reg_old;

	reg_new &= ~(NRF_CONFIG_CRCO | NRF_CONFIG_EN_CRC);

	switch(crc_type)
	{
	case NRF_CRC_DISABLED:
		break;

	case NRF_CRC_1B:
		reg_new |= NRF_CONFIG_EN_CRC;
		break;

	case NRF_CRC_2B:
		reg_new |= NRF_CONFIG_EN_CRC | NRF_CONFIG_CRCO;
		break;

	default:
		break;
	}

	if(reg_old != reg_new)
	{
		rf_write_reg_byte(NRF_REG_CONFIG, reg_new); // Update if we need
	}
}

void rf_set_retr_retries(uint16_t retries)
{
	uint8_t reg_old = rf_read_reg_byte(NRF_REG_SETUP_RETR);
	uint8_t reg_new = reg_old;

	reg_new &= ~NRF_SETUP_RETR_ARC;
	reg_new |= (uint8_t) retries & 0xF;

	if(reg_old != reg_new)
	{
		rf_write_reg_byte(NRF_REG_SETUP_RETR, reg_new); // Update if we need
	}
}

void rf_set_retr_delay(NRF_RETR_DELAY delay)
{
	uint8_t reg_old = rf_read_reg_byte(NRF_REG_SETUP_RETR);
	uint8_t reg_new = reg_old;

	reg_new &= ~NRF_SETUP_RETR_ARD;
	reg_new |= ((uint8_t) delay & 0xF) << 4;

	if(reg_old != reg_new)
	{
		rf_write_reg_byte(NRF_REG_SETUP_RETR, reg_new); // Update if we need
	}
}

void rf_set_rx_addr(uint16_t pipe, const uint8_t *address, uint16_t addr_len)
{
	rf_write_reg(NRF_REG_RX_ADDR_P0 + pipe, address, addr_len);
}

void rf_set_tx_addr(const uint8_t *address, uint16_t addr_len)
{
	rf_write_reg(NRF_REG_TX_ADDR, address, addr_len);
}

void rf_write_tx_payload(const uint8_t *data, uint16_t length)
{
	uint8_t cmd = NRF_CMD_WRITE_TX_PAYLOAD;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(0, data, length);
	spi_sw_end();
}

// Write payload for transmission without requesting acknowledge
void rf_write_tx_payload_no_ack(const uint8_t *data, uint16_t length)
{
	uint8_t cmd = NRF_CMD_WRITE_TX_PAYLOAD_NO_ACK;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(0, data, length);
	spi_sw_end();
}

// Write payload for acknowledge
void rf_write_ack_payload(uint16_t pipe, const uint8_t *data, uint16_t length)
{
	uint8_t cmd = NRF_CMD_WRITE_ACK_PAYLOAD | (pipe & 0x7);
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(0, data, length);
	spi_sw_end();
}

// Read recieved payload
void rf_read_rx_payload(uint8_t *data, uint16_t length)
{
	uint8_t cmd = NRF_CMD_READ_RX_PAYLOAD;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(data, 0, length);
	spi_sw_end();
}

// Set radio frequency in MHz (2400 to 2525 allowed)
void rf_set_frequency(uint16_t freq)
{
	rf_write_reg_byte(NRF_REG_RF_CH, (freq - 2400) & 0x7F);
}

// Get radio frequency in MHz
uint16_t rf_get_frequency(void)
{
	return rf_read_reg_byte(NRF_REG_RF_CH) + 2400;
}

uint8_t rf_get_address_width(void)
{
	return rf_read_reg_byte(NRF_REG_SETUP_AW) + 2;
}

// Turn on radio
void rf_power_up(void)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_CONFIG);
	if((tmp & (NRF_CONFIG_PWR_UP)) != (NRF_CONFIG_PWR_UP))
	{
		tmp |= (NRF_CONFIG_PWR_UP);
		rf_write_reg_byte(NRF_REG_CONFIG, tmp); //Update if we need
	}
}

// Turn off radio
void rf_power_down(void)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_CONFIG);
	if(tmp & (NRF_CONFIG_PWR_UP))
	{
		tmp &= ~(NRF_CONFIG_PWR_UP);
		rf_write_reg_byte(NRF_REG_CONFIG, tmp); //Update if we need
	}
}

// Set up radio for transmission
void rf_mode_tx(void)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_CONFIG);
	if(tmp & (NRF_CONFIG_PRIM_RX))
	{
		tmp &= ~(NRF_CONFIG_PRIM_RX);
		rf_write_reg_byte(NRF_REG_CONFIG, tmp); //Update if we need
	}
}

// Set up radio for reception
void rf_mode_rx(void)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_CONFIG);
	if((tmp & (NRF_CONFIG_PRIM_RX)) != (NRF_CONFIG_PRIM_RX))
	{
		tmp |= (NRF_CONFIG_PRIM_RX);
		rf_write_reg_byte(NRF_REG_CONFIG, tmp); //Update if we need
	}
}

// Enable autoack on pipe
void rf_enable_pipe_autoack(uint16_t pipes)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_EN_AA);
	if((tmp & (pipes)) != (pipes))
	{
		tmp |= (pipes);
		rf_write_reg_byte(NRF_REG_EN_AA, tmp); //Update if we need
	}
}

// Enable address on pipe
void rf_enable_pipe_address(uint16_t pipes)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_EN_RXADDR);
	if((tmp & (pipes)) != (pipes))
	{
		tmp |= (pipes);
		rf_write_reg_byte(NRF_REG_EN_RXADDR, tmp); //Update if we need
	}
}

// Enable dynamic payload length
void rf_enable_pipe_dlp(uint16_t pipes)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_DYNPD);
	if((tmp & (pipes)) != (pipes))
	{
		tmp |= (pipes);
		rf_write_reg_byte(NRF_REG_DYNPD, tmp); //Update if we need
	}
}

// Enabled various features
void rf_enable_features(uint16_t features)
{
	uint16_t tmp = rf_read_reg_byte(NRF_REG_FEATURE);
	if((tmp & (features)) != (features))
	{
		tmp |= (features);
		rf_write_reg_byte(NRF_REG_FEATURE, tmp); //Update if we need
	}
}

void rf_flush_tx(void)
{
	uint8_t cmd = NRF_CMD_FLUSH_TX;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_end();
}

void rf_flush_rx(void)
{
	uint8_t cmd = NRF_CMD_FLUSH_RX;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_end();
}

void rf_flush_all(void)
{
	rf_flush_rx();
	rf_flush_tx();
}

void rf_clear_irq(void)
{
	rf_write_reg_byte(NRF_REG_STATUS, NRF_STATUS_IRQ);
}

void rf_clear_rx_irq(void)
{
	rf_write_reg_byte(NRF_REG_STATUS, NRF_STATUS_RX_DR);
}

void rf_clear_tx_irq(void)
{
	rf_write_reg_byte(NRF_REG_STATUS, NRF_STATUS_TX_DS);
}

void rf_clear_maxrt_irq(void)
{
	rf_write_reg_byte(NRF_REG_STATUS, NRF_STATUS_MAX_RT);
}

uint8_t rf_get_payload_width(void)
{
	uint8_t w;
	uint8_t cmd = NRF_CMD_READ_RX_PAYLOAD_WIDTH;
	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(&w, 0, 1);
	spi_sw_end();
	return w;
}

uint8_t rf_status(void)
{
	uint8_t w = NRF_CMD_NOP;
	spi_sw_begin();
	spi_sw_transfer(&w, &w, 1);
	spi_sw_end();
	return w;
}

uint8_t rf_fifo_status(void)
{
	return rf_read_reg_byte(NRF_REG_FIFO_STATUS);
}

uint8_t rf_rx_power_detect(void)
{
	return rf_read_reg_byte(NRF_REG_RPD) >> 1;
}

void rf_write_reg(uint16_t reg, const uint8_t *data, uint16_t len)
{
	uint8_t cmd = NRF_CMD_WRITE_REGISTER | reg;

	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(0, data, len);
	spi_sw_end();
}

void rf_write_reg_byte(uint16_t reg, uint8_t data)
{
	rf_write_reg(reg, &data, 1);
}

void rf_read_reg(uint16_t reg, uint8_t *data, uint16_t len)
{
	uint8_t cmd = NRF_CMD_READ_REGISTER | reg;

	spi_sw_begin();
	spi_sw_transfer(0, &cmd, 1);
	spi_sw_transfer(data, 0, len);
	spi_sw_end();
}

uint8_t rf_read_reg_byte(uint16_t reg)
{
	uint8_t result;
	rf_read_reg(reg, &result, 1);
	return result;
}
