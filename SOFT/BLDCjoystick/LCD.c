/* Includes ------------------------------------------------------------------*/
#include "LCD.h"
#include "stdbool.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LCD_ADDRESS 0x78 //or 0x3D or 0x3C
#define BQ_ADDRESS 	0xAA //or 0x3D or 0x3C

//101x33

#define _PD 	(1<<7)
#define _V 		(1<<2)
#define _MX 	(1<<4)
#define _MY 	(1<<3)
#define _TRS 	(1<<1)
#define _BRS 	(1<<0)
#define _DO 	(1<<2)
#define _PRS 	(1<<0)
#define _D 		(1<<2)
#define _E 		(1<<0)
//101x33

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t LCDConfig = 0;

static uint8_t LCDBuffer[505];

bool startWas = false;

struct _current_font
{
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
	uint8_t inverse;
} cfont;

I2C_InitTypeDef I2C_InitStructure;

uint8_t inverseMode = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void reload()
{
	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);
	I2C_DeInit(I2C1);
				delay_ms(2);
				I2C_Init(I2C1, &I2C_InitStructure);
			I2C_Cmd(I2C1, ENABLE);
}
/*******************************************************************************
 * Function Name  : I2C_Start
 *******************************************************************************/
static void I2C_Start()
{
	int32_t failcnt = 150;
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && (failcnt-- > 0));
	if(failcnt == 0) {reload();startWas = false;return;}
	I2C_GenerateSTART(I2C1, ENABLE);
	failcnt = 150;
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && (failcnt-- > 0));
	if(failcnt == 0) {reload();startWas = false;return;}

	startWas = true;
}


///*******************************************************************************
// * Function Name  : I2C_Start_Repeated
// *******************************************************************************/
//static void I2C_Start_Repeated()
//{
//	I2C_GenerateSTART(I2C1, ENABLE);
//	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
//}

/*******************************************************************************
 * Function Name  : I2C_SendAddr
 *******************************************************************************/
static void I2C_SendAddr()
{
	if(startWas == false)return;
	I2C_Send7bitAddress(I2C1, LCD_ADDRESS, I2C_Direction_Transmitter);
	int32_t failcnt = 150;
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && (failcnt-- > 0));
	if(failcnt == 0) {reload();startWas = false;return;}
}

///*******************************************************************************
// * Function Name  : I2C_SendAddr
// *******************************************************************************/
//static void I2C_SendAddrBq()
//{
//	I2C_Send7bitAddress(I2C1, BQ_ADDRESS, I2C_Direction_Receiver);
//	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
//	I2C_Cmd(I2C1, ENABLE);
//}
//
///*******************************************************************************
// * Function Name  : I2C_SendAddrBqWr
// *******************************************************************************/
//static void I2C_SendAddrBqWr()
//{
//	I2C_Send7bitAddress(I2C1, BQ_ADDRESS, I2C_Direction_Transmitter);
//	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
//	I2C_Cmd(I2C1, ENABLE);
//}

/*******************************************************************************
 * Function Name  : I2C_SendByte
 *******************************************************************************/
static void I2C_SendByte(uint8_t data)
{
	if(startWas == false)return;
	I2C_SendData(I2C1, data);
	int32_t failcnt = 150;
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && (failcnt-- > 0) );
	if(failcnt == 0) {reload();startWas = false;return;}
}

///*******************************************************************************
// * Function Name  : I2C_ReadByteNack
// *******************************************************************************/
//static uint8_t I2C_ReadByteNack()
//{
//	I2C_AcknowledgeConfig(I2C1, DISABLE);
//	I2C_GenerateSTOP(I2C1, ENABLE);
//	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
//	return I2C_ReceiveData(I2C1);
//}

/*******************************************************************************
 * Function Name  : I2C_Stop
 *******************************************************************************/
