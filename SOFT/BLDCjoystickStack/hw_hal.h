/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HW_HAL_H
#define HW_HAL_H


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
#define COM_NRF_BRAKE 		(1 << 0)
#define COM_NRF_DUTY 		(1 << 1)
#define COM_NRF_CURR		(1 << 2)
#define COM_NRF_SPEED		(1 << 3)
#define COM_NRF_SMRT_SPEED	(1 << 4)

#define COM_NRF_OFF			(1 << 6)
#define COM_NRF_INV			(1 << 7)


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define UTILS_LP_FAST(value, sample, filter_constant)	(value -= (filter_constant) * (value - (sample)))


/* Exported define -----------------------------------------------------------*/
#define SHDN_DELAY_MS	(1000*15)


/* Exported functions ------------------------------------------------------- */
void InitPeriph();

void HALLDisable();
void HALLEnable();

uint16_t GetThrottle();
uint16_t GetBrake();
int16_t GetTemperatureChip();
int16_t GetVBatMV();


void HapticOn();
void HapticOff();
void HapticBuzz(uint32_t ms);
uint8_t IsCharging();

void Sound(uint16_t freq);
void Beep(uint16_t freq, uint32_t len);


void delay_ms(uint32_t nTime);

#endif //HW_HAL_H
