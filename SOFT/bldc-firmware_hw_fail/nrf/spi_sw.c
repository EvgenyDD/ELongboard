/*
 Copyright 2015 Benjamin Vedder	benjamin@vedder.se

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "spi_sw.h"
#include "ch.h"
#include "hal.h"
#include <stdbool.h>

#define USE_HW_SPI

// Private variables
static bool init_done = false;

// Private functions
static void spi_sw_delay(void);

uint8_t zerodata[128], trashdata[128];


#define SPIDIV_8    SPI_CR1_BR_1
#define SPIDIV_16   SPI_CR1_BR_1 | SPI_CR1_BR_0
#define SPIDIV_32   SPI_CR1_BR_2
#define SPIDIV_64   SPI_CR1_BR_2 | SPI_CR1_BR_0                 //666khz
#define SPIDIV_128  SPI_CR1_BR_2 | SPI_CR1_BR_1
#define SPIDIV_256  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0  //138khz

static const SPIConfig spicfg = {
  NULL,
  NRF_PORT_CSN,
  NRF_PIN_CSN,
  SPIDIV_64 | SPI_CR1_MSTR | SPI_CR1_BIDIOE
};

void spi_sw_init(void)
{
    for(uint8_t i=0; i<128; i++)
        zerodata[i] = 0;

    if(!init_done)
    {
#ifndef USE_HW_SPI
        palSetPadMode(NRF_PORT_MISO, NRF_PIN_MISO, PAL_MODE_INPUT);
        palSetPadMode(NRF_PORT_CSN, NRF_PIN_CSN, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(NRF_PORT_SCK, NRF_PIN_SCK, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(NRF_PORT_MOSI, NRF_PIN_MOSI, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(NRF_PORT_CE, NRF_PIN_CE, PAL_MODE_OUTPUT_PUSHPULL);

        palSetPadMode(DRV8301_SPI_PORT_MISO, DRV8301_SPI_PIN_MISO, PAL_MODE_INPUT);
        palSetPadMode(DRV8301_SPI_PORT_NSS, DRV8301_SPI_PIN_NSS, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(DRV8301_SPI_PORT_SCK, DRV8301_SPI_PIN_SCK, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(DRV8301_SPI_PORT_MOSI, DRV8301_SPI_PIN_MOSI, PAL_MODE_OUTPUT_PUSHPULL);

        palSetPad(NRF_PORT_CSN, NRF_PIN_CSN);
        palSetPad(DRV8301_SPI_PORT_NSS, DRV8301_SPI_PIN_NSS);
        palClearPad(NRF_PORT_SCK, NRF_PIN_SCK);
#else

        palSetPadMode(NRF_PORT_CSN, NRF_PIN_CSN, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPadMode(NRF_PORT_CE, NRF_PIN_CE, PAL_MODE_OUTPUT_PUSHPULL);

        palSetPad(NRF_PORT_CSN, NRF_PIN_CSN);

        palSetPadMode(NRF_PORT_SCK, NRF_PIN_SCK, PAL_MODE_ALTERNATE(6) | PAL_STM32_OSPEED_HIGHEST);
        palSetPadMode(NRF_PORT_MISO, NRF_PIN_MISO, PAL_MODE_ALTERNATE(6));
        palSetPadMode(NRF_PORT_MOSI, NRF_PIN_MOSI, PAL_MODE_ALTERNATE(6) | PAL_STM32_OSPEED_HIGHEST);


        palSetPadMode(DRV8301_SPI_PORT_NSS, DRV8301_SPI_PIN_NSS, PAL_MODE_OUTPUT_PUSHPULL);
        palSetPad(DRV8301_SPI_PORT_NSS, DRV8301_SPI_PIN_NSS);

        palSetPadMode(DRV8301_SPI_PORT_SCK, DRV8301_SPI_PIN_SCK, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
        palSetPadMode(DRV8301_SPI_PORT_MOSI, DRV8301_SPI_PIN_MOSI, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
        palSetPadMode(DRV8301_SPI_PORT_MISO, DRV8301_SPI_PIN_MISO, PAL_MODE_ALTERNATE(5));

        spiStart(&SPID3, &spicfg);
        spiStart(&SPID1, &spicfg);
#endif

        init_done = true;
    }
}

void spi_sw_transfer(uint8_t *in_buf, const uint8_t *out_buf, uint8_t length)
{
#ifdef USE_HW_SPI

    spiExchange(&SPID3, length, ((out_buf == 0) ? zerodata : out_buf), ((in_buf == 0) ? trashdata : in_buf));

#else
    palClearPad(NRF_PORT_SCK, NRF_PIN_SCK);
    spi_sw_delay();

    for(int i = 0; i < length; i++)
    {
        unsigned char send = out_buf ? out_buf[i] : 0;
        unsigned char recieve = 0;

        for(int bit = 0; bit < 8; bit++)
        {
            palWritePad(NRF_PORT_MOSI, NRF_PIN_MOSI, send >> 7);
            send <<= 1;

            spi_sw_delay();

            recieve <<= 1;
            if(palReadPad(NRF_PORT_MISO, NRF_PIN_MISO))
            {
                recieve |= 0x1;
            }

            palSetPad(NRF_PORT_SCK, NRF_PIN_SCK);
            spi_sw_delay();
            palClearPad(NRF_PORT_SCK, NRF_PIN_SCK);
        }

        if(in_buf)
        {
            in_buf[i] = recieve;
        }
    }
#endif
}

void spi_sw_begin(void)
{
#ifndef USE_HW_SPI
    palClearPad(NRF_PORT_CSN, NRF_PIN_CSN);

    spi_sw_delay();
#else
    spiSelect(&SPID3);
#endif
}

void spi_sw_end(void)
{
#ifndef USE_HW_SPI
    spi_sw_delay();

    palSetPad(NRF_PORT_CSN, NRF_PIN_CSN);
#else
    spiUnselect(&SPID3);
#endif
}

static void spi_sw_delay(void)
{
    for(volatile int i = 0; i < 5; i++)
    {
        __NOP();
    }
}

void spi_drv_begin(void)
{
    spiSelect(&SPID1);
}

void spi_drv_transfer(uint8_t *in_buf, const uint8_t *out_buf, uint8_t length)
{
    spiExchange(&SPID1, length, ((out_buf == 0) ? zerodata : out_buf), ((in_buf == 0) ? trashdata : in_buf));
}

void spi_drv_end(void)
{
    spiUnselect(&SPID1);
}