static void I2C_Stop()
{
	if(startWas == false)return;
	I2C_GenerateSTOP(I2C1, ENABLE);
	int32_t failcnt = 150;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) && (failcnt-- > 0));
	if(failcnt == 0) {reload();startWas = false;return;}
}

/*******************************************************************************
 * Function Name  : LCD_WriteComBasic
 * Description    : Write command from basic set
 *******************************************************************************/
static void LCD_WriteComBasic(uint8_t data)
{
	I2C_Start();
	I2C_SendAddr();
	I2C_SendByte(0x80);
	I2C_SendByte(0x20 | LCDConfig);
	I2C_Stop();

	I2C_Start();
	I2C_SendAddr();
	I2C_SendByte(0x80);
	I2C_SendByte(data);
	I2C_Stop();
}

/*******************************************************************************
 * Function Name  : LCD_WriteComExtended
 * Description    : Write command from extended set
 *******************************************************************************/
static void LCD_WriteComExtended(uint8_t data)
{
	I2C_Start();
	I2C_SendAddr();
	I2C_SendByte(0x80);
	I2C_SendByte(0x20 | LCDConfig | 1);
	I2C_Stop();

	I2C_Start();
	I2C_SendAddr();
	I2C_SendByte(0x80);
	I2C_SendByte(data);
	I2C_Stop();
}

/*******************************************************************************
 * Function Name  : LCD_InitOn
 * Description    : Turn LCD On and Clear screen
 *******************************************************************************/
void LCD_InitOn()
{
	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* I2C1 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	/* Configure I2C1 pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* I2C1 Init */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 100000;
	I2C_Init(I2C1, &I2C_InitStructure);

	/* GPIOB Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

	I2C_Cmd(I2C1, ENABLE);

	for(uint16_t i = 0, f = 1; i < 505; i++, f++)
		LCDBuffer[i] = f;

	/* I2C1 Init */

	LCDConfig = /*_PD | _MX | _MY | _V |*/ 0;

	for(uint32_t i = 0; i < 0x100000; i++)
		asm("nop");

	LCD_WriteComBasic(0x08 | _D /*| _E*/);
	LCD_WriteComBasic(0x04 | 0x02); //??
	LCD_WriteComExtended(0x10 | 5); //BIAS SYSTEM
	LCD_WriteComBasic(0x40); // Y address - 0
	LCD_WriteComBasic(0x80); // X address - 0

	LCD_Fill();
	LCD_Update();

//	for(uint32_t i = 0; i < 0x100000; i++)
//		asm("nop");
//	LCD_Clear();
//
//	LCD_Update();
//
//	LCD_Pixel(10,10,1);
//	LCD_Pixel(32,32,1);
//
//	LCD_Pixel(32,100,1);
//extern const unsigned char font5x8[], font3x5[], fontSTD_numSevenSegNumFont[], fontSTD_DotMatrix_M[];
//	LCD_setFont(font5x8);
//	LCD_MultChar(0,0,'A', 2);
//	//LCD_setFont(font3x5);
//	LCD_String(0,17, "ABC", 1);
//
//	LCD_setFont(fontSTD_DotMatrix_M);
//	LCD_BigString(0,30, "01", 1);
//
//	LCD_Update();
}

/*******************************************************************************
 * Function Name  : LCD_Off
 *******************************************************************************/
void LCD_Off()
{
	LCDConfig |= _PD;
	LCD_WriteComBasic(0); //?
}


/*******************************************************************************
 * Function Name  : LCD_Fill
 *******************************************************************************/
void LCD_Fill()
{
	for(uint16_t i=0; i<505; i++)
	{
		LCDBuffer[i] = 0xFF;
//		if(i%10 == 0)
//			LCDBuffer[i] = 0x00;
	}
}

/*******************************************************************************
 * Function Name  : LCD_Clear
 *******************************************************************************/
void LCD_Clear()
{
	for(uint16_t i=0; i<505; i++)
		LCDBuffer[i] = 0x00;
}



/*******************************************************************************
 * Function Name  : LCD_Update
 *******************************************************************************/
