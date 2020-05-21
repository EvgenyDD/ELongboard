/*
 Copyright 2012-2015 Benjamin Vedder	benjamin@vedder.se

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

/*
 * nrf_driver.c
 *
 *  Created on: 29 mar 2015
 *      Author: benjamin
 */

#include <string.h>
#include "nrf_driver.h"
#include "rf.h"
#include "rfhelp.h"
#include "interface.h"

// Settings
#define MAX_PL_LEN				25
#define RX_BUFFER_SIZE			PACKET_MAX_PL_LEN


// Variables
int nosend_cnt;
int nrf_restart_rx_time;
int nrf_restart_tx_time;

extern InterfaceStatusTypedef currentState;
extern uint32_t sysTickerMs;
extern bool isMovingNow;

float smartSpeedIncrement;


uint8_t nrfStatus = STS_LOST;
bool nrfNotDisbled = true;



void nrf_driver_init()
{
	rfhelp_init();

	nosend_cnt = 0;
	nrf_restart_rx_time = 0;
	nrf_restart_tx_time = 0;
}

int rf_tx_wrapper(uint8_t *data, uint32_t len)
{
	int res = rfhelp_send_data_crc(data, len);

	if(res == 0)
	{
		nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
	}

	return res;
}

void NRF_processIRQData()
{
	static uint8_t buf[32];
	static uint16_t len;
	static uint8_t pipe = 0;

	int res = rfhelp_read_rx_data_crc(buf, &len, &pipe);

	if(res >= 0)
	{
		nrfStatus = STS_CONN;
		nrf_restart_rx_time = NRF_RESTART_TIMEOUT;

		InterfaceProcessInputPacket(buf, len);

		if(abs(currentState.speed) > 1.0)
			isMovingNow = true;

		static uint32_t prev = 0;
		currentState.RxLatency = (sysTickerMs - prev) > 999 ? 999 : (sysTickerMs - prev);
		prev = sysTickerMs;
	}
	else
		isMovingNow = false;
}
