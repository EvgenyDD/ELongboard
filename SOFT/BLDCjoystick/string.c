/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stm32f10x.h"
//#include "math_.h"
#include <stdbool.h>
#include "macros.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  : strlen
 * Description    : calculating length of the string
 * Input          : pointer to text string
 * Return         : string length
 *******************************************************************************/
int strlen(char *pText)
{
	int len = 0;
	for(; *pText != '\0'; pText++, len++);
	return len;
}

/*******************************************************************************
 * Function Name  : strlenNum
 * Description    : calculating length of the string
 * Input          : pointer to text string
 * Return         : string length
 *******************************************************************************/
int strlenNum(char *pText, int begin)
{
	int len = 0;
	pText += begin;
	for(; *pText != '\0'; pText++, len++);
	return len;
}

/*******************************************************************************
 * Function Name  : itoa
 * Description    : Convert int to char
 * Input          : int number (signed/unsigned)
 * Return         : pointer to text string
 *******************************************************************************/
void itoa_(int32_t n, char s[])
{
	int64_t sign;

	if((sign = n) < 0)
		n = -n;

	int i = 0;

	do
	{
		s[i++] = n % 10 + '0';
	} while((n /= 10) > 0);

	if(sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}

/*******************************************************************************
 * Function Name  : ftoaPseudo_
 * Description    : Convert int to pseudo float by inserting point in
 * 					the middle of the number
 * Input          : point rank
 *******************************************************************************/
void ftoaPseudo_(int32_t n, char s[], uint8_t pointNumbers)
{
	int64_t sign;

	if((sign = n) < 0)
		n = -n;

	int i = 0;

	do
	{
		s[i++] = n % 10 + '0';
		if(i == pointNumbers)
			s[i++] = '.';
	}while((n /= 10) > 0);

	if((i <= (pointNumbers + 1)) && (pointNumbers != 0))
		s[i++] = '0';

	if(sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}
/*******************************************************************************
 * Function Name  : itoa
 * Description    : Convert int to char
 * Input          : int number (signed/unsigned)
 * Return         : pointer to text string
 *******************************************************************************/
void itoa_hex(int64_t n, char s[])
{
	int64_t sign;

	if((sign = n) < 0)
		n = -n;

	int i = 0;

	do
	{
		uint8_t dd = n % 16;
		if(dd < 10)
			dd = dd + '0';
		else
			dd = dd - 10 + 'A';
		s[i++] = dd;
	} while((n /= 16) > 0);

	if(sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}


/*******************************************************************************
 * Function Name  : itoa
 * Description    : Convert int to char
 * Input          : int number (signed/unsigned)
 * Return         : pointer to text string
 *******************************************************************************/
void itoa_zeros(uint32_t n, char s[], uint8_t end)
{
	int i = end-1;

	ClearBuffer((uint8_t*)s, end, /*0*/'0');

	do
	{
		s[i--] = n % 10 + '0';
	} while((n /= 10) > 0);

}


/*******************************************************************************
 * Function Name  : itoa
 * Description    : Convert int to char
 * Input          : int number (signed/unsigned)
 * Return         : pointer to text string
 *******************************************************************************/
void dtoa_(int64_t n, char s[])
{
	int64_t sign;

	if((sign = n) < 0)
		n = -n;

	int i = 0;

	do
	{
		s[i++] = n % 10 + '0';
	} while((n /= 10) > 0);

	if(sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}

/*******************************************************************************
 * Function Name  : itoa
 * Description    : Convert int to char
 * Input          : int number (signed/unsigned)
 * Return         : pointer to text string
 *******************************************************************************/
void dtoaPositive_(int64_t n, char s[])
{
	int64_t sign;

	if((sign = n) < 0)
		n = -n;

	int i = 0;

	do
	{
		s[i++] = n % 10 + '0';
	} while((n /= 10) > 0);

	s[i] = '\0';
	reverse(s);
}

/*******************************************************************************
 * Function Name  : ftoa_
 * Description    : Convert float to char
 * Input          : float number, char, output precision
 * Return         : pointer to text string
 *******************************************************************************/
void ftoa_(float num, char str[], uint8_t precision)
{
	uint8_t zeroFlag = 0;
	int digit = 0, reminder = 0;
	long wt = 0;

	if(num < 0)
	{
		num = -num;
		zeroFlag = 1;
	}

	int whole_part = num;
	int log_value = log10_(num), index = log_value;

	if(zeroFlag)
		str[0] = '-';

	//Extract the whole part from float num
	for(int i = 1; i < log_value + 2; i++)
	{
		wt = pow_(10.0, i);
		reminder = whole_part % wt;
		digit = (reminder - digit) / (wt / 10);

		//Store digit in string
		str[index-- + zeroFlag] = digit + 48;              // ASCII value of digit  = digit + 48
		if(index == -1)
			break;
	}

	index = log_value + 1;
	str[index + zeroFlag] = '.';

	float fraction_part = num - whole_part;
	float tmp1 = fraction_part, tmp = 0;

	//Extract the fraction part from  number
	for(int i = 1; i <= precision; i++)
	{
		wt = 10;
		tmp = tmp1 * wt;
		digit = tmp;

		//Store digit in string
		str[++index + zeroFlag] = digit + 48;           // ASCII value of digit  = digit + 48
		tmp1 = tmp - digit;
	}
	str[++index + zeroFlag] = '\0';
}

/*******************************************************************************
 * Function Name  : reverse
 * Description    : Reverses string
 * Input          : pointer to string
 *******************************************************************************/
void reverse(char s[])
{
	int c, i, j;
	for(i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/*******************************************************************************
 * Function Name  : strcat
 * Description    : add
 * Input          : pointer to string
 *******************************************************************************/
void strcat_(char first[], char second[])
{
	int i = 0, j = 0;

	while(first[i] != '\0')
		i++;
	while((first[i++] = second[j++]) != '\0');
}


/*******************************************************************************
 * Function Name  : strcat
 * Description    : add
 * Input          : pointer to string
 *******************************************************************************/
void strcat_NO_EOL(char first[], char second[])
{
	int i = 0, j = 0;

//	while(first[i] != '\0')
//		i++;
	for(uint8_t k=0; k<strlen(second); k++)
		first[i++] = second[j++];
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 * Input          :
 *******************************************************************************/
void strcat_white(char first[], uint8_t first_limit, char second[], uint8_t second_limit)
{
	int i = first_limit-1, j = 0, sec_end = second_limit;

	while(first[i-1] == ' ' || first[i-1] == '\0')
		i--;

	while(second[sec_end-1] == ' ' || second[sec_end-1] == '\0')
		sec_end--;

	first[i++] = '-';

	for(uint8_t g = 0; g < sec_end; g++)
		first[i++] = second[j++];

	first[i] = '\0';
	//while((first[i++] = second[j++]) != '\0');
}

void strcpy_(char first[], char second[])
{
	int i = 0, j = 0;

	while((first[i++] = second[j++]) != '\0');
}

/*******************************************************************************
 * Function Name  : strcat
 * Description    : add
 * Input          : pointer to string
 *******************************************************************************/
void strcatnum(char first[], char second[], uint8_t num)
{
	for(uint8_t i = 0; i < num; i++)
		first[i] = second[i];
}

/*******************************************************************************
 * Function Name  : strcat
 * Description    : add
 * Input          : pointer to string
 *******************************************************************************/
void strcatNum(char first[], char second[], int begin, int end)
{
	if(begin >= end)
		return;

	int i = 0, j = begin;

	while(j != end)
		first[i++] = second[j++];

	first[i] = '\0';
}

void strcatNum2(char first[], char second[], int begin)
{
	int i = begin, j = 0;

	while((first[i++] = second[j++]) != '\0');
}

/*******************************************************************************
 * Function Name  : pow_
 * Description    : Power x in y
 *******************************************************************************/
float pow_(float x, float y)
{
	double result = 1;

	for(int i = 0; i < y; i++)
		result *= x;

	return result;
}

/*******************************************************************************
 * Function Name  : pow_
 * Description    : Power x in y
 *******************************************************************************/
uint64_t pow10_(uint8_t x)
{
	uint64_t result = 1;

	for(int i = 0; i < x; i++)
		result *= 10;

	return result;
}

/*******************************************************************************
 * Function Name  : log10_
 *******************************************************************************/
float log10_(int v)
{
	return (v >= 1000000000u) ? 9 : (v >= 100000000u) ? 8 : (v >= 10000000u) ? 7 : (v >= 1000000u) ? 6 :
			(v >= 100000u) ? 5 : (v >= 10000u) ? 4 : (v >= 1000u) ? 3 : (v >= 100u) ? 2 : (v >= 10u) ? 1u : 0u;
}

/*******************************************************************************
 * Function Name  : stof
 * Description    : String to float
 *******************************************************************************/
float atof(uint8_t* num)
{
	if(!num || !*num)
		return 0;

	double integerPart = 0;
	double fractionPart = 0;
	int divisorForFraction = 1;
	int sign = 1;
	bool inFraction = false;
	/*Take care of +/- sign*/
	if(*num == '-')
	{
		++num;
		sign = -1;
	}
	else if(*num == '+')
	{
		++num;
	}
	while(*num != '\0')
	{
		if(*num >= '0' && *num <= '9')
		{
			if(inFraction)
			{
				/*See how are we converting a character to integer*/
				fractionPart = fractionPart * 10 + (*num - '0');
				divisorForFraction *= 10;
			}
			else
			{
				integerPart = integerPart * 10 + (*num - '0');
			}
		}
		else if(*num == '.')
		{
			if(inFraction)
				return sign * (integerPart + fractionPart / divisorForFraction);
			else
				inFraction = true;
		}
		else
		{
			return sign * (integerPart + fractionPart / divisorForFraction);
		}
		++num;
	}
	return sign * (integerPart + fractionPart / divisorForFraction);
}


/*******************************************************************************
 * Function Name  : atoi_special
 * Description    : Shifted String to uint64_t
 *******************************************************************************/
uint64_t atoi_special(char *s, uint8_t start, uint8_t end)
{
	uint64_t result = 0;
	if(start >= end)
		return 0;

	for(uint8_t i = start; i <= end; i++)
		result += pow10_(end - i) * s[i];

	return result;
}

/*******************************************************************************
 * Function Name  : atoi_special
 * Description    : Shifted String to uint64_t
 *******************************************************************************/
uint64_t atoi_special2(char *s, uint8_t start, uint8_t end)
{
	uint64_t result = 0;
	if(start >= end)
		return 0;
	for(;s[end] > 9 && end != 0; end--);

	for(uint8_t i = start; i <= end; i++)
	{
		if(s[i] <= 9)
			result += pow10_(end - i) * s[i];
	}

	return result;
}

/*******************************************************************************
 * Function Name  : atoi_BCD
 * Description    : Shifted String (ASCII) to uint64_t
 *******************************************************************************/
uint64_t atoi_BCD(char *s, uint8_t start, uint8_t end)
{
	uint64_t result = 0;
	uint8_t ss;

	if(start >= end)
		return 0;

	for(uint8_t i = start; i < end; i++)
	{
		if(s[i] == 0)
			continue;

		ss = s[i] - '0';
		result += pow10_(end-1 - i) * ss;
	}

	return result;
}

/*******************************************************************************
 * Function Name  : stoi_
 * Description    : String (ASCII) to int
 *******************************************************************************/
uint64_t stoi_(char *s)
{
	int len = 0;
	char *s2 = s;

	for(; *s2 != ' ' /*&& *s2 != '\0'*/ && *s != '\n'; s2++, len++)
		;

	uint64_t result = 0;

	for(uint8_t i = 0; i < len; i++)
		result += pow10_(len - i - 1) * s[i];

	return result;
}

/*******************************************************************************
 * Function Name  : stoi_
 * Description    : String (ASCII) to int
 *******************************************************************************/
uint64_t stoi_num(char *s)
{
	int len = 0;
	char *s2 = s;

	for(; *s2 != ' ' && *s2 != '\0' && *s != '\n'; s2++, len++)
		;

	uint64_t result = 0;

	for(uint8_t i = 0; i < len; i++)
		result += pow10_(len - i - 1) * s[i];

	return result;
}
