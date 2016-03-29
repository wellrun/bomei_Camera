/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：SMSCodeDe.c
 * Function：Encapsulate mathematic library. mainly including double operator.
 
 *
 * Author：Zou Mai
 * Date：2002-11-14
 * Version：1.0          
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Eng_Math.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Eng_String_UC.h"
#include "fwl_oscom.h"
#include "Eng_DataConvert.h"
#include "stdlib.h"
#include "Eng_debug.h"

/**
 * @brief Initial a two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_S32 number:
 * @param T_S16 power:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblInit(T_S32 number, T_S16 power)
{
    T_DBL result;

    result.Number = number;
    result.Power = power;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Copy a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblCopy(T_DBL num)
{
    T_DBL result;

    result.Number = num.Number;
    result.Power = num.Power;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Add two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num1:
 * @param T_DBL num2:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblAdd(T_DBL num1, T_DBL num2)
{
    T_DBL result;

    Fwl_DblTidyUp(&num1);
    Fwl_DblTidyUp(&num2);
    if (Fwl_DblOverflow(&num1) || Fwl_DblOverflow(&num2))
        return Fwl_DblSetOverflow(&result);

    Fwl_DblPowerMatch(&num1, &num2);
    result.Number = num1.Number + num2.Number;
    result.Power = num1.Power;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Minus two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num1:
 * @param T_DBL num2:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblMinus(T_DBL num1, T_DBL num2)
{
    T_DBL    result;

    Fwl_DblTidyUp(&num1);
    Fwl_DblTidyUp(&num2);
    if (Fwl_DblOverflow(&num1) || Fwl_DblOverflow(&num2))
        return Fwl_DblSetOverflow(&result);

    Fwl_DblPowerMatch(&num1, &num2);
    result.Number = num1.Number - num2.Number;
    result.Power = num1.Power;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Multiply two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num1:
 * @param T_DBL num2:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblMulti(T_DBL num1, T_DBL num2)
{
    T_DBL    result;
    T_DBL    dblTemp;
    T_U8    intLen1, intLen2;
    T_S8    sign = 1;

    Fwl_DblTidyUp(&num1);
    Fwl_DblTidyUp(&num2);
    if (Fwl_DblOverflow(&num1) || Fwl_DblOverflow(&num2))
        return Fwl_DblSetOverflow(&result);
    
    /* high precision porcess */
    if (num1.Number < 0 && num2.Number > 0 || num1.Number > 0 && num2.Number < 0)
        sign = -1;
    num1.Number = Fwl_Abs(num1.Number);
    num2.Number = Fwl_Abs(num2.Number);
    intLen1 = Fwl_DblGetIntLen(num1.Number);
    intLen2 = Fwl_DblGetIntLen(num2.Number);
    if (intLen1 == MAX_DBL_LEN && intLen2 == MAX_DBL_LEN)
    {
        if ((num1.Number % 10) > (num2.Number % 10))
        {
            dblTemp = Fwl_DblCopy(num1);
            num1 = Fwl_DblIntTidyUp(&num2, -1);
            num2 = Fwl_DblCopy(dblTemp);
        }
        else
        {
            num1 = Fwl_DblIntTidyUp(&num1, -1);
        }
    }
    else if (intLen1 > intLen2)
    {
        dblTemp = Fwl_DblCopy(num1);
        num1 = Fwl_DblCopy(num2);
        num2 = Fwl_DblCopy(dblTemp);
    }
    /* now, num1 and num2 have the following relation:
        num1 > 0, num2 > 0,
        num1's Length < MAX_DBL_LEN, num2's Length <= MAX_DBL_LEN
        num1's Length <= num2's Length */

    result = Fwl_DblInit(0, 0);
    while (num2.Number != 0)
    {
        dblTemp = Fwl_DblInit((num2.Number % 10) * num1.Number, num2.Power);
        if (Fwl_DblOverflow(&dblTemp))
            return Fwl_DblSetOverflow(&dblTemp);
        result = Fwl_DblAdd(result, dblTemp);
        num2.Number /= 10;
        num2.Power++;
    }
    result.Power = (T_S16)(result.Power + num1.Power);
    result.Number *= sign;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Double number divide.
 * 
 * @author ZouMai ZhuSiZhe
 * @date 2002-11-02 2005-12-04
 * @param T_DBL num1:
 * @param T_DBL num2:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblDivide(T_DBL num1, T_DBL num2)
{
    T_DBL    result;
    T_DBL    dblTemp;
    T_U8    intLen1, intLen2;
    T_S32    int1, int2;
    T_S32    dValue;
    T_U8    count = 0;
    T_S16    power = 0;
    T_S8    sign = 1;

    Fwl_DblTidyUp(&num1);
    Fwl_DblTidyUp(&num2);
    if (Fwl_DblOverflow(&num1) || Fwl_DblOverflow(&num2))
        return Fwl_DblSetOverflow(&result);

    if (num1.Number == 0 && num2.Number != 0)
        return Fwl_DblInit(0, 0);
    if (num2.Number == 0)
        return Fwl_DblSetOverflow(&result);
    if (num1.Number == num2.Number)
    {
        result = Fwl_DblInit(1, (T_S16)(num1.Power - num2.Power));
        return Fwl_DblTidyUp(&result);
    }

#if 0    
    /* here is the simple process: low precision */
    intLen1 = Fwl_DblGetIntLen(num1.Number);
    intLen2 = Fwl_DblGetIntLen(num2.Number);

    Fwl_DblIntTidyUp(&num1, (T_S16)(MAX_DBL_LEN-(intLen1)));
    if (intLen2 > MAX_DBL_LEN/2+1)
        Fwl_DblIntTidyUp(&num2, (T_S16)(MAX_DBL_LEN/2+1-intLen2));

    result.Number = num1.Number / num2.Number;
    result.Power = num1.Power - num2.Power;
#endif
    /* high precision porcess */
    if (num1.Number < 0 && num2.Number > 0 || num1.Number > 0 && num2.Number < 0)
        sign = -1;
    num1.Number = Fwl_Abs(num1.Number);
    num2.Number = Fwl_Abs(num2.Number);
    intLen1 = Fwl_DblGetIntLen(num1.Number);
    intLen2 = Fwl_DblGetIntLen(num2.Number);
    if (intLen2 == MAX_DBL_LEN)
    {
        num1 = Fwl_DblIntTidyUp(&num1, (T_S16)(intLen2 - intLen1));
        if (num2.Number > num1.Number)
        {
            num2 = Fwl_DblIntTidyUp(&num2, -1);        /* cut the last byte if num2 is too long and is too large */
            intLen2 = Fwl_DblGetIntLen(num2.Number);
        }
        num1 = Fwl_DblTidyUp(&num1);
    }
    int1 = num1.Number;
    int2 = num2.Number;
    if (int1 > int2)        /* get power gap */
    {
        int1 /= 10;
        while (int1 >= int2)
        {
            power++;
            int1 /= 10;
        }
    }
    else
    {
        while (int1 < int2)
        {
            power--;
            if (Fwl_DblGetIntLen(int1) < MAX_DBL_LEN)
                int1 *= 10;
            else
                break;
        }
    }
    result = Fwl_DblInit(0, 0);
    int1 = num1.Number;
    dblTemp = Fwl_DblInit(int1, (T_S16)(power * (-1)));
    while (count++ <= MAX_DBL_LEN)    /* loop number: MAX_DBL_LEN + 1 */
    {
        if (power > 0)
        {
            dValue = dblTemp.Number / (num2.Number * Fwl_Power(10, (T_U8)power));
            int1 = dblTemp.Number - num2.Number * dValue * Fwl_Power(10, (T_U8)power);
            dblTemp = Fwl_DblInit(int1, 1);
            Fwl_DblIntTidyUp(&dblTemp, dblTemp.Power);
        }
        else
        {
            Fwl_DblIntTidyUp(&dblTemp, dblTemp.Power);
            dValue = dblTemp.Number / num2.Number;
            int1 = dblTemp.Number - num2.Number * dValue;
            dblTemp = Fwl_DblInit(int1, 1);
        }

        /* assign result */
        if (count <= MAX_DBL_LEN)
        {
            result = Fwl_DblMulti(result, Fwl_DblInit(1, 1));
            result = Fwl_DblAdd(result, Fwl_DblInit(dValue, 0));
        }
        else
        {
            result = Fwl_DblAdd(result, Fwl_DblInit(dValue >= 5 ? 1: 0, 0));
        }
    }

    result.Power = (T_S16)(power + num1.Power - num2.Power - Fwl_DblGetIntLen(result.Number) + 1);
    result.Number *= sign;

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Compare two double numbers.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num1:
 * @param T_DBL num2:
 * @return T_S8
 * @retval 1: num1 > num2
 * @retval 0: num1 = num2
 * @retval -1: num1 < num2
 */
T_S8 Fwl_DblCompare(T_DBL num1, T_DBL num2)
{
    Fwl_DblPowerMatch(&num1, &num2);
    if (num1.Number > num2.Number)
        return 1;
    else if (num1.Number < num2.Number)
        return -1;
    return 0;
}

/**
 * @brief Compare two double numbers.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num1:
 * @param T_eCOMP_OPER oper:
 * @param T_DBL num2:
 * @return T_BOOL
 * @retval
 */
T_BOOL    Fwl_DblComOper(T_DBL num1, T_eCOMP_OPER oper, T_DBL num2)
{
    T_BOOL ret = AK_FALSE;

    switch (oper) {
    case coLESSTHAN:
        if (Fwl_DblCompare(num1, num2) == -1)
            ret = AK_TRUE;
        break;
    case coLESSEQUAL:
        if (Fwl_DblCompare(num1, num2) != 1)
            ret = AK_TRUE;
        break;
    case coGREATTHAN:
        if (Fwl_DblCompare(num1, num2) == 1)
            ret = AK_TRUE;
        break;
    case coGREATEQUAL:
        if (Fwl_DblCompare(num1, num2) != -1)
            ret = AK_TRUE;
        break;
    case coEQUAL:
        if (Fwl_DblCompare(num1, num2) == 0)
            ret = AK_TRUE;
        break;
    case coNOTEQUAL:
        if (Fwl_DblCompare(num1, num2) != 0)
            ret = AK_TRUE;
        break;
    default:
        break;
    }
    return ret;
}

/**
 * @brief Get power value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num:
 * @param T_S16 power:
 * @return T_VOID
 * @retval
 */
T_DBL Fwl_DblPower(T_DBL num, T_S16 power)
{
    T_DBL result;

	T_TSTR_INFO     ansiStrResult;

	if(0 == num.Number)
	{
		result.Number = 0;
		result.Power = 0;
		return result;
	}

	if(0 == power)
	{
		result.Number = 1;
		result.Power = 0;
		return result;
	}

	result = num;

	//Utl_Itoa(result.Number, ansiStrResult, 10);
	//Fwl_Print(C3, M_STANDBY, "num is  mantissa : %s, Power %d", ansiStrResult, result.Power);

	while(power > 1)
	{
		result.Number *= num.Number;    
		result.Power += num.Power;   
		power--;

	//    Utl_Itoa(result.Number, ansiStrResult, 10);
	//    Fwl_Print(C3, M_STANDBY, "___ result is  mantissa : %s, Power %d", ansiStrResult, result.Power);
	}

	Utl_UItoa(result.Number, ansiStrResult, 10);
	Fwl_Print(C3, M_STANDBY, "result is  mantissa : %s, Power %d", ansiStrResult, result.Power);

	return Fwl_DblTidyUp(&result);;    
}

/**
 * @brief Get absolute value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param const T_DBL *num:
 * @return T_VOID
 * @retval
 */
T_DBL Fwl_DblAbs(const T_DBL *num)
{
    T_DBL result;

    AK_ASSERT_PTR(num, "Fwl_DblAbs()", Fwl_DblInit(1, 0));

    result.Number = Fwl_Abs(num->Number);
    result.Power = num->Power;

    return result;
}

/**
 * @brief Get square root of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-03
 * @param T_DBL num:
 * @return T_VOID
 * @retval
 */
T_DBL Fwl_DblSqrt(T_DBL num)
{
    T_DBL    result;
    T_S16    count = 100;        /* loop count */
    T_DBL    x0;                    /* initial value to calculate */
    T_DBL    eps = Fwl_DblInit(1, -6);

    T_DBL    y[2];
    T_DBL    d, p, x1;

    Fwl_DblTidyUp(&num);
    if (Fwl_DblOverflow(&num) || num.Number < 0)
        return Fwl_DblSetOverflow(&result);
    if (num.Number == 0)
        return Fwl_DblInit(0, 0);

    if (num.Number == 1 && num.Power % 2 == 0)    /* 1 or 100 or 10000 or 1000000 ... */
    {
        return Fwl_DblInit(1, (T_S16)(num.Power / 2));
    }

    /* Get a proper initial seed to x0 */
    x1 = Fwl_DblCopy(num);    /* x1 is a temp variable here */
    Fwl_DblIntTidyUp(&x1, (T_S16)(2 - Fwl_DblGetIntLen(x1.Number)));
    if (x1.Power % 2 != 0)
    {
        Fwl_DblIntTidyUp(&x1, -1);
    }
    x0.Power = (T_S16)(x1.Power / 2);    /* proper seed can improve speed */
    if (x1.Number > 72)
        x0.Number = 9;
    else if (x1.Number > 56)
        x0.Number = 8;
    else if (x1.Number > 42)
        x0.Number = 7;
    else if (x1.Number > 30)
        x0.Number = 6;
    else if (x1.Number > 20)
        x0.Number = 5;
    else if (x1.Number > 12)
        x0.Number = 4;
    else if (x1.Number > 6)
        x0.Number = 3;
    else if (x1.Number > 2)
        x0.Number = 2;
    else
        x0.Number = 1;

    y[0] = Fwl_DblMinus(Fwl_DblMulti(x0, x0), num);        /* x*x - num */

    if(0==(y[0].Number))
        return Fwl_DblTidyUp(&x0);
    y[1] = Fwl_DblMulti(Fwl_DblInit(2, 0), x0);            /* 2*x */
    d=Fwl_DblAdd(eps, Fwl_DblInit(1, 0));
    while ((Fwl_DblComOper(d, coGREATEQUAL, eps)) && (count-- != 0))
    {
        if (Fwl_DblComOper(Fwl_DblAdd(Fwl_DblAbs(&y[1]), Fwl_DblInit(1, 0)), coEQUAL, Fwl_DblInit(1, 0)))
        {
            return Fwl_DblSetOverflow(&result);
        }
        x1= Fwl_DblMinus(x0, Fwl_DblDivide(y[0], y[1]));
        y[0] = Fwl_DblMinus(Fwl_DblMulti(x1, x1), num);        /* x*x - num */
        y[1] = Fwl_DblMulti(Fwl_DblInit(2, 0), x1);            /* 2*x */

        d = Fwl_DblMinus(x1, x0);
        d = Fwl_DblAbs(&d);
        p = Fwl_DblAbs(&y[0]);
        if (Fwl_DblComOper(p, coGREATTHAN, d))
            d = Fwl_DblCopy(p);
        x0 = Fwl_DblCopy(x1);
    }
    result = Fwl_DblCopy(x1);
    result = Fwl_DblAbs(&result);
    if (result.Number % 10 == 1 && (result.Number - 1) % 1000000 == 0)    /* x.000001 */
    {
        result.Number -= 1;
    }
    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Get factorial value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-05
 * @param T_DBL num:
 * @return T_DBL
 * @retval
 */
T_DBL    Fwl_DblFact(T_DBL num)
{
    T_DBL    result;

    Fwl_DblTidyUp(&num);
    if (Fwl_DblOverflow(&num) || num.Number < 0)
        return Fwl_DblSetOverflow(&result);
    if (num.Number == 0)
        return Fwl_DblInit(1, 0);

    if (num.Power < 0)
        Fwl_DblIntTidyUp(&num, num.Power);

    if (num.Number >= 450)            /* for platform quick return */
        return Fwl_DblSetOverflow(&result);

    result = Fwl_DblInit(1, 0);
    while (num.Number > 0)
    {
        result = Fwl_DblMulti(result, num);
        if (Fwl_DblOverflow(&result))
            break;
        num = Fwl_DblMinus(num, Fwl_DblInit(1, 0));
    }

    return Fwl_DblTidyUp(&result);
}

/**
 * @brief Delete the last digital of the double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL *num:
 * @return T_DBL
 * @retval
 */
T_DBL    Fwl_DblBackspace(T_DBL *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblBackspace()", Fwl_DblInit(1, 0));

    if (num->Power > 0)
    {
        num->Power--;
    }
    else if (num->Power < 0)
    {
        num->Number /= 10;
        num->Power++;
    }
    else
    {
        num->Number /= 10;
    }

    return Fwl_DblTidyUp(num);
}

/**
 * @brief Check the double number is overflow or not.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param const T_DBL num:
 * @return T_BOOL
 * @retval
 */
T_BOOL Fwl_DblOverflow(const T_DBL *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblSetOverflow()", AK_TRUE);

    if (Fwl_Abs(num->Power) + Fwl_DblGetIntLen(num->Number) > MAX_DBL_POWER)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

/**
 * @brief Check the double number is overflow or not.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL *num:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblSetOverflow(T_DBL *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblSetOverflow()", *num);

    num->Number = 1;
    num->Power = MAX_DBL_POWER;
    return *num;
}

/**
 * @brief Conver a double number to a string.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL num: double number
 * @param T_pSTR string: string for return
 * @param T_U8 maxlen: maximum length of the string
 * @return T_pDATA 
 * @retval
 */
T_pWSTR Fwl_Dbl2String(T_DBL num, T_pWSTR string, T_U8 maxlen)
{
    T_U8        len = Fwl_DblGetIntLen(num.Number);
    T_U8        numlen = maxlen;
    T_U8        start;
    T_S16        decimalLen;
    T_WSTR_100    strTemp;
    T_DBL        dblTemp;

    AK_ASSERT_PTR(string, "Fwl_Dbl2String()", 0);

    if (num.Number < 0)        /* negative */
    {
        numlen--;
        maxlen--;
    }

    if (numlen > MAX_DBL_LEN)
        numlen = MAX_DBL_LEN;

    string[0] = (T_WCHR)('\0');
    if (numlen < 3)
    {
        return string;
    }

    if ((len + num.Power > maxlen) ||                                /* format: (-)x.xxxxExxx */
        (len + num.Power <= 0 && num.Power < (-1)*(maxlen-2)) ||    /* format: (-)0.xxxE-xxx */
        (len + num.Power > 0 && num.Power < (-1)*(maxlen-1)))        /* format: (-)x.xxxE-xxx */
    {
        dblTemp = Fwl_DblCopy(num);
        Fwl_DblIntTidyUp(&num, (T_S16)(1 - len));
        decimalLen = (T_S16)(Fwl_DblGetIntLen(dblTemp.Number)-(maxlen-2-Fwl_DblGetIntLen(num.Power)));
        if (num.Power < 0)        /* format: x.xxxE-xxx */
            decimalLen++;
        if (decimalLen > 0)
            Fwl_DblIntTidyUp(&dblTemp, (T_S16)(decimalLen*(-1)));
        Utl_UItoa(dblTemp.Number, strTemp, 10);
        Utl_UStrCpy(string, strTemp);        /* format: xxxxx */
        if (Utl_UStrLen(strTemp) > 1)        /* == 1: do not add "." */
        {
            if (dblTemp.Number > 0)
                Utl_UStrInsChr(string, (T_WCHR)('.'), 1);    /* format: x.xxxx */
            else
                Utl_UStrInsChr(string, (T_WCHR)('.'), 2);    /* format: -x.xxxx */
        }
        Utl_UStrCatChr(string, (T_WCHR)('e'), 1);        /* format: x.xxxxE */
        Utl_UItoa(num.Power, strTemp, 10);
        Utl_UStrCat(string, strTemp);        /* format: x.xxxxExx */
    }
    else if (num.Power >= 0)                                        /* fromat: xxxxxxxxxx */
    {
        Utl_UItoa(num.Number, string, 10);
        while (num.Power-- > 0)
            Utl_UStrCatChr(string, (T_WCHR)('0'), 1);    /* format: (-)xxxxxx000 */
    }
    else                                                            /* fromat: xxxx.xxxxx */
    {
        Utl_UItoa(num.Number, string, 10);    /* format: xxxxxxxx */
        if (num.Number >= 0)
            start = 0;
        else
            start = 1;
        if (len + num.Power > 0)            /* format: xxx.xxxxx */
        {
            start = (T_U8)(start + len + num.Power);
            Utl_UStrIns(string, _T("."), start);
        }
        else                                /* format: 0.xxxxxxxx */
        {
            while ((len + num.Power++) <= 0)
                Utl_UStrIns(string, _T("0"), start);    /* format: (-)000xxxxxx */
            Utl_UStrIns(string, _T("."), (T_U16)(start+1));    /* format: (-)0.00xxxxxx */
        }
    }

    return string;
}

/**
 * @brief Conver a string to a double number.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @return T_DBL 
 * @retval
 */
T_DBL Fwl_DblFromString(T_pWSTR string)
{
    T_DBL        result;
    T_WCHR        *strTemp;
    T_S16        len;
    T_S16        dot = Utl_UStrFnd(string, _T("."), 0);
    T_S16        exp = Utl_UStrFndChr(string, AK_EXP_CHR, 0);
    T_S16        redun;    /* redundance */

    AK_ASSERT_PTR(string, "Fwl_DblFromString()", Fwl_DblInit(1, 0));
    
    len = (T_U16)Utl_UStrLen(string);
    strTemp = (T_pWSTR)Fwl_Malloc((T_U32)(len+1) * sizeof(T_WCHR));
    if (strTemp == AK_NULL)        /* memory allocate error */
        return Fwl_DblInit(1, 0);

    if (exp >= 0)
    {
        Utl_UStrMid(strTemp, string, (T_U16)(exp+1), (T_U16)(len-1));
        result.Power = (T_S16)Utl_UAtoi(strTemp);
        Utl_UStrMid(strTemp, string, 0, (T_U16)(exp-1));
    }
    else
    {
        result.Power = 0;
        Utl_UStrCpy(strTemp, string);
    }
    len = (T_U16)Utl_UStrLen(strTemp);

    if (dot >= 0)
    {
        result.Power = (T_S16)(result.Power - (len - dot - 1));
        Utl_UStrDel(strTemp, dot, 1);
        len--;
    }

    redun = Fwl_DblGetAvaiLen(strTemp, AK_FALSE) - MAX_DBL_LEN;
    if (redun > 0)
    {
       // Utl_UStrDel(strTemp, (T_U16)(len-redun-1), redun);
        Utl_UStrDel(strTemp, (T_U16)(len-redun), redun);
        result.Power = (T_S16)(result.Power + redun);
    }

    result.Number = Utl_UAtoi(strTemp);

    strTemp = Fwl_Free(strTemp);

    return result;
}

/**
 * @brief Get the available byte of a double string.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @param T_BOOL zeroOK: AK_TRUE: begin with 0, AK_FALSE: begin with 1--9
 * @return T_DBL 
 * @retval
 */
T_U16 Fwl_DblGetAvaiLen(T_pCWSTR string, T_BOOL zeroOK)
{
    T_S16    i = 0;
    T_U16    avai = 0;

    AK_ASSERT_PTR(string, "Fwl_DblGetAvaiLen()", 0);

    while (string[i] != 0)
    {
        if (string[i] == (T_WCHR)('e'))
            break;
        if (string[i] != (T_WCHR)('-') && string[i] != (T_WCHR)('.'))
        {
            if (!(zeroOK))
            {
                if (string[i] != (T_WCHR)('0'))
                    zeroOK = AK_TRUE;
                else
                {
                    i++;
                    continue;
                }
            }
            avai++;
        }
        i++;
    }
    return avai;
}

/**
 * @brief Get exp value of a double stirng.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @return T_S16
 * @retval
 */
T_S16 Fwl_DblGetExp(T_pWSTR string, T_pWSTR expStr)
{
    T_S16        exp;

    AK_ASSERT_PTR(string, "Fwl_DblGetExp()", 0);
    AK_ASSERT_PTR(expStr, "Fwl_DblGetExp()", 0);

    exp = Utl_UStrFndChr(string, AK_EXP_CHR, 0);
    if (exp >= 0)
    {
        Utl_UStrMid(expStr, string, (T_U16)(exp+1), (T_U16)(Utl_UStrLen(string)-1));
        return (T_S16)Utl_UAtoi(expStr);
    }

    expStr[0] = (T_WCHR)('\0');
    return 0;
}

/**
 * @brief Get size of a integer number, not include sign.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_S16 num:
 * @return T_VOID
 * @retval
 */
T_U8 Fwl_DblGetIntLen(T_S32 num)
{
    T_U8 size = 0;

    if (num > 0)
        num *= (-1);
    do
    {
        size++;
        num /= 10;
    }
    while (num != 0);

    return size;
}

/**
 * @brief Didy up integer number of a double, the result of num will not change.
 * If power < 0, num->Number decrease, num->Power increase
 * If power > 0, num->Number increase, num->Power decrease
 * If power = 0, num do not change
 *
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL *num: double operator
 * @param T_S16 power:
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblIntTidyUp(T_DBL *num, T_S16 power)
{
    AK_ASSERT_PTR(num, "Fwl_DblIntTidyUp()", Fwl_DblInit(1, 0));

    num->Power = (T_S16)(num->Power - power);
    if (power < 0)
    {
        while (power < 0)
        {
            if (power == -1)    /* round in the last time */
            {
                if (Fwl_Abs(num->Number) % 10 >= 5)
                {
                    if (num->Number > 0)
                        num->Number = num->Number / 10 + 1;
                    else
                        num->Number = num->Number / 10 - 1;
                    if (Fwl_Abs(num->Number) % 10 == 0)        /* 999999 ==> 1000000 */
                    {
                        Fwl_DblTidyUp(num);
                    }
                }
                else
                    num->Number = num->Number / 10;
            }
            else
            {
                num->Number = num->Number / 10;
            }
            power++;
        }
    }
    else if (power > 0)
    {
        while (power > 0)
        {
            if (Fwl_DblGetIntLen(num->Number < MAX_DBL_LEN))
            {
                num->Number *= 10;
                power--;
            }
            //else
                //*num;
        }
    }
    return *num;
}

