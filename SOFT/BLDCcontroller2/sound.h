/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SOUND_H
#define SOUND_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported types ------------------------------------------------------------*/
struct SoundCBType{
	uint16_t noteFreq;
	uint16_t noteLen;
};

/* Exported constants --------------------------------------------------------*/
enum Notes{C, Cd, D, Dd, E, F, Fd, G, Gd, A, Ad, B};
#define DELAY 	(0)
#define OCTAVE 	(12)

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SoundInit(void);


int SoundPlayNote(uint16_t note, uint16_t noteLen);
void PlayStarWars(void);
void SoundDispatcher(void);

#endif //SOUND_H
