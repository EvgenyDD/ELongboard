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
#include "conf_general.h"
#include "app.h"
#include "buffer.h"
#include "commands.h"
#include "crc.h"
#include "packet.h"
#include "hw.h"
#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include "encoder.h"

#include "mc_interface.h"
#include "timeout.h"

#include "rf.h"

// Settings
#define MAX_PL_LEN				32
#define RX_BUFFER_SIZE			PACKET_MAX_PL_LEN

#define TX_FREQ_INTERVAL		50  // Send alive packets at this rate
#define NRF_RESTART_TIMEOUT		400  // Restart the NRF if nothing has been received or acked for this time
// Variables
static THD_WORKING_AREA(rx_thread_wa, 2048);
static THD_WORKING_AREA(tx_thread_wa, 512);


static int nosend_cnt;
static int nrf_restart_rx_time;
static int nrf_restart_tx_time;

// Functions
static THD_FUNCTION(rx_thread, arg);
static THD_FUNCTION(tx_thread, arg);


extern mutex_t rf_mutex;

binary_semaphore_t nrf_bsem;

void nrf_driver_init(void)
{
    rfhelp_init();

    nosend_cnt = 0;
    nrf_restart_rx_time = 0;
    nrf_restart_tx_time = 0;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    chBSemObjectInit(&nrf_bsem, true);

    // Connect EXTI Line to pin
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource0);

    // Configure EXTI Line
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Enable and set EXTI Line Interrupt priority

    nvicEnableVector(EXTI0_IRQn, 10);

    chThdCreateStatic(rx_thread_wa, sizeof(rx_thread_wa), NORMALPRIO+1, rx_thread, NULL);
    chThdCreateStatic(tx_thread_wa, sizeof(tx_thread_wa), NORMALPRIO, tx_thread, NULL);

    rfhelp_power_up();
    rfhelp_restart();
}

static void MakeStatusPacket(uint8_t *data, int32_t *ind)
{
    *ind = 0;
    data[(*ind)++] = 0; //packet id
    buffer_append_float16(data, mc_interface_get_rpm() / 7.0, 1.0, ind);
    buffer_append_int16(data, mc_interface_read_reset_avg_motor_current() * 10, ind);
    buffer_append_int16(data, mc_interface_read_reset_avg_input_current() * 10, ind);
    data[(*ind)++] = (uint8_t)(GET_INPUT_VOLTAGE() * 10.0);
    data[(*ind)++] = (int8_t)(NTC_TEMP(ADC_IND_TEMP_MOTOR));
    data[(*ind)++] = (int8_t)(NTC_TEMP(ADC_IND_TEMP_INVRT));
    data[(*ind)++] = mc_interface_get_fault();
    buffer_append_uint16(data, mc_interface_get_amp_hours(false) * 1000, ind);
    buffer_append_uint16(data, mc_interface_get_amp_hours_charged(false) * 1000, ind);
}

uint8_t stata[10];
static void MakeStatusInfoPacket(uint8_t *data, int32_t *ind)
{
    *ind = 0;
    data[(*ind)++] = 1; //packet id
    data[(*ind)++] = (uint8_t)(GET_CELL1()*100.0) - 200;
    data[(*ind)++] = (uint8_t)(GET_CELL2()*100.0) - 200;
    data[(*ind)++] = (uint8_t)(GET_CELL3()*100.0) - 200;
    data[(*ind)++] = (uint8_t)(GET_CELL4()*100.0) - 200;
    data[(*ind)++] = (uint8_t)(GET_CELL5()*100.0) - 200;
    data[(*ind)++] = (uint8_t)(GET_CELL6()*100.0) - 200;

    for(uint8_t i=0; i<6; i++)
        stata[i] = data[i+1];
}

static int NrfTxSender(uint8_t *data, uint8_t len)
{
    int res;

    //while((res = rfhelp_send_data_crc(data, len)) != 0);
    res = rfhelp_send_data_crc(data, len);

    if(res == 0)
    {
        nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
    }

    return res;
}