/**
 * @brief Didy up a double number
 * feature of the result:
 * 1. size of the integer number less or equal than MAX_DBL_LEN
 * 2. the last digital of the integer is not zero
 * 3. overflow judged
 *
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL *num: double operator
 * @return T_DBL
 * @retval
 */
T_DBL Fwl_DblTidyUp(T_DBL *num)
{
    T_U8 len;

    AK_ASSERT_PTR(num, "Fwl_DblTidyUp()", Fwl_DblInit(1, 0));

    if (num->Number == 0)
    {
        num->Power = 0;
        return *num;
    }

    len = Fwl_DblGetIntLen(num->Number);
    if (len > MAX_DBL_LEN)
        Fwl_DblIntTidyUp(num, (T_S16)(MAX_DBL_LEN - len));

    while ((num->Number != 0) && ((num->Number % 10) == 0))
    {
        num->Number /= 10;
        num->Power++;
    }

    if (num->Power > MAX_DBL_POWER)
        num->Power = MAX_DBL_POWER;

//    if (num.Number % 10 == 1 && (num.Number - 1) % 1000000 == 0)    /* x.000001 */
//    {
//        num.Number -= 1;
//    }
    return *num;
}

/**
 * @brief Didy up two double numbers to have same power number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param T_DBL *num1:
 * @param T_DBL *num2:
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_DblPowerMatch(T_DBL *num1, T_DBL *num2)
{
    T_S16    gap;
    T_U8    len;

    AK_ASSERT_PTR_VOID(num1, "Fwl_DblPowerMatch()");
    AK_ASSERT_PTR_VOID(num2, "Fwl_DblPowerMatch()");

    gap = (T_S16)(num1->Power - num2->Power);
    if (gap == 0)
        return;

    if (num1->Number == 0)
    {
        num1->Power = num2->Power;
        return;
    }
    else if (num2->Number == 0)
    {
        num2->Power = num1->Power;
        return;
    }

    if (gap > 0)    /* num1->Power > num2->Power */
    {
        len = Fwl_DblGetIntLen(num1->Number);
        if (len + gap > MAX_DBL_LEN)
        {
            Fwl_DblIntTidyUp(num2, (T_S16)(MAX_DBL_LEN - (len + gap)));
            Fwl_DblPowerMatch(num1, num2);
        }
        else
        {
            Fwl_DblIntTidyUp(num1, gap);
        }
    }
    else            /* num2->Power > num1->Power */
    {
        gap *= (-1);
        len = Fwl_DblGetIntLen(num2->Number);
        if (len + gap > MAX_DBL_LEN)
        {
            Fwl_DblIntTidyUp(num1, (T_S16)(MAX_DBL_LEN - (len + gap)));
            Fwl_DblPowerMatch(num1, num2);
        }
        else
        {
            Fwl_DblIntTidyUp(num2, gap);
        }
    }
    return;
}