void LCD_Update()
{
	for(uint8_t i = 0; i < 5; i++)
	{
		I2C_Start();
		I2C_SendAddr();
		I2C_SendByte(0x40 + i);
		for(uint8_t j = 0; j < 101; j++)
			I2C_SendByte(LCDBuffer[i * 101 + j]);
		I2C_Stop();
	}
}

/*******************************************************************************
 * Function Name  : LCD_Pixel
 *******************************************************************************/
void LCD_Pixel(uint8_t x, uint8_t y, uint8_t color)
{
	if(x>32 || y>100) return;

	BitWrite(color, LCDBuffer[(x/8)*101 + y], x % 8);
}

/*******************************************************************************
 * Function Name  : LCD_Pixel
 *******************************************************************************/
void LCD_PixelInvert(uint8_t x, uint8_t y)
{
	if(x>32 || y>100) return;

	BitWrite(BitIsReset(LCDBuffer[(x/8)*101 + y], x % 8), LCDBuffer[(x/8)*101 + y], x % 8);
}

/*******************************************************************************
 * Function Name  : LCD_HLine
 *******************************************************************************/
void LCD_HLine(uint8_t x, uint8_t y, uint8_t width, uint8_t color)
{
	for(uint8_t i = 0; i < width; i++)
		LCD_Pixel(x + i, y, color);
}

/*******************************************************************************
 * Function Name  : LCD_VLine
 *******************************************************************************/
void LCD_VLine(uint8_t x, uint8_t y, uint8_t height, uint8_t color)
{
	for(uint8_t i = 0; i < height; i++)
		LCD_Pixel(x, y + i, color);
}

/*******************************************************************************
 * Function Name  : LCD_Rectangle
 *******************************************************************************/
void LCD_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
	LCD_VLine(x, y, height, color);
	LCD_VLine(x + width, y, height, color);
	LCD_HLine(x, y, width, color);
	LCD_HLine(x, y + height, width+1, color);
}

/*******************************************************************************
 * Function Name  : LCD_RectangleFill
 *******************************************************************************/
void LCD_RectangleFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
	for(uint8_t i = 0; i < width; i++)
		for(uint8_t j = 0; j < height; j++)
			LCD_Pixel(x + i, y + j, color);
}


/*******************************************************************************
 * Function Name  : LCD_RectangleInvert
 *******************************************************************************/
void LCD_RectangleInvert(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
	for(uint8_t i = 0; i < width; i++)
		for(uint8_t j = 0; j < height; j++)
			LCD_PixelInvert(x + i, y + j);
}

/*******************************************************************************
 * Function Name  : LCD_String
 *******************************************************************************/
void LCD_String(uint8_t x, uint8_t y, char* s, uint8_t scale)
{

	while(*s != '\0')
	{
		LCD_MultChar(x, y, *s, scale);

		if(inverseMode)
			LCD_VLine(x + cfont.x_size * scale, y, cfont.y_size, 1);

		x += cfont.x_size*scale + 1;

		s++;
	}
}

/*******************************************************************************
 * Function Name  : LCD_String
 *******************************************************************************/
void LCD_StringCentered(uint8_t x, uint8_t y, char* s, uint8_t scale)
{
	int len = strlen(s);
	if((x + len * (cfont.x_size + 1)) < 34)
		x += (34 - len * (cfont.x_size * scale + 1)) / 2;

	LCD_String(x,y,s,scale);
}

/*******************************************************************************
 * Function Name  : LCD_BigString
 *******************************************************************************/
void LCD_BigString(int x, int y, char *s, uint8_t scale)
{
	while(*s != '\0')
	{
		LCD_MultChar(x, y, *s++, scale);
		x += cfont.x_size * scale + 1;
	}
}

/*******************************************************************************
 * Function Name  : LCD_setFont
 *******************************************************************************/
