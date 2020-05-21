/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MACROS_H
#define MACROS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
//#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
//#include "stm32f10x_spi.h"
//#include "stm32f10x_i2c.h"
//#include "stm32f10x_tim.h"
//#include "stm32f10x_usart.h"


#include "string.h"

//#include "menu.h"
#include "macros.h"


//#include "debug.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (1<<(m)))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)

#define swap(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void delay_ms(uint32_t nTime);
void delay_500us(uint32_t nTime);


void ClearBuffer(uint8_t *, uint16_t, char);



#endif //MACROS_H