/**
 * @brief Get cos value of an angle.
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S32
 * @retval cos value
 */
T_S32 Fwl_Cos(T_S16 angle)
{
#ifdef OS_WIN32
    return (T_S32)(cos(angle/10000.) * 10000);
#else
    return 0;
#endif
}

/**
 * @brief
 * Get sin value of an angle.
 * @author @b
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S32
 * @retval sin value
 */
T_S32 Fwl_Sin(T_S16 angle)
{
#ifdef OS_WIN32
    return (T_S32)(sin(angle/10000.) * 10000);
#else
    return 0;
#endif
}

/**
 * @brief
 * Get absolute value of 'num'.
 * @author @b
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 num
 * @return T_S16
 * @retval The absolute value of 'num'
 */
T_S32 Fwl_Abs(T_S32 num)
{
    if (num < 0)
        return num * (-1);
    else
        return num;
}

/**
 * @brief
 * Get Get power value of num
 * @author @b
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 num
 * @param T_U8 power
 * @return T_S16
 * @retval
 */
T_S32 Fwl_Power(T_S32 num, T_U8 power)
{
    T_S32 result = num;

    if (num == 0)
        return 0;

    while (power-- > 0)
        result *= num;
    return result / num;
}


static T_U32 randSeed;

T_VOID Fwl_RandSeed(T_VOID)
{
    randSeed = Fwl_GetTickCount();
}
/**
 create a random number 
 author: wjw
 date: 2003.11.19
 **/