void LCD_setFont(const uint8_t* font)
{
	cfont.font = (uint8_t*) font;
	cfont.x_size = cfont.font[0];
	cfont.y_size = cfont.font[1];
	cfont.offset = cfont.font[2];
	cfont.numchars = cfont.font[3];
	cfont.inverse = cfont.font[4];
}

/*******************************************************************************
 * Function Name  : LCD_Char
 * Description    : Display char without scaling
 *******************************************************************************/
void LCD_Char(uint8_t x, uint8_t y, char s)
{
	LCD_MultChar(x, y, s, 1);
}

/*******************************************************************************
 * Function Name  : LCD_MultChar
 * Description    : Display char with scaling
 *******************************************************************************/
void LCD_MultChar(uint8_t x, uint8_t y, char s, uint8_t scale)
{
#define FONT_WIDTH cfont.x_size
#define FONT_HEIGHT cfont.y_size

	if(scale == 0)
		scale = 1;

	uint32_t temp = ((s - cfont.offset) * FONT_WIDTH * (FONT_HEIGHT < 8 ? 8 : FONT_HEIGHT) / 8) + 5;

	if(BitIsSet(cfont.inverse, 7))
	{ //STD FONTS
		for(int16_t j = FONT_HEIGHT * scale - 1; j > -1; j--)
		{
			for(int8_t k = 0; k < FONT_WIDTH / 8; k++)
			{
				char frame = cfont.font[temp++];
				for(uint8_t i = 0; i < 8 * scale; i++)
				{
					if(BitIsSet(frame, 7 - i / scale))
						LCD_Pixel(x + k * 8 * scale + i, y + j, inverseMode?0:1);
					else
					{
						//if(!_transparent)
							LCD_Pixel(x + k * 8 * scale + i, y + j, inverseMode?1:0);
					}
				}
			}
			if(j % (scale) != 0)
				temp -= 2;
		}
	}
	else
	{ //NOT STD FONTS
		for(uint16_t j = 0; j < FONT_WIDTH * (FONT_HEIGHT / 8 + FONT_HEIGHT % 8 ? 1 : 0) * scale; j++)
		{
			char frame = cfont.font[temp];

			for(uint8_t i = 0; i < FONT_HEIGHT * scale; i++)
			{
				if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - i / scale) : (i / scale)))
					LCD_Pixel(x + j, y + i, inverseMode?0:1);
				else
				{
					//if(!_transparent)
						LCD_Pixel(x + j, y + i, inverseMode?1:0);
				}
			}
			if(j % scale == (scale - 1))
				temp++;
		}
	}

#if 0
	uint8_t t = x;
	x = y;
	y = t;

	char frame;
	uint16_t temp;

	if (!_transparent)
	{
		if (orient == PORTRAIT)
		{
			//setXY(x, y, x+cfont.x_size-1, y+cfont.y_size-1);

			temp = ((s - cfont.offset) * ((cfont.y_size / 8) * cfont.x_size))
			+ 5;

			for (uint16_t j = 0;
				j < ((cfont.y_size / 8) * cfont.x_size) * scale; j++)
			{
				frame = cfont.font[temp];

				for (uint8_t i = 0; i < 8 * scale; i++)
				{
					if (BitIsSet(frame,
							cfont.inverse ? (7 - i / scale) : (i / scale)))
					LCD_Pixel(x + i, y + j, WHITE);
					else
					LCD_Pixel(x + i, y + j, BLACK);
				}
				if (j % scale == (scale - 1))
				temp++;
			}
		}
		else
		{ //LANDSCAPE
			temp = ((s - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size))
			+ 5;

			for (uint16_t j = 0; j < ((cfont.x_size / 8) * cfont.y_size);
				j += (cfont.x_size / 8))
			{
				//setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));

				for (int zz = (cfont.x_size / 8) - 1; zz >= 0; zz--)
				{
					frame = cfont.font[temp + zz];
					for (uint8_t i = 0; i < 8; i++)
					{
						if ((frame & (1 << i)) != 0)
						LCD_Pixel(y + j, x + i, WHITE);
						else
						LCD_Pixel(y + j, x + i, BLACK);
					}
				}
				temp += (cfont.x_size / 8);
			}
		}
	}
	else
	{
		/*temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+5 ;
		 for(uint16_t j=0; j<cfont.y_size; j++)
		 {
		 for(int zz=0; zz<(cfont.x_size/8); zz++)
		 {
		 frame = cfont.font[temp+zz];
		 for(uint8_t i=0; i<8; i++)
		 {
		 setXY(x+i+(zz*8), y+j, x+i+(zz*8)+1, y+j+1);

		 if((frame & (1<<(7-i))) != 0)
		 LCD_Pixel(fillColor);
		 }
		 }
		 temp += (cfont.x_size/8);
		 }*/
	}
