/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MATH__H
#define MATH__H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
float pow_(float, float);
float log10_(int);
float sin_(float);
float cos_(float);
float tan_(float);
float factorial_(double);
float sqrt_(float);

void MultStrings(uint8_t*, uint8_t*);

#endif //MATH__H
