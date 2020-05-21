/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
__IO uint32_t count_out = 0;
uint32_t count_in = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void EP3_OUT_Callback()
{
//	unsigned char* yc = buffer_out;
	unsigned i;
	/*
	 void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
	 uint32_t n = (wNBytes + 1) >> 1; // /2
	 uint32_t i;
	 uint32_t *pdwVal;

	 pdwVal = (uint32_t *)(wPMABufAddr * 2 + PMAAddr);
	 for (i = n; i != 0; i--)
	 {
	 *(uint16_t*)pbUsrBuf++ = *pdwVal++;
	 pbUsrBuf++;
	 }
	 */
	count_out += i = GetEPRxCount(ENDP3);
	PMAToUserBufferCopy(buffer_out, ENDP3_RXADDR, i);

//	if(count_out < 255)
//	{
//		for(; i != 0; i--)
//		{
//			*yy++ = *yc++;
//		}
//	}
//  sdvig_usb+=count_out;
	// count_out=0;
	SetEPRxValid(ENDP3);

//	static uint8_t sss = 0;

//	DisplayNumAtPos(sss++, 0);
//
//	if(sss > 99)
//		sss = 0;
}


/*******************************************************************************
 * Function Name  : EP1_IN_Callback
 * Description    :
 *******************************************************************************/
void EP1_IN_Callback()
{
	count_in = 0;
}