#endif
}

/*******************************************************************************
 * Function Name  : DisplayNum
 * Description    : Display Number
 *******************************************************************************/
void LCD_Num(uint8_t x, uint8_t y, int32_t num, uint8_t scale)
{
	char d[25];
	itoa_(num, d);
	LCD_String(x, y, d, scale);
}

/*******************************************************************************
 * Function Name  : DisplayNum
 * Description    : Display Number at specific position (0 is first)
 *******************************************************************************/
void LCD_Float(uint8_t x, uint8_t y, float num, uint8_t prec)
{
	char d[25];
	ftoa_(num, d, prec);
	LCD_String(x, y, d, 1);
}

void LCD_Float2Str(uint8_t x, uint8_t y, char* str1, float num, char* str2, uint8_t prec, uint8_t scale)
{
	char d[25], o[25];
	strcpy_(d, str1);
	ftoa_(num, o, prec);
	strcat_(d, o);
	strcat_(d, str2);
	LCD_String(x, y, d, 1);
}


/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void LCD_FloatPseudo(uint8_t x, uint8_t y, int32_t num, uint8_t prec, uint8_t scale)
{
	char d[25];
	ftoaPseudo_(num, d, prec);
	LCD_String(x, y, d, scale);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void LCD_FloatPseudoWString(uint8_t x, uint8_t y, int32_t num, char *text, uint8_t prec, uint8_t scale)
{
	char d[25];
	ftoaPseudo_(num, d, prec);
	strcat_(d, text);
	LCD_String(x, y, d, scale);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void LCD_FloatPseudoW2String(uint8_t x, uint8_t y, char *text, int32_t num, char *text2, uint8_t prec, uint8_t scale)
{
	char d[25], o[25];
	strcpy_(d, text);
	ftoaPseudo_(num, o, prec);
	strcat_(d, o);
	strcat_(d, text2);
	LCD_String(x, y, d, scale);
}

/*******************************************************************************
 * Function Name  : DisplayNumWDesc
 * Description    : Display String + Number
 *******************************************************************************/
void LCD_NumWDesc(uint8_t x, uint8_t y, char *s, int32_t num)
{
	char o[30], d[25];
	strcpy_(o, s);
	itoa_(num, d);
	strcat_(o, d);
	LCD_String(x, y, o, 1);
}

void LCD_textNumText(uint8_t x, uint8_t y, char *s, int32_t num, char *s2, uint8_t scale)
{
	char o[30], d[25];
	strcpy_(o, s);
	itoa_(num, d);
	strcat_(o, d);
	strcat_(o, s2);
	LCD_String(x, y, o, scale);
}


void LCD_Celsium(uint8_t x, uint8_t y, char *text, uint16_t num, uint8_t scale)
{
	char d[25], xx[20];
	strcpy_(d, text);
	itoa_(num, xx);
	strcat_(d, xx);
	strcat_(d, "\x7f");
	LCD_String(x, y, d, scale);
}


void LCD_InverseModeOn()
{
	inverseMode = 1;
}

void LCD_InverseModeOff()
{
	inverseMode = 0;
}