T_U32 Fwl_GetRand(T_U32 maxVal)
{
    if (maxVal == 0)
        return 0;

#ifdef OS_ANYKA
    {
        return ((rand()+Fwl_GetTickCount())%maxVal);
    }
#endif
#if 0
    randSeed = (randSeed * 7 + 13) * 19;
    return (randSeed % maxVal);
    //return (gb.BsInfo.SysTime.wMilliseconds % maxVal);
#endif
#ifdef OS_WIN32
    return (rand()%maxVal);
#endif
}


/**
 * @brief calculate the GCD of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-1
 * @param T_S32 num1: num1 to calculate GCD
 * @param T_S32 num2: num2 to calculate GCD
 * @return T_S32
 * @retval
 */
T_S32 Fwl_GCD(T_S32 num1, T_S32 num2)
{
    T_S32 swp = 0;
    //make num1 is the lower number
    if (num1 > num2)
    {
       swp = num1;
       num1 = num2;
       num2 = swp;
    }

    while(0 != num1)
    {
        swp = num2%num1;
        num2 = num1;
        num1 = swp;
    }
    return num2;
}



T_S32 Fwl_LCM(T_S32 num1, T_S32 num2, T_S32 base)
{
    T_S32 mul = num1*num2;
    T_S32 i = 0;
    if(mul >= base)
        return mul / (Fwl_GCD(num1, num2));
    else
    {
        if(0==base%num1 && 0==base%num2)
            return base;    
        //make num1 is the larger
        if(num1 < num2)
        {
            T_S32 swp = num2;
            num2 = num1;
            num1 = swp;
        }

        i = base/num1;
        if(0 != base%num1)
            i = (i+1)*num1;
        else
            i = i*num1;


        do
        {
            if(0 == i%num2)
                return i;
            i += num1;
            AK_ASSERT_VAL(i+num1 >= i, "Fwl_LCM i overflow", 0);
        }while(AK_TRUE);
    }
    AK_ASSERT_VAL(AK_FALSE, "Fwl_LCM never run here!", 0);
    return 0;
}

