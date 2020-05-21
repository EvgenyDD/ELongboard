/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_crc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_pwr.h"
#include "misc.h"
#include "hw_hal.h"

#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern uint32_t buzzDelay, soundDelay;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : InitPeriph
 * Description    : Initialize peripheral
 *******************************************************************************/
void InitPeriph()
{
	RCC_DeInit();
//	RCC_HSEConfig(RCC_HSE_ON);
//	ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
//	if(HSEStartUpStatus == SUCCESS)
//	{
	// Enable Prefetch Buffer
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	// Flash 2 wait state
	FLASH_SetLatency(FLASH_Latency_2);

	// HCLK = SYSCLK
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	// PCLK2 = HCLK
	RCC_PCLK2Config(RCC_HCLK_Div1);
	// PCLK1 = HCLK /2
	RCC_PCLK1Config(RCC_HCLK_Div2);
	// ADC Clock
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	// PLL Clock
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12); //9

	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != 0x08);

	RCC_APB2PeriphClockCmd(
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
		RCC_APB2Periph_GPIOD | /*RCC_AHBPeriph_CRC|*/RCC_APB2Periph_TIM1 |
		RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1,
		ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

//	RCC_AHBPeriphClockCmd(RCC_AHBENR_CRCEN, ENABLE);

	//System Clock = (72Mhz) / 36000 = 2000Hz = 0.5ms reload
	SysTick_Config(48000); //1000us

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	HALLDisable();

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = /*GPIO_Pin_2 | */GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	GPIO_SetBits(GPIOA, GPIO_Pin_3);


	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; 			// Single Channel
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 	// Scan on Demand
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_TempSensorVrefintCmd(ENABLE);

	TIM_TimeBaseInitTypeDef TimBaseStruct;
	TimBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimBaseStruct.TIM_Period = 40;
	TimBaseStruct.TIM_Prescaler = 4-1;
	TimBaseStruct.TIM_ClockDivision = 0;
	TimBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TimBaseStruct);

	TIM_OCInitTypeDef TimOCInitStruct;
	TIM_OCStructInit(&TimOCInitStruct);
	TimOCInitStruct.TIM_Pulse = 20;
	TimOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TimOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TimOCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
	TimOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TimOCInitStruct.TIM_OCNPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TimOCInitStruct);

	TIM_BDTRInitTypeDef TIMBDTRInitStruct;
	TIM_BDTRStructInit(&TIMBDTRInitStruct);
	TIMBDTRInitStruct.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
	TIM_BDTRConfig(TIM1, &TIMBDTRInitStruct);

	TIM_Cmd(TIM1, ENABLE);

	//TIM1->CCR1 = 100;

	TimBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimBaseStruct.TIM_Period = 100;
	TimBaseStruct.TIM_Prescaler = 10;
	TimBaseStruct.TIM_ClockDivision = 0;
	TimBaseStruct.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM2, &TimBaseStruct);
	TIM_TimeBaseInit(TIM3, &TimBaseStruct);

	TIM_OCStructInit(&TimOCInitStruct);
	TimOCInitStruct.TIM_Pulse = 0;
	TimOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TimOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TimOCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
	TimOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TimOCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;

	TIM_OC2Init(TIM3, &TimOCInitStruct);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM3, &TimOCInitStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);

//	TIM3->CCR1 = 10;

//	TIM_OC3Init(TIM2, &TimOCInitStruct);
//	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM2, &TimOCInitStruct);
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

	//sound
	TimOCInitStruct.TIM_Pulse = 10;
	TimOCInitStruct.TIM_OCMode = TIM_OCMode_Toggle;
	TimOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TimOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM2, &TimOCInitStruct);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

	/* Sound */