static THD_FUNCTION(tx_thread, arg)
{
    (void) arg;

    chRegSetThreadName("Nrf TX Thread");

    for(;;)
    {
        nrf_restart_tx_time = NRF_RESTART_TIMEOUT;

        if(++nosend_cnt >= TX_FREQ_INTERVAL)
        {
            uint8_t TxArray[MAX_PL_LEN];
            int32_t len;

            MakeStatusPacket(TxArray, &len);

            if(NrfTxSender(TxArray, len) == 0)
                asm("nop");

            static uint8_t dlyCnt = 0;
            if(++dlyCnt >= 10)
            {
                dlyCnt = 0;

                MakeStatusInfoPacket(TxArray, &len);

                if(NrfTxSender(TxArray, len) == 0)
                    asm("nop");
            }

            nosend_cnt = 0;
        }

        if(nrf_restart_rx_time > 0 && nrf_restart_tx_time > 0)
        {
            nrf_restart_rx_time -= 1;
            nrf_restart_tx_time -= 1;
        }
        else
        {
            rfhelp_power_up();
            rfhelp_restart();
            nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
            nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
        }

        chThdSleepMilliseconds(1);
    }
}


static void UpdateControl(uint8_t Thr, uint8_t Brk, uint8_t inverse, uint8_t frameType, float limit)
{
    mc_control_mode mode = frameType;

    float throttle = Thr;

    if(Thr > 10)
    {
        float mappedValue = ((inverse != 0) ? -1.0 : 1.0) * fmap(throttle, 0.0, 100.0, 0, limit);

        switch(mode)
        {
        case CONTROL_MODE_DUTY:
            mc_interface_set_duty(mappedValue);
            break;

        case CONTROL_MODE_SPEED:
            mc_interface_set_pid_speed(mappedValue);
            break;

        case CONTROL_MODE_CURRENT:
            mc_interface_set_current(mappedValue);
            break;

        default:
            break;
        }
    }
    else if(Brk > 10)
    {
        float brake = Brk;
        float mappedValueBrk = fmap(brake, 0.0, 100.0, 0, limit);
        mc_interface_set_brake_current(mappedValueBrk);
    }
    else
    {
        mc_interface_release_motor();
    }

    timeout_reset();
}


static void processInputFrame(uint8_t *buf, uint8_t len)
{
    switch(buf[0])
    {
    case 0:
        //controls
        if(len == 8)
        {
            int32_t ind = 1;
            uint8_t invertFlag = (buf[ind] & (1 << 7)) ? 1 : 0;
            uint8_t accel = buf[ind++] & 0x7F;
            uint8_t brake = buf[ind++];
            uint8_t frameType = buf[ind++];
            float lim = buffer_get_float32(buf, 1000.0, &ind);

            UpdateControl(accel, brake, invertFlag, frameType, lim);
        }
        break;

    default:
        break;
    }
}

static THD_FUNCTION(rx_thread, arg)
{
    (void) arg;

    chRegSetThreadName("Nrf RX Thread");

    for(;;)
    {
        //wait IRQ
        msg_t msg = chBSemWaitTimeout(&nrf_bsem, MS2ST(50));

        if(msg == MSG_TIMEOUT)
            continue;

        uint8_t buf[32], len;

        chMtxLock(&rf_mutex);
        uint8_t nrf_status = rf_status();
        chMtxUnlock(&rf_mutex);

        if(NRF_STATUS_GET_RX_P_NO(nrf_status) != 7)
        {
            static uint8_t pipe = 0;
            int res = rfhelp_read_rx_data_crc(buf, &len, &pipe);

            if(res > 0)
            {
                rf_flush_rx();
            }

            if(res >= 0)
            {
                processInputFrame(buf, len);
                nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
            }
        }
    }
}

