/* Includes ------------------------------------------------------------------*/
#include "math_.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const unsigned int sinTable[91] =
{ 0, 18, 36, 54, 71, 89, 107, 125, 143, 160, 178, 195, 213, 230, 248, 265, 282, 299, 316, 333, 350, 367, 384, 400, 416,
	433, 449, 465, 481, 496, 512, 527, 543, 558, 573, 587, 602, 616, 630, 644, 658, 672, 685, 698, 711, 724, 737, 749,
	761, 773, 784, 796, 807, 818, 828, 839, 849, 859, 868, 878, 887, 896, 904, 912, 920, 928, 935, 943, 949, 956, 962,
	968, 974, 979, 984, 989, 994, 998, 1002, 1005, 1008, 1011, 1014, 1016, 1018, 1020, 1022, 1023, 1023, 1024, 1024 };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : pow_
 * Description    : Power x in y
 *******************************************************************************/
/*float pow_(float x, float y)
 {
 double result = 1;

 for (int i=0; i<y; i++)
 result *= x;

 return result;
 }*/

void MultStrings(uint8_t* s1, uint8_t* s2)
{
	char j, ch;
	char s3[22] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	ch = 0;

	for(j = 0; j <= 10; j++)
	{
		for(uint8_t I = 0; I < 12; I++)
		{
			s3[21 - I - j] += (s1[11 - I] * s2[11 - j]) + ch;
			ch = s3[21 - I - j] / 10;
			s3[21 - I - j] %= 10;
		}
	}

	j = 0;

	for(uint8_t i = 0; i < 8; i++)
		j += s3[i];

	//@if(j != 0)
	//@	Err_KL = 1;

	for(uint8_t i = 0; i < 12; i++)
	{
		s1[11 - i] = s3[18 - i];
	}
}

/*******************************************************************************
 * Function Name  : log10_
 * Description    : log10 of x
 *******************************************************************************/
/*float log10_(int x)
 {
 return (x >= 1000000000u) ? 9 : (x >= 100000000u) ? 8 :
 (x >= 10000000u) ? 7 : (x >= 1000000u) ? 6 :
 (x >= 100000u) ? 5 : (x >= 10000u) ? 4 :
 (x >= 1000u) ? 3 : (x >= 100u) ? 2 : (x >= 10u) ? 1u : 0u;
 }*/

#if 0
/*******************************************************************************
 * Function Name  : sinReal
 * Description    : Count sin of x with Taylor expansion
 *******************************************************************************/
float sinReal_(float x)
{
	signed char sign = 1;
	if(x<-90)
	{
		while(x<-90)
		{	x += 180; sign=-sign;}
	}
	else if(x>90)
	{
		while(x>90)
		{	x -= 180; sign=-sign;}
	}

	x = x*0.0174532925;
	float sine = x;

#define LIMIT 7
	for(double i=2; i<=LIMIT; i++)
	sine+= pow_(-1,i-1)*pow_(x,2*i-1)/factorial(2*i-1);

	return ((float)sign)*sine;
}
#endif

/*******************************************************************************
 * Function Name  : sin_
 *******************************************************************************/
float sin_(float x)
{
	float sign = 1, sine;

	if(x < 0)
	{
		while(x < 0)
		{
			x += 180;
			sign = -sign;
		}
	}
	else
		if(x > 180)
		{
			while(x > 180)
			{
				x -= 180;
				sign = -sign;
			}
		}

	if(x < 90)
		sine = sinTable[(unsigned int) x];
	else
		sine = sinTable[(unsigned int) (180 - x)];

	return sign * sine / 1024;
}

/*******************************************************************************
 * Function Name  : cos_
 *******************************************************************************/
float cos_(float x)
{
	x -= 90;
	float sign = 1, sine;

	if(x < 0)
	{
		while(x < 0)
		{
			x += 180;
			sign = -sign;
		}
	}
	else
		if(x > 180)
		{
			while(x > 180)
			{
				x -= 180;
				sign = -sign;
			}
		}

	if(x < 90)
		sine = sinTable[(unsigned int) x];
	else
		sine = sinTable[(unsigned int) (180 - x)];

	return -sign * sine / 1024;
}

/*******************************************************************************
 * Function Name  : tan_
 *******************************************************************************/
float tan_(float x)
{
	return sin_(x) / cos_(x);
}

/*******************************************************************************
 * Function Name  : factorial
 *******************************************************************************/
float factorial_(double x)
{
	double result = 1;

	for(int i = 1; i <= x; i++)
		result *= i;

	return result;
}

/*******************************************************************************
 * Function Name  : sqrt_
 * Description    : Square root of x
 *******************************************************************************/
float sqrt_(float x)
{
	union
	{
		int i;
		float x;
	} u;

	u.x = x;
	u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
	return u.x;
}