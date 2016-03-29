
#include "akdefine.h"
#include "utils.h"


int  strlen(const char * s)
{
	int ret;
	for(ret = 0 ; s[ret] != '\0'; ret++)
		;
	return ret;
}

void memset(T_U8 *s, T_U8 c, int n)
{
	int i;
	for( i = 0 ; i < n ; i ++)
	{
		s[i] = c;
	}
}


int strcmp(const char *s1, const char *s2)
{
	int i;
	int len = strlen (s1);
	if ( len != strlen(s2) )
	{
		return -1;
	}
	for(i =0 ; i<len; i ++)
	{
		if (s1[i] != s2[i])
		{
			return -1;
		}
	}
	
	return 0;
}

unsigned long strtoul(char *s, char **endptr, int radix)
{
	unsigned long ret;
	int i;

	ret = 0;

	while (*s != '\0') {
		if (*s >= '0' && *s <= '9')
			i = *s - '0';
		else if (*s >= 'a' && *s <= 'f')
			i = *s - 'a' + 0xa;
		else if (*s >= 'A' && *s <= 'F')
			i = *s - 'A' + 0xa;
		else
			break;
		ret = (ret << 4) + i;
		s++;
	}

	return ret;
}

void memcpy(T_U8 *src, T_U8 *dst, int n)
{
	int i;
	for( i = 0 ; i < n ; i ++)
	{
		src[i] = dst[i];
	}
}

/**
 * @brief Compare characters of two strings without regard to case by length
 * 
 * @author MiaoBaoLi 
 * @date  2001-08-1 
 * @param T_pDATA s one string pointer
 * @param T_pDATA d another string pointer
 * @param T_U16 length
 * @return T_S16
 * @retval 0:	s substring identical to d substring
 *		   1:	s substring greater than d substring
 *		   -1: s substring less than d substring
 */
T_S8 Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length)
{
	T_U8	c1,c2;
	T_U16  i;
	T_pDATA	pStr1 = ( T_pDATA )str1;
	T_pDATA	pStr2 = ( T_pDATA )str2;

    if (0 == str1)
    {
        return -1;
    }
    if (0 == str2)
    {
        return 1;
    }

	//AK_ASSERT_PTR(str1, "Utl_StrCmpNC()", 0);
	//AK_ASSERT_PTR(str2, "Utl_StrCmpNC()", 0);

	for( i=0; i<length; i++, pStr1++, pStr2++ )
	{
		c1 = ( T_U8 )( *pStr1 );
		if( c1 >= 'A' && c1 <= 'Z' )
			c1 += 0x20;

		c2 = ( T_U8 )( *pStr2 );
		if( c2 >= 'A' && c2 <= 'Z' )
			c2 += 0x20;

		if(c1 > c2) 
        {
            return 1;
        }
		if(c1 < c2)
        {
            return -1;
        }
	}
	return 0;
}

// the picture must be 24-bits pixel.
void displayPowerOnPic( int x, int y, unsigned char *buffer, int picWidth, int picHeight )
{
	//int bytesPerWindowLine = 240*3, bytesPerPicLine = picWidth*3, i;
	unsigned char *p, *q;
	int i, j, x1, y1;
	unsigned long color;

    printf("display bios picture\n");
	
	p = buffer;
	for (y1=y; y1<y+picHeight; y1++)
	{
    	for (x1=x; x1<x+picWidth; x1++)
	    {
	        color = ((*p)<<16) | ((*(p+1))<<8) | (*(p+2));
	        //lcd_set_pixel(0, x1, y1, color);
	        p += 3;
	    }
	}
	
	/*
	p = gb_DisplayBuffer + y*bytesPerWindowLine + x*3;
	q = buffer;
	for( i=0; i<picHeight; i++ )
	{
		memcpy( p, q, bytesPerPicLine );
		p += bytesPerWindowLine;
		q += bytesPerPicLine;
	}*/
}