//	TIM_OCInitTypeDef TIM_OCInitStructure2;
//	TIM_OCInitStructure2.TIM_OCMode = TIM_OCMode_PWM1;
//	TIM_OCInitStructure2.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure2.TIM_OCPolarity = TIM_OCPolarity_High;
//	TIM_OCInitStructure2.TIM_Pulse = 50; //?
//	TIM_OC2Init(TIM3, &TIM_OCInitStructure2);
//
//	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_ARRPreloadConfig(TIM3, ENABLE);


	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM2, ENABLE);


	TIM2->CCR2 = 0;
	//TIM2->CCR3 = 0;
	TIM2->CCR4 = 0;

	TimBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimBaseStruct.TIM_Period = 100;
	TimBaseStruct.TIM_Prescaler = 0;
	TimBaseStruct.TIM_ClockDivision = 0;
	TimBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TimBaseStruct);
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI1, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

	TIM_Cmd(TIM4, ENABLE);

	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource1);

	EXTI_InitTypeDef EXTIInitStructure;
	EXTIInitStructure.EXTI_Line = EXTI_Line1;
	EXTIInitStructure.EXTI_LineCmd = ENABLE;
	EXTIInitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTIInitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTIInitStructure);


    /* Add IRQ vector to NVIC */
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

	//USB Init
//	Set_System();
//	Set_USBClock();
//	USB_Interrupts_Config();
//	USB_Init();

}

/*******************************************************************************
 * Function Name  : GetADCConv
 *******************************************************************************/
static uint16_t GetADCChannelValue(uint8_t channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_28Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) );
	return ADC_GetConversionValue(ADC1);
}

/*******************************************************************************
 * Function Name  : GetThrottle
 *******************************************************************************/
uint16_t GetThrottle()
{
	uint16_t result = GetADCChannelValue(ADC_Channel_9);
	return result;
}

/*******************************************************************************
 * Function Name  : GetBrake
 *******************************************************************************/
uint16_t GetBrake()
{
	uint16_t result = GetADCChannelValue(ADC_Channel_8);
	return result;
}

/*******************************************************************************
 * Function Name  : GetTemperature
 *******************************************************************************/
int16_t GetTemperatureChip()
{
	return (3529 - ((30 * GetADCChannelValue(ADC_Channel_TempSensor)) >> 4))/10;
}

/*******************************************************************************
 * Function Name  : GetVBatMV
 *******************************************************************************/
int16_t GetVBatMV()
{
	return GetADCChannelValue(ADC_Channel_2) * 2226 / 1000 - 1862 - 3 / 10;
	//2.226885001*x-1862.295861
}

/*******************************************************************************
 * Function Name  : HALLEnable
 * Description    : Enable power for hall sensors
 *******************************************************************************/
void HALLEnable()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

/*******************************************************************************
 * Function Name  : HALLDisable
 * Description    : Disable power for hall sensors
 *******************************************************************************/
void HALLDisable()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}


/*******************************************************************************
 * Function Name  : HapticOn
 *******************************************************************************/
void HapticOn()
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/*******************************************************************************
 * Function Name  : HapticOff
 *******************************************************************************/
void HapticOff()
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}


/*******************************************************************************
 * Function Name  : HapticBuzz
 *******************************************************************************/
void HapticBuzz(uint32_t ms)
{
	HapticOn();
	buzzDelay = ms;
}


/*******************************************************************************
 * Function Name  : IsCharging
 *******************************************************************************/
uint8_t IsCharging()
{
	return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0) == Bit_RESET;
}


/*******************************************************************************
* Function Name  : Sound
* Description    : Make sound
* Input		 	 : Frequency
*******************************************************************************/
void Sound(uint16_t freq)
{
	GPIO_InitTypeDef GPIOInitStructure;
	if(freq == 0)
	{
		GPIOInitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIOInitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIOInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIOInitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		return;
	}

	GPIOInitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIOInitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIOInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIOInitStructure);

//48mhz base

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = 480-1;//100khz
	TIM_TimeBaseStructure.TIM_Period = 50000/freq;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
}


/*******************************************************************************
 * Function Name  : Beep
 * Description    : Beep with frequency ms length
 *******************************************************************************/
void Beep(uint16_t freq, uint32_t len)
{
	//Sound(freq);
	soundDelay = len;
}