/**
 * @brief to sort number from *pHead to *(pHead+count) quickly
 * @author Junhua Zhao
 * @date 2005-08-30
 * @param T_U32 *pHead: the head address of number array
 * @param T_U32 count: the count of number
 * @return T_U32    *
 * @retval
 */
T_U32   *Fwl_QuicklySort(T_U32 *pHead, T_S32 count)
{
    T_S32 lFlag = 0, rFlag = count;
    if (count<2)
        return pHead;

    while(AK_TRUE)
    {
        do{
            lFlag++;
        }while(*(pHead+lFlag)<*pHead && lFlag<rFlag);
            
        do{
            rFlag--;
        }while(*(pHead+rFlag)>*pHead && rFlag>0);
            
        if (lFlag >= rFlag)
            break;

        //*(pHead+lFlag)<=>*(pHead+rFlag)
        *(pHead+lFlag) ^= *(pHead+rFlag);
        *(pHead+rFlag) ^= *(pHead+lFlag);
        *(pHead+lFlag) ^= *(pHead+rFlag);
    }

    if (rFlag>0)
    {
        //*(pHead)<=>*(pHead+rFlag)
        *pHead ^= *(pHead+rFlag);
        *(pHead+rFlag) ^= *pHead;
        *pHead ^= *(pHead+rFlag);
    }

    Fwl_QuicklySort(pHead,rFlag);
    Fwl_QuicklySort(pHead+rFlag+1,count-rFlag-1);
    return pHead;
}