//
//
//			;
//
//			static uint8_t xx = 0, yy = 0;
//
//			//chuck_data cdata;
//			int32_t ind = 0;
//			int buttons;
//
//			// If something was read
//			if(res >= 0)
//			{
////				DebugSendString("\n>>SMTH WAS READED");
////				DebugSendChar('\n');
////				DebugSendChar('-');
//
//				//DebugSendNumWDesc("Thr = ", buf[1]);
//				//DebugSendNumWDesc("Brk = ", buf[0]);
////				uint32_t cur = ST2MS(chVTGetSystemTime());
////				static uint32_t prev = 0;
////
////				//DebugSendNumWDesc("> ", cur-prev);
////				prev = cur;
//
//				UpdateControl(buf[1], buf[0]);
//
//				nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
//
//				break;
//
////				for(uint8_t i=0; i<32; i++)
////					DebugSendNumSpace(buf[i]);
////				MOTE_PACKET packet = buf[0];
////
//
////
////				switch (packet) {
////				case MOTE_PACKET_BATT_LEVEL:
////					// TODO!
////					break;
////
////				case MOTE_PACKET_BUTTONS:
////					ind = 1;
////					mstate.js_x = buf[ind++];
////					mstate.js_y = buf[ind++];
////					buttons = buf[ind++];
////					mstate.bt_c = buttons & (1 << 0);
////					mstate.bt_z = buttons & (1 << 1);
////					mstate.bt_push = buttons & (1 << 2);
////					mstate.vbat = (float)buffer_get_int16(buf, &ind) / 1000.0;
////
////					cdata.js_x = 255 - mstate.js_x;
////					cdata.js_y = mstate.js_y;
////					cdata.bt_c = mstate.bt_c;
////					cdata.bt_z = mstate.bt_z;
////
////					app_nunchuk_update_output(&cdata);
////					break;
////
////				case MOTE_PACKET_FILL_RX_BUFFER:
////					memcpy(rx_buffer + buf[1], buf + 2, len - 2);
////					break;
////
////				case MOTE_PACKET_FILL_RX_BUFFER_LONG: {
////					int rxbuf_ind = (unsigned int)buf[1] << 8;
////					rxbuf_ind |= buf[2];
////					if (rxbuf_ind < RX_BUFFER_SIZE) {
////						memcpy(rx_buffer + rxbuf_ind, buf + 3, len - 3);
////					}
////				}
////				break;
////
////				case MOTE_PACKET_PROCESS_RX_BUFFER: {
////					ind = 1;
////					int rxbuf_len = (unsigned int)buf[ind++] << 8;
////					rxbuf_len |= (unsigned int)buf[ind++];
////
////					if (rxbuf_len > RX_BUFFER_SIZE) {
////						break;
////					}
////
////					uint8_t crc_high = buf[ind++];
////					uint8_t crc_low = buf[ind++];
////
////					memcpy(rx_buffer + rxbuf_len - (len - ind), buf + ind, len - ind);
////
////					if (crc16(rx_buffer, rxbuf_len)
////							== ((unsigned short) crc_high << 8
////									| (unsigned short) crc_low)) {
////
////						// Wait a bit in case retries are still made
////						chThdSleepMilliseconds(2);
////
////						commands_set_send_func(nrf_driver_send_buffer);
////						commands_process_packet(rx_buffer, rxbuf_len);
////					}
////				}
////				break;
////
////				case MOTE_PACKET_PROCESS_SHORT_BUFFER:
////					// Wait a bit in case retries are still made
////					chThdSleepMilliseconds(2);
////
////					commands_set_send_func(nrf_driver_send_buffer);
////					commands_process_packet(buf + 1, len - 1);
////					break;
////
////				default:
////					break;
////				}
//			}
//
//			chThdSleepMilliseconds(1);
//		}
//		//chThdSleepMilliseconds(5);
//
//		// Restart the nrf if nothing has been received for a while
////		if(nrf_restart_rx_time > 0 && nrf_restart_tx_time > 0)
////		{
////			nrf_restart_rx_time -= 5;
////			nrf_restart_tx_time -= 5;
////		}
////		else
////		{
////			//DebugSendString("@Restarting NRF...");
////			//DebugSendChar('^');
////			rfhelp_power_up();
////			rfhelp_restart();
////			nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
////			nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
////
//////			DebugSendNumWDesc("\nconf=", rf_read_reg_byte(NRF_REG_CONFIG));
//////			DebugSendNumWDesc("rf-stp=", rf_read_reg_byte(NRF_REG_RF_SETUP));
//////			DebugSendNumWDesc("a-w=", rf_read_reg_byte(NRF_REG_SETUP_AW));
//////			DebugSendNumWDesc("ch=", rf_read_reg_byte(NRF_REG_RF_CH));
//////			DebugSendNumWDesc("sts=", rf_read_reg_byte(NRF_REG_STATUS));
//////			DebugSendNumWDesc("RX_PW_P0=", rf_read_reg_byte(NRF_REG_RX_PW_P0));
//////			DebugSendNumWDesc("DYNPD=", rf_read_reg_byte(NRF_REG_DYNPD));
//////			DebugSendNumWDesc("FEATURE=", rf_read_reg_byte(NRF_REG_FEATURE));
////
//////			uint8_t stst[8];
//////			rf_read_reg(NRF_REG_RX_ADDR_P0, stst, 5);
//////			for(uint8_t i=0; i<5; i++)
//////				DebugSendNumWDesc("RX=", stst[i]);
////		}
//	}
//}

