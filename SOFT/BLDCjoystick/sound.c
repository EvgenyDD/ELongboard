/*******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2015 Evgeny Dolgalev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICUAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "sound.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "hw_hal.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum {FALSE=0, TRUE=!FALSE} bool;


/* Private define ------------------------------------------------------------*/
#define BUF_SIZE	150 //sound circular buffer size



/* Note frequencies (in Hz)
 * from C - low octave
 * till H - 4th octale
 * octave length - 12
 */
const uint16_t NoteMass[OCTAVE*5] =
{
	/*131*/0, 139, 148, 156, 165, 175, 185, 196, 207, 220, 233, 247,
	262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
	523, 554, 587, 622, 659, 698, 740, 784, 830, 880, 932, 988,
	1046,1109,1174,1244,1318,1397,1480,1568,1661,1720,1865,1975,
	2093,2217,2349,2489,2637,2794,2960,3136,3332,3440,3729,3951
};

#define BPM 	(180)    		// you can change this value changing all the others
#define LEN_Q 	(60000/BPM) 	// quarter 1/4
#define LEN_H 	(2*LEN_Q) 		// half 2/4
#define LEN_E 	(LEN_Q/2)   	// eighth 1/8
#define LEN_S 	(LEN_Q/4) 		// sixteenth 1/16
#define LEN_W 	(4*LEN_Q)		// whole 4/4
#define LEN_ES	(LEN_E + LEN_S)

const uint16_t StarWarsTheme[137][2] =
{
	{ A + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ A + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ A + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ A + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ A + 3 * OCTAVE, LEN_H },
	{ DELAY, LEN_H },

	{ E + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ E + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ E + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 4 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ Gd + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ A + 3 * OCTAVE, LEN_H },
	{ DELAY, LEN_H },

	{ A + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ A + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ A + 3 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ A + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ Gd + 4 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ G + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ Fd + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ E + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ F + 4 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ DELAY, LEN_E },
	{ Ad + 3 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ Dd + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ D + 4 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ Cd + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ B + 3 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ C + 4 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ DELAY, LEN_E },
	{ F + 3 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ Gd + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ A + 3 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ C + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ A + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ E + 4 * OCTAVE, LEN_H },
	{ DELAY, LEN_H },

	{ A + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ A + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ A + 3 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ A + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ Gd + 4 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ G + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ Fd + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ E + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ F + 4 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ DELAY, LEN_E },
	{ Ad + 3 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ Dd + 4 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ D + 4 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ Cd + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ B + 3 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ C + 4 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ DELAY, LEN_E },
	{ F + 3 * OCTAVE, LEN_E },
	{ DELAY, LEN_E },
	{ Gd + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },

	{ A + 3 * OCTAVE, LEN_Q },
	{ DELAY, LEN_Q },
	{ F + 3 * OCTAVE, LEN_ES },
	{ DELAY, LEN_ES },
	{ C + 4 * OCTAVE, LEN_S },
	{ DELAY, LEN_S },
	{ A + 3 * OCTAVE, LEN_H },
	{ DELAY, LEN_H },

	{ DELAY, LEN_H }
};


/* Private macro -------------------------------------------------------------*/
#define abs(x)  ( (x)<0 ) ? (-(x)) : (x)


/* Private variables ---------------------------------------------------------*/
volatile uint16_t soundTimer = 0;

bool noteState = FALSE; //playing/not playing

struct SoundCBType SoundCB[BUF_SIZE]; //circular buffer
uint8_t cbStart = 0, cbEnd = 0; //and it's indexes


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void PlayStarWars()
{
	for(uint8_t i=0; i<137; i++)
	{
		SoundPlayNote(StarWarsTheme[i][0], StarWarsTheme[i][1]);
	}
}
/*******************************************************************************
* Function Name  : SoundInit
* Description    : Initialize sound module
*******************************************************************************/
void SoundInit()
{

}



/*******************************************************************************
* Function Name  : SoundPlayNote
* Description    : Add note to circular buffer
*******************************************************************************/
int SoundPlayNote(uint16_t note, uint16_t noteLen)
{
	assert_param(note <= 12*5);

	SoundCB[cbEnd].noteFreq = NoteMass[note];
	SoundCB[cbEnd].noteLen = noteLen;
	noteState = TRUE;

	if(++cbEnd == BUF_SIZE) cbEnd = 0;
	return 0;
}



/*******************************************************************************
* Function Name  : SoundDispatcher
* Description    : Process sound playing
*******************************************************************************/
void SoundDispatcher()
{
	if(soundTimer)
		soundTimer--;

	if(noteState && !soundTimer)
	{
		if(cbStart != cbEnd)
		{ //take new note from buffer

			soundTimer = SoundCB[cbStart].noteLen;
			Sound(SoundCB[cbStart].noteFreq);

			if(++cbStart == BUF_SIZE) cbStart = 0;
			noteState = TRUE;
		}
		else
		{ //stop sound playing = no new data in buffer
			noteState = FALSE;
			Sound(0);
		}
	}
}