/**
    * @BRIEF    A 64bit unsigned integer subtract a 32 bit unsigned integer
    *           the minuend can't be less than the Subtrahend, otherwise it will fail!
    * @AUTHOR   Liu weijun
    * @DATE     2008-01-23
    * @PARAM    size64   - Minuend, a pointer to a 64 bit unsigned integer. 
    *                      And the result is stored by this pointer
    *           size32   - Subtrahend, a 32 bit unsigned integer 
    * @RETURN
    * @RETVAL   AK_FALSE - size64 is null,or size64 is less than size32
    * @RETVAL   AK_TRUE  - subtration success,and the result is stored by size64
*/
T_BOOL U64subU32(T_U64_INT *size64, T_U32 size32)
{
    AK_ASSERT_PTR(size64, "U64subU32: error! the ptr of T_U64_INT is null!", AK_FALSE);
    AK_ASSERT_VAL((!((size64->high == 0) && (size64->low < size32))), "U64subU32: size64 is less than size32, fail done!", AK_FALSE);

    if (size64->low >= size32)
    {
        size64->low -= size32;
    }
    else
    {
        if (size64->high > 0)
        {
            size64->high--;
            size64->low = 0xffffffff - size32 + size64->low;
        }
        else
        {
            //assert has dealed with this case. 
        }
    }

    return AK_TRUE;
}

/**
    * @BRIEF    A 64bit unsigned integer to compare with a 32 bit unsigned integer
    * @AUTHOR   Liu weijun
    * @DATE     2008-01-23
    * @PARAM    size64   - a pointer to a 64 bit unsigned integer. 
    *           size32   - a 32 bit unsigned integer
    * @RETURN
    * @RETVAL   -1  size64 is less than size32
    * @RETVAL   0   size64 is equal to size32
    * @RETVAL   1   size64 is larger than size32
*/
T_S8 U64cmpU32(T_U64_INT *size64, T_U32 size32)
{
    AK_ASSERT_PTR(size64, "U64cmpU32: error! the ptr of T_U64_INT is null!", -127);

    if (size64->high > 0)
        return 1;

    if (size64->low > size32)
        return 1;

    if (size64->low == size32)
        return 0;

    return -1;
}


/**
    * @BRIEF    A 64bit unsigned integer to right shift serveral bit
    * @AUTHOR   Liu weijun
    * @DATE     2008-01-23
    * @PARAM    size64   - a pointer to a 64 bit unsigned integer to be right-shift. 
    *                     and the result is stored by this parameter
    *           num      - bit number to shift
    *
    * @RETURN
    * @RETVAL   AK_FALSE  right-shift operation is failed
    * @RETVAL   AK_TRUE   right-shift operation is success, and the result is stored by size64
*/
T_BOOL  U64RightShift(T_U64_INT *size64, T_U32 num)
{
    AK_ASSERT_PTR(size64, "U64RightShift: error! the ptr of T_U64_INT is null!", AK_FALSE);
    AK_ASSERT_VAL((num <= 32), "U64RightShift: error! bit num of right-shifting is not valid!", AK_FALSE);

    if (size64->high < 1)
    {
        size64->low = size64->low >> num;
    }
    else
    {
        T_U32   temp = size64->high;

        size64->high = size64->high>>num;
        size64->low = size64->low >> num;
        size64->low +=  temp << (32 - num);
    }
    return AK_TRUE; 
}