//void nrf_driver_send_buffer(unsigned char *data, unsigned int len)
//{
//    uint8_t send_buffer[MAX_PL_LEN];
//
//    if(len <= (MAX_PL_LEN - 1))
//    {
//        uint32_t ind = 0;
//        send_buffer[ind++] = MOTE_PACKET_PROCESS_SHORT_BUFFER;
//        memcpy(send_buffer + ind, data, len);
//        ind += len;
//        NrfTxSender(send_buffer, ind);
//        nosend_cnt = 0;
//    }
//    else
//    {
//        unsigned int end_a = 0;
//        unsigned int len2 = len - (MAX_PL_LEN - 5);
//
//        for(unsigned int i = 0; i < len2; i += (MAX_PL_LEN - 2))
//        {
//            if(i > 255)
//            {
//                break;
//            }
//
//            end_a = i + (MAX_PL_LEN - 2);
//
//            uint8_t send_len = (MAX_PL_LEN - 2);
//            send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER;
//            send_buffer[1] = i;
//
//            if((i + (MAX_PL_LEN - 2)) <= len2)
//            {
//                memcpy(send_buffer + 2, data + i, send_len);
//            }
//            else
//            {
//                send_len = len2 - i;
//                memcpy(send_buffer + 2, data + i, send_len);
//            }
//
//            NrfTxSender(send_buffer, send_len + 2);
//            nosend_cnt = 0;
//        }
//
//        for(unsigned int i = end_a; i < len2; i += (MAX_PL_LEN - 3))
//        {
//            uint8_t send_len = (MAX_PL_LEN - 3);
//            send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER_LONG;
//            send_buffer[1] = i >> 8;
//            send_buffer[2] = i & 0xFF;
//
//            if((i + (MAX_PL_LEN - 3)) <= len2)
//            {
//                memcpy(send_buffer + 3, data + i, send_len);
//            }
//            else
//            {
//                send_len = len2 - i;
//                memcpy(send_buffer + 3, data + i, send_len);
//            }
//
//            NrfTxSender((char*) send_buffer, send_len + 3);
//            nosend_cnt = 0;
//        }
//
//        uint32_t ind = 0;
//        send_buffer[ind++] = MOTE_PACKET_PROCESS_RX_BUFFER;
//        send_buffer[ind++] = len >> 8;
//        send_buffer[ind++] = len & 0xFF;
//        unsigned short crc = crc16(data, len);
//        send_buffer[ind++] = (uint8_t)(crc >> 8);
//        send_buffer[ind++] = (uint8_t)(crc & 0xFF);
//        memcpy(send_buffer + 5, data + len2, len - len2);
//        ind += len - len2;
//
//        NrfTxSender((char*) send_buffer, ind);
//        nosend_cnt = 0;
//    }
//}