/**
    * @BRIEF    long  integer add
    * @AUTHOR  wang xi
    * @DATE     2010-10-28
    * @PARAM    sumBuf   -  a long integer buffer and for restory the sum . 
    *                   augendBuf   -  a long integer buffer  for add  to sumBuf
    *                   msb      -  most hight bit
    *                   base     -  mathematic base number(must between 2 and 255)
    * @RETURN  T_S32
    * @RETVAL   the msb of sum
*/
T_S32 LongInt_Add(T_U8 *sumBuf,const T_U8 *augendBuf,T_S32 msb,T_U8 base)
{
	T_S32 i = 0 , tmpVal = 0;
	T_S32 mostHighBit = msb;
	T_BOOL bExistCarry = AK_FALSE;
	
	for (i=0;i<mostHighBit;i++)
	{
		sumBuf[i] = sumBuf[i] + augendBuf[i];
	}
	
	do
	{
		bExistCarry = AK_FALSE;
		for (i=0;i<mostHighBit;i++)
		{
			tmpVal = sumBuf[i];
			
			if (tmpVal >= base)
			{
				bExistCarry = AK_TRUE;
				sumBuf[i] = tmpVal%base;
				sumBuf[i+1] = sumBuf[i+1] + tmpVal/base;
				if (mostHighBit < (i+2))
				{
					mostHighBit = (i+2);
				}
			}
		}
	} while(bExistCarry);
	
	return mostHighBit;
}



/**
    * @BRIEF    64 bit  integer tanslate to Ascii string(base number is 10)
    * @AUTHOR  wang xi
    * @DATE     2010-10-28
    * @PARAM    high  /low -  value of 64 bit integer high / low 32 bit
    *                   strBuf   - restore the translate result
    * @RETURN  T_S32
    * @RETVAL   0 - translate successful
    * @RETVAL   other  - Failed
*/
T_S32 U64_Int2Str(T_S8 *strBuf,T_U32 high , T_U32 low)
{
	T_U8 sumbuf[22] = {0}; 
	T_U8 augendbuf[22] = {0}; 
	T_U8 i=0;
	T_U32 tmpVal = 0;
	T_U32 length = 0;
	T_U8 mostBit = 0;
	
	if (AK_NULL == strBuf)
	{
		return -1;
	}

	memset(sumbuf,0,sizeof(sumbuf));
	if (high > 0)
	{
		memset(augendbuf,0,sizeof(augendbuf));
	// "4294967296" 2^32
		augendbuf[0] = 6;augendbuf[1] = 9;augendbuf[2] = 2;augendbuf[3] = 7;augendbuf[4] = 6;
		augendbuf[5] = 9;augendbuf[6] = 4;augendbuf[7] = 9;augendbuf[8] = 2;augendbuf[9] = 4;
		mostBit = 10;
		length = high;
		while(length--)
		{
			mostBit = (T_U8)LongInt_Add(sumbuf,augendbuf,mostBit,10);
		}
	}
	
	tmpVal = low;
	i = 0;
	memset(augendbuf,0,sizeof(augendbuf));

	while (tmpVal >= 10)
	{
		augendbuf[i] = (T_U8)(tmpVal%10);
		tmpVal = tmpVal/10;
		i++;
	}
	
	augendbuf[i] = (T_U8)tmpVal;

	if (mostBit < (i+1))
	{
		mostBit = i+1;
	}
	mostBit = (T_U8)LongInt_Add(sumbuf,augendbuf,mostBit,10);

	for (i=0;i<mostBit;i++)
	{
		strBuf[i] = sumbuf[mostBit-1-i] + '0';
	}
	return 0;
}

/**
    * @BRIEF    A 64bit unsigned integer add
    * @AUTHOR  wangxi
    * @DATE     2011-05-17
*/
T_BOOL U64addU32(T_U64_INT *size64, T_U32 high32,T_U32 low32)
{
	T_U32 tmpsum = 0;

	AK_ASSERT_PTR(size64, "U64subU32: error! the ptr of T_U64_INT is null!", AK_FALSE);

	tmpsum = (T_U32)(size64->low + low32);

	if ((tmpsum < size64->low) || (tmpsum < low32))//溢出(有进位)
	{
		size64->high++;
	}

	size64->low = tmpsum;

	//===================================
	tmpsum = size64->high + high32;

	if ((tmpsum < size64->high) || (tmpsum < high32))//溢出(有进位)
	{
		size64->high = 0x7fffffff;
		return AK_FALSE;
	}
	else
	{
		if (tmpsum > 0x7fffffff)
		{
			size64->high = 0x7fffffff;
			return AK_FALSE;
		}
		else
		{
			size64->high = tmpsum;
			return AK_TRUE;
		}
	}

	return AK_FALSE;
}

/**
    * @BRIEF    64 bit  Divide 32 bit 
    * @AUTHOR  lu heshan
    * @DATE     2010-12-28
    * @PARAM   T_U64_INT
    * @PARAM   T_U32
    * @RETURN  T_U32
    * @RETVAL   0 is  successful anther num is fail
*/

T_U8 U64DivideU32(T_U64_INT size64, T_U32 size32, T_U32 *pQuto)
{
	T_U32 dividend_H ;  /*被除数高32位*/
	T_U32 dividend_L ;  /*被除数低32位*/
	T_U32 divisor_H ;   /*除数高32位*/
	T_U32 divisor_L ;   /*除数低32位*/

	T_U32 uFlag = 0x0;//用于标记分子高32位最高位
	T_U32 uQuto = 0x0;//商值
	T_U32 i = 0;
	T_U8 ret = 2;

    dividend_H = size64.high;
	dividend_L = size64.low;

	divisor_H = 0; 
	divisor_L = size32;  

	if (divisor_L == 0x0)
	{
		*pQuto = 0;
		return ret = 1;
	}
	else if(dividend_H/divisor_L)/*分子高32位除分母取整有值的话，说明64位的除式的商会超过32位，溢出*/
	{	 
		*pQuto = 0;
		return ret = 0xff;
	}
	else if (dividend_H == 0x0)/*如果分子和分母高32位都没有值，就可以使用32位的除法进行运算*/
	{
		*pQuto = dividend_L / divisor_L;
     	return ret = 0;
	} 
	else 
	{  
		/*
		if(divisor_H != 0)//在高32位有值的情况下
		{
			while(divisor_H)//分子分母同缩小N倍，N为divisor_H中最高的'1'的位置值，例：0010 0001 N=5。这里2的N次方作为分子分母的公约数
			{
				dividend_L /= 2;//实现分子循环右移位,
				if(dividend_H % 2)
				{    
					dividend_L += 0x80000000;
				}
				dividend_H /= 2;

				divisor_L /= 2;//实现分母循环右移位
				if(divisor_H %2)
				{    
					divisor_L += 0x80000000;
				}
				divisor_H /= 2;
			}
		}   
		*/
		for   (i = 0; i <= 31; i++) 
		{ 
			//    将被除数的低32位值dividend_L可以全部移到高32位dividend_H中进行计算(长除的特征)
			//    (1)uFlag标记被除数是否够除，如果最高位移出一个'1',说明被除数是足够大，可以除除数
			//           uFlag
			//      | 64 |63<-------------->|<--------------->0| 
			//    _____________________________________________
			//    |0(1) | dividend_H   | dividend_L   |0
			//    -----^-^----------------^-^----------------^-^ //这个步骤实际上是将处于低32的最高位移到高32位的最低位，实现循环移位了
			//                   -        
			//      ____________________
			//      | divisor_L   |
			//     31------------------0
			uFlag = (T_U32)dividend_H >> 31;//(1)        

			dividend_H = (dividend_H << 1)|(dividend_L >> 31);//(2)-1 
			dividend_L <<= 1; //(2)-2 

			uQuto <<= 1;
			if((dividend_H|uFlag) >= divisor_L)/*判断分子是否够除*/
			{ 
				dividend_H -= divisor_L;   
				uQuto++;   
			}   
		} 
	   *pQuto = uQuto;
	   return ret = 0;
	}
	return ret;
}

/**
    * @BRIEF    32 bit  Multiply 32 bit return T_U64
    * @AUTHOR  lu heshan
    * @DATE     2010-12-28
    * @PARAM   T_U32
    * @PARAM   T_U32
    * @RETURN   T_U64_INT
    * @RETVAL   0 is  successful anther num is fail
*/
T_U8 U32MultiplyU32_REU64(T_U32 multiplicator_0,T_U32 multiplicator_1,T_U64_INT *pQuto)
{

	#define LOWORD(l)           ((T_U16)(l))
	#define HIWORD(l)           ((T_U16)(((T_U32)(l) >> 16) & 0xFFFF))

	T_U32 A0 = 0, A1 = 0,B0 = 0,B1 = 0;
	T_U32 A0B0 = 0,A1B0 = 0,A0B1 = 0,A1B1 = 0;

	T_U8 ret = 2;
	  
	if((multiplicator_0 == 0) || multiplicator_1 == 0)
	{
	   	pQuto->high = 0;
	    pQuto->low = 0;
	    return ret = 1;
	}
	else if((multiplicator_0 != 0) && ((multiplicator_0 * multiplicator_1)/multiplicator_0   ==   multiplicator_1))
	{
		pQuto->high = 0;
		pQuto->low = multiplicator_0 * multiplicator_1;
		return ret = 0;
	}
	else
	{
		A0 = LOWORD( multiplicator_0 ); 
	    A1 = HIWORD( multiplicator_0 );
	    B0 = LOWORD( multiplicator_1 ); 
	    B1 = HIWORD( multiplicator_1 ); 
	  
	   	A0B0 = A0 * B0;
	   	A1B0 = A1 * B0 + HIWORD( A0B0 );
	   	A0B1 = A0 * B1 + LOWORD( A1B0 );
	   	A1B1 = A1 * B1 + HIWORD( A1B0 ) + HIWORD( A0B1 );
	  
	   	pQuto->high = A1B1;
	   	pQuto->low = multiplicator_0 * multiplicator_1;
		return ret = 0;
	}
	return ret;
}
/**
    * @BRIEF    64 bit  sub 64 bit return T_U64
    * @AUTHOR  lu heshan
    * @DATE     2010-12-28
    * @PARAM   T_U64
    * @PARAM   T_U64
    * @RETURN   T_U64_INT
    * @RETVAL   1 or -1 is  successful anther num is fail
*/
T_S8 U64subU64_ReU64(T_U64_INT *size64_1, T_U64_INT size64_2)
{
    T_S8 ret1 = 1;
	T_S8 ret = 0; 
	T_U64_INT *P_B;
	T_U64_INT *P_S;
	
	AK_ASSERT_PTR(size64_1, "U64subU32: error! the ptr of T_U64_INT is null!", AK_FALSE);

	if (size64_1->high < size64_2.high)
   	{
		ret1 = -1;
	}
	else
	{
	    if (size64_1->low < size64_2.low)
	   	{
			ret1 = -1;
		}
	}
	
	if (ret1 == -1)
	{
		P_B = &size64_2;
		P_S = size64_1;
	}
	else
	{
		P_B = size64_1;
		P_S = &size64_2;
	}
		
	if ((P_B->high == 0) && (P_S->high == 0))
   	{
		P_B->low -= P_S->low;
		size64_1->high = 0;
		size64_1->low = P_B->low;
		ret = 1;
		return ret1*ret;
	}
	else
	{
		if (P_B->low >= P_S->low)
		{
			P_B->low -= P_S->low;

			if (P_S->high > 0)
			{          
				P_B->high -= P_S->high;
			}
		}
		else
		{
			P_B->high--;
			P_B->low = 0xffffffff - P_S->low+ P_B->low;

			if (P_S->high > 0)
			{
                if (P_B->high >= P_S->high)
               	{
					P_B->high -= P_S->high;
				}
				else
				{
					return 0;
				}
			}
		}
		size64_1->high = P_B->high;
		size64_1->low = P_B->low;
		ret = 1;
		return ret1*ret;
	}
	return ret1*ret;
}


/* end of files */
