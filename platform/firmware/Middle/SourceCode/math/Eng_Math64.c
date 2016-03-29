/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：SMSCodeDe.c
 * Function：Encapsulate mathematic library. mainly including double operator.
 
 *
 * Author：
 * Date：
 * Version：       
 *
 * Reversion: 
 * Author: 
 * Date: 
 * Description: 用64位表示，代替原来的32位表示
**************************************************************************/


#include "Eng_Math.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Eng_String_UC.h"
#include "fwl_oscom.h"
#include "Eng_DataConvert.h"

//add by Jassduke
#include "Eng_Math64.h"
#include "Anyka_types.h"
#include "Eng_Debug.h"
#include "Eng_Math.h"


#ifdef OS_WIN32
#include "stdlib.h"
#endif

#if 0
extern T_pWSTR Utl_UI64toa(T_S64 intNum, T_U16 * strDest, T_U8 flag);
extern T_S64 Utl_UAtoi64(T_U16 * str);
#endif

/**
 * @brief Add two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num1:
 * @param T_DBL64 num2:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblAdd64(T_DBL64 num1, T_DBL64 num2)
{
    T_DBL64 result;

    Fwl_DblTidyUp64(&num1);
    Fwl_DblTidyUp64(&num2);
    if (Fwl_DblOverflow64(&num1) || Fwl_DblOverflow64(&num2))
        return Fwl_DblSetOverflow64(&result);

    Fwl_DblPowerMatch64(&num1, &num2);
    result.Number = num1.Number + num2.Number;
    if(0 == num1.Number || 0 == num2.Number){
        result.Power = num1.Power + num2.Power;
    }
    else{
        result.Power = num1.Power;
    }
    

    return Fwl_DblTidyUp64(&result);
}



/**
 * @brief Initial a two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_S64 number:
 * @param T_S16 power:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblInit64(T_S64 number, T_S16 power)
{
    T_DBL64 result;

    result.Number = number;
    result.Power = power;

    return Fwl_DblTidyUp64(&result);
}


/**
 * @brief Copy a double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblCopy64(T_DBL64 num)
{
    T_DBL64 result;

    result.Number = num.Number;
    result.Power = num.Power;

    return Fwl_DblTidyUp64(&result);
}


/**
 * @brief Minus two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num1:
 * @param T_DBL64 num2:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblMinus64(T_DBL64 num1, T_DBL64 num2)
{
    T_DBL64    result;

    Fwl_DblTidyUp64(&num1);
    Fwl_DblTidyUp64(&num2);
    if (Fwl_DblOverflow64(&num1) || Fwl_DblOverflow64(&num2))
        return Fwl_DblSetOverflow64(&result);

    Fwl_DblPowerMatch64(&num1, &num2);
    result.Number = num1.Number - num2.Number;
     if(0 == num1.Number || 0 == num2.Number){
        result.Power = num1.Power + num2.Power;
    }
    else{
        result.Power = num1.Power;
    }

    return Fwl_DblTidyUp64(&result);
}

/**
 * @brief Multiply two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num1:
 * @param T_DBL64 num2:
 * @return T_DBL64
 * @retval
 */

T_DBL64 Fwl_DblMulti64(T_DBL64 num1, T_DBL64 num2)
{
    T_DBL64    result;
    T_DBL64    dblTemp;
    T_S16    intLen1, intLen2;
    T_S8    sign = 1;

    //T_STR_INFO  ansiStrResult;

    Fwl_DblTidyUp64(&num1);
    Fwl_DblTidyUp64(&num2);
    if (Fwl_DblOverflow64(&num1) || Fwl_DblOverflow64(&num2))
        return Fwl_DblSetOverflow64(&result);
    
    /* high precision porcess */
    if (num1.Number < 0 && num2.Number > 0 || num1.Number > 0 && num2.Number < 0)
        sign = -1;
    num1.Number = Fwl_Abs64(num1.Number);
    num2.Number = Fwl_Abs64(num2.Number);
    intLen1 = Fwl_DblGetIntLen64(num1.Number);
    intLen2 = Fwl_DblGetIntLen64(num2.Number);
    if (intLen1 == MAX_DBL_LEN64 && intLen2 == MAX_DBL_LEN64)
    {
        if ((num1.Number % 10) > (num2.Number % 10))
        {
            dblTemp = Fwl_DblCopy64(num1);
            num1 = Fwl_DblIntTidyUp64(&num2, -1);
            num2 = Fwl_DblCopy64(dblTemp);
        }
        else
        {
            num1 = Fwl_DblIntTidyUp64(&num1, -1);
        }
    }
    else if (intLen1 > intLen2)
    {
        dblTemp = Fwl_DblCopy64(num1);
        num1 = Fwl_DblCopy64(num2);
        num2 = Fwl_DblCopy64(dblTemp);
    }
    /* now, num1 and num2 have the following relation:
        num1 > 0, num2 > 0,
        num1's Length < MAX_DBL_LEN64, num2's Length <= MAX_DBL_LEN64
        num1's Length <= num2's Length */

    result = Fwl_DblInit64(0, 0);

    while (num2.Number != 0)
    {
        dblTemp = Fwl_DblInit64((num2.Number % 10) * num1.Number, num2.Power);

        if (Fwl_DblOverflow64(&dblTemp))
            return Fwl_DblSetOverflow64(&dblTemp);
       
        result = Fwl_DblAdd64(result, dblTemp);

        num2.Number /= 10;
        num2.Power++;
    }
    result.Power = (T_S16)(result.Power + num1.Power);
    result.Number *= sign;

    return Fwl_DblTidyUp64(&result);
}



/**
 * @brief Double number divide.
 * 
 * @author Jassduke ZhuSiZhe
 * @date 2008-12-31 2005-12-04
 * @param T_DBL64 num1:
 * @param T_DBL64 num2:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblDivide64(T_DBL64 num1, T_DBL64 num2)
{
    T_DBL64    result;
    T_DBL64    dblTemp;
    T_U8    intLen1, intLen2;
    T_S64    int1, int2;
    T_S64    dValue;
    T_U8    count = 0;
    T_S16    power = 0;
    T_S8    sign = 1;

    Fwl_DblTidyUp64(&num1);
    Fwl_DblTidyUp64(&num2);
    if (Fwl_DblOverflow64(&num1) || Fwl_DblOverflow64(&num2))
        return Fwl_DblSetOverflow64(&result);

    if (num1.Number == 0 && num2.Number != 0)
        return Fwl_DblInit64(0, 0);
    if (num2.Number == 0)
        return Fwl_DblSetOverflow64(&result);
    if (num1.Number == num2.Number)
    {
        result = Fwl_DblInit64(1, (T_S16)(num1.Power - num2.Power));
        return Fwl_DblTidyUp64(&result);
    }

#if 0    
    /* here is the simple process: low precision */
    intLen1 = Fwl_DblGetIntLen64(num1.Number);
    intLen2 = Fwl_DblGetIntLen64(num2.Number);

    Fwl_DblIntTidyUp64(&num1, (T_S16)(MAX_DBL_LEN64-(intLen1)));
    if (intLen2 > MAX_DBL_LEN64/2+1)
        Fwl_DblIntTidyUp64(&num2, (T_S16)(MAX_DBL_LEN64/2+1-intLen2));

    result.Number = num1.Number / num2.Number;
    result.Power = num1.Power - num2.Power;
#endif
    /* high precision porcess */
    if (num1.Number < 0 && num2.Number > 0 || num1.Number > 0 && num2.Number < 0)
        sign = -1;
    num1.Number = Fwl_Abs64(num1.Number);
    num2.Number = Fwl_Abs64(num2.Number);
    intLen1 = Fwl_DblGetIntLen64(num1.Number);
    intLen2 = Fwl_DblGetIntLen64(num2.Number);
    if (intLen2 == MAX_DBL_LEN64)
    {
        num1 = Fwl_DblIntTidyUp64(&num1, (T_S16)(intLen2 - intLen1));
        if (num2.Number > num1.Number)
        {
            num2 = Fwl_DblIntTidyUp64(&num2, -1);        /* cut the last byte if num2 is too long and is too large */
            intLen2 = Fwl_DblGetIntLen64(num2.Number);
        }
        num1 = Fwl_DblTidyUp64(&num1);
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
            if (Fwl_DblGetIntLen64(int1) < MAX_DBL_LEN64)
                int1 *= 10;
            else
                break;
        }
    }
    result = Fwl_DblInit64(0, 0);
    int1 = num1.Number;
    dblTemp = Fwl_DblInit64(int1, (T_S16)(power * (-1)));
    while (count++ <= MAX_DBL_LEN64)    /* loop number: MAX_DBL_LEN64 + 1 */
    {
        if (power > 0)
        {
            dValue = dblTemp.Number / (num2.Number * Fwl_Power64(10, (T_U8)power));
            int1 = dblTemp.Number - num2.Number * dValue * Fwl_Power64(10, (T_U8)power);
            dblTemp = Fwl_DblInit64(int1, 1);
            Fwl_DblIntTidyUp64(&dblTemp, dblTemp.Power);
        }
        else
        {
            Fwl_DblIntTidyUp64(&dblTemp, dblTemp.Power);
            dValue = dblTemp.Number / num2.Number;
            int1 = dblTemp.Number - num2.Number * dValue;
            dblTemp = Fwl_DblInit64(int1, 1);
        }

        /* assign result */
        if (count <= MAX_DBL_LEN64)
        {
            result = Fwl_DblMulti64(result, Fwl_DblInit64(1, 1));
            result = Fwl_DblAdd64(result, Fwl_DblInit64(dValue, 0));
        }
        else
        {
            result = Fwl_DblAdd64(result, Fwl_DblInit64(dValue >= 5 ? 1: 0, 0));
        }
    }

    result.Power = (T_S16)(power + num1.Power - num2.Power - Fwl_DblGetIntLen64(result.Number) + 1);
    result.Number *= sign;

    return Fwl_DblTidyUp64(&result);
}



/**
 * @brief Compare two double numbers.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num1:
 * @param T_DBL64 num2:
 * @return T_S8
 * @retval 1: num1 > num2
 * @retval 0: num1 = num2
 * @retval -1: num1 < num2
 */
T_S8 Fwl_DblCompare64(T_DBL64 num1, T_DBL64 num2)
{
    Fwl_DblPowerMatch64(&num1, &num2);
    if (num1.Number > num2.Number)
        return 1;
    else if (num1.Number < num2.Number)
        return -1;
    return 0;
}



/**
 * @brief Compare two double numbers.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num1:
 * @param T_eCOMP_OPER oper:
 * @param T_DBL64 num2:
 * @return T_BOOL
 * @retval
 */
T_BOOL    Fwl_DblComOper64(T_DBL64 num1, T_eCOMP_OPER oper, T_DBL64 num2)
{
    T_BOOL ret = AK_FALSE;

    switch (oper) {
    case coLESSTHAN:
        if (Fwl_DblCompare64(num1, num2) == -1)
            ret = AK_TRUE;
        break;
    case coLESSEQUAL:
        if (Fwl_DblCompare64(num1, num2) != 1)
            ret = AK_TRUE;
        break;
    case coGREATTHAN:
        if (Fwl_DblCompare64(num1, num2) == 1)
            ret = AK_TRUE;
        break;
    case coGREATEQUAL:
        if (Fwl_DblCompare64(num1, num2) != -1)
            ret = AK_TRUE;
        break;
    case coEQUAL:
        if (Fwl_DblCompare64(num1, num2) == 0)
            ret = AK_TRUE;
        break;
    case coNOTEQUAL:
        if (Fwl_DblCompare64(num1, num2) != 0)
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
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num:
 * @param T_S16 power:
 * @return T_VOID
 * @retval
 */
T_DBL64 Fwl_DblPower64(T_DBL64 num, T_S16 power)
{
    T_DBL64 result;

    if(0 == num.Number){
        result.Number = 0;
        result.Power = 0;
        return result;
    }

    if(0 == power){
        result.Number = 1;
        result.Power = 0;
        return result;
    }

    result = num;

    while(power > 1){
        result.Number *= num.Number;
        result.Power += num.Power;
        Fwl_DblTidyUp64(&result);
        power--;
    }

    return result;
}




/**
 * @brief Get absolute value of a double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param const T_DBL64 *num:
 * @return T_VOID
 * @retval
 */
T_DBL64 Fwl_DblAbs64(const T_DBL64 *num)
{
    T_DBL64 result;

    AK_ASSERT_PTR(num, "Fwl_DblAbs64()", Fwl_DblInit64(1, 0));

    result.Number = Fwl_Abs64(num->Number);
    result.Power = num->Power;

    return result;
}



/**
 * @brief Get square root of a double number.
 * 
 * @author Jassduke
 * @date 2002-11-03
 * @param T_DBL64 num:
 * @return T_VOID
 * @retval
 */
T_DBL64 Fwl_DblSqrt64(T_DBL64 num)
{
    T_DBL64    result;
    T_S16    count = 100;        /* loop count */
    T_DBL64    x0;                    /* initial value to calculate */
    T_DBL64    eps =Fwl_DblInit64(1, -6); // Fwl_DblInit64(1, -6);

    T_DBL64    y[2];
    T_DBL64    d, p, x1;

    //add for test by Jassduke
    T_STR_INFO  ansiStrResult;
    

    Fwl_DblTidyUp64(&num);
    if (Fwl_DblOverflow64(&num) || num.Number < 0)
        return Fwl_DblSetOverflow64(&result);
    if (num.Number == 0)
        return Fwl_DblInit64(0, 0);

    if (num.Number == 1 && num.Power % 2 == 0)    /* 1 or 100 or 10000 or 1000000 ... */
    {
        return Fwl_DblInit64(1, (T_S16)(num.Power / 2));
    }

    /* Get a proper initial seed to x0 */
    x1 = Fwl_DblCopy64(num);    /* x1 is a temp variable here */

    //add for test by Jassduke
    Utl_I64toa(x1.Number, ansiStrResult, 10);
    Fwl_Print(C3, M_STANDBY, " ___________the x1 is %s ,Power : %d", ansiStrResult, x1.Power);  
    
    Fwl_DblIntTidyUp64(&x1, (T_S16)(2 - Fwl_DblGetIntLen64(x1.Number)));
    
    if (x1.Power % 2 != 0)
    {
        Fwl_DblIntTidyUp64(&x1, -1);
    }

    //add for test by Jassduke
    Utl_I64toa(x1.Number, ansiStrResult, 10);
    Fwl_Print(C3, M_STANDBY, " ___________after tidyup the x1 is %s ,Power : %d", ansiStrResult, x1.Power); 

    
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


    y[0] = Fwl_DblMinus64(Fwl_DblMulti64(x0, x0), num);        /* x*x - num */

    if(0==(y[0].Number))
        return Fwl_DblTidyUp64(&x0);
       
    y[1] = Fwl_DblMulti64(Fwl_DblInit64(2, 0), x0);            /* 2*x */

    d=Fwl_DblAdd64(eps, Fwl_DblInit64(1, 0));
    while ((Fwl_DblComOper64(d, coGREATEQUAL, eps)) && (count-- != 0))
    {
        if (Fwl_DblComOper64(Fwl_DblAdd64(Fwl_DblAbs64(&y[1]), Fwl_DblInit64(1, 0)), coEQUAL, Fwl_DblInit64(1, 0)))
        {
            return Fwl_DblSetOverflow64(&result);
        }
        x1= Fwl_DblMinus64(x0, Fwl_DblDivide64(y[0], y[1]));
        y[0] = Fwl_DblMinus64(Fwl_DblMulti64(x1, x1), num);        /* x*x - num */
        y[1] = Fwl_DblMulti64(Fwl_DblInit64(2, 0), x1);            /* 2*x */

        d = Fwl_DblMinus64(x1, x0);
        d = Fwl_DblAbs64(&d);
        p = Fwl_DblAbs64(&y[0]);
        if (Fwl_DblComOper64(p, coGREATTHAN, d))
            d = Fwl_DblCopy64(p);
        x0 = Fwl_DblCopy64(x1);
    }
    result = Fwl_DblCopy64(x1);    
    result = Fwl_DblAbs64(&result);
    if (result.Number % 10 == 1 && (result.Number - 1) % 1000000 == 0)    /* x.000001 */
    {
        result.Number -= 1;
    }
    return Fwl_DblTidyUp64(&result);
}



/**
 * @brief Get factorial value of a double number.
 * 
 * @author Jassduke
 * @date 2002-11-05
 * @param T_DBL64 num:
 * @return T_DBL64
 * @retval
 */
T_DBL64    Fwl_DblFact64(T_DBL64 num)
{
    T_DBL64    result;

    Fwl_DblTidyUp64(&num);
    if (Fwl_DblOverflow64(&num) || num.Number < 0)
        return Fwl_DblSetOverflow64(&result);
    if (num.Number == 0)
        return Fwl_DblInit64(1, 0);

    if (num.Power < 0)
        Fwl_DblIntTidyUp64(&num, num.Power);
#if 0
    if (num.Number >= 450)            /* for platform quick return */
        return Fwl_DblSetOverflow64(&result);
#endif

    if(Fwl_DblCompare64(num, Fwl_DblInit64(450,0)) >= 0){
        return Fwl_DblSetOverflow64(&result);
    }
        
    result = Fwl_DblInit64(1, 0);
    while (num.Number > 0)
    {
        result = Fwl_DblMulti64(result, num);
        if (Fwl_DblOverflow64(&result))
            break;
        num = Fwl_DblMinus64(num, Fwl_DblInit64(1, 0));
    }

    return Fwl_DblTidyUp64(&result);
}



/**
 * @brief Delete the last digital of the double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 *num:
 * @return T_DBL64
 * @retval
 */
T_DBL64    Fwl_DblBackspace64(T_DBL64 *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblBackspace64()", Fwl_DblInit64(1, 0));

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

    return Fwl_DblTidyUp64(num);
}


/**
 * @brief Check the double number is overflow or not.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param const T_DBL64 num:
 * @return T_BOOL
 * @retval
 */
T_BOOL Fwl_DblOverflow64(const T_DBL64 *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblSetOverflow64()", AK_TRUE);

    if (Fwl_Abs64(num->Power) + Fwl_DblGetIntLen64(num->Number) > MAX_DBL_POWER64)
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
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 *num:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblSetOverflow64(T_DBL64 *num)
{
    AK_ASSERT_PTR(num, "Fwl_DblSetOverflow64()", *num);

    num->Number = 1;
    num->Power = MAX_DBL_POWER64;
    return *num;
}



/**
 * @brief Conver a double number to a string.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 num: double number
 * @param T_pSTR string: string for return
 * @param T_U8 maxlen: maximum length of the string
 * @return T_pDATA 
 * @retval
 */
T_pWSTR Fwl_Dbl2String64(T_DBL64 num, T_pWSTR string, T_U16 maxlen)
{
#if 0
    T_U16        len = Fwl_DblGetIntLen64(num.Number);
    T_U16        numlen = maxlen;
    T_U16        start;
    T_S32        decimalLen;
    T_WSTR_100    strTemp;
    T_DBL64        dblTemp;

    
    AK_ASSERT_PTR(string, "Fwl_Dbl2String64()", 0);

    if (num.Number < 0)        /* negative */
    {
        numlen--;
        maxlen--;
    }

    if (numlen > MAX_DBL_LEN64){
        numlen = MAX_DBL_LEN64;
    }    

    string[0] = (T_WCHR)('\0');
    if (numlen < 3)
    {
        return string;
    }

    if ((len + num.Power > maxlen) ||                                /* format: (-)x.xxxxExxx */
        (len + num.Power <= 0 && num.Power < (-1)*(maxlen-2)) ||    /* format: (-)0.xxxE-xxx */
        (len + num.Power > 0 && num.Power < (-1)*(maxlen-1)))        /* format: (-)x.xxxE-xxx */
    {        
        dblTemp = Fwl_DblCopy64(num);
        Fwl_DblIntTidyUp64(&num, (T_S16)(1 - len));
        decimalLen = (T_S16)(Fwl_DblGetIntLen64(dblTemp.Number)-(maxlen-2-Fwl_DblGetIntLen64(num.Power)));
        if (num.Power < 0)        /* format: x.xxxE-xxx */
            decimalLen++;
        if (decimalLen > 0)
            Fwl_DblIntTidyUp64(&dblTemp, (T_S16)(decimalLen*(-1)));
		
        Utl_UI64toa(dblTemp.Number, strTemp, 10);
        Utl_UStrCpy(string, strTemp);        /* format: xxxxx */
        if (Utl_UStrLen(strTemp) > 1)        /* == 1: do not add "." */
        {
            if (dblTemp.Number > 0)
                Utl_UStrInsChr(string, (T_WCHR)('.'), 1);    /* format: x.xxxx */
            else
                Utl_UStrInsChr(string, (T_WCHR)('.'), 2);    /* format: -x.xxxx */
        }
        Utl_UStrCatChr(string, (T_WCHR)('e'), 1);        /* format: x.xxxxE */
        Utl_UI64toa(num.Power, strTemp, 10);
        Utl_UStrCat(string, strTemp);        /* format: x.xxxxExx */
    }
    else if (num.Power >= 0)                                        /* fromat: xxxxxxxxxx */
    {        
        Utl_UI64toa(num.Number, string, 10);     
        
        while (num.Power-- > 0)
            Utl_UStrCatChr(string, (T_WCHR)('0'), 1);    /* format: (-)xxxxxx000 */
    }
    else                                                            /* fromat: xxxx.xxxxx */
    {        
        Utl_UI64toa(num.Number, string, 10);    /* format: xxxxxxxx */
        
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
#endif
	return AK_NULL;
}


/**
 * @brief Conver a string to a double number.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @return T_DBL64 
 * @retval
 */
T_DBL64 Fwl_DblFromString64(T_pWSTR string)
{
	T_DBL64        result;
#if 0
    T_WCHR        *strTemp;
    T_S16        len;
    T_S16        dot = Utl_UStrFnd(string, _T("."), 0);
    T_S16        exp = Utl_UStrFndChr(string, AK_EXP_CHR, 0);
    T_S16        redun;    /* redundance */

    AK_ASSERT_PTR(string, "Fwl_DblFromString64()", Fwl_DblInit64(1, 0));
    
    len = Utl_UStrLen(string);
    strTemp = (T_pWSTR)Fwl_Malloc((T_U32)(len+1) * sizeof(T_WCHR));
    if (strTemp == AK_NULL)        /* memory allocate error */
        return Fwl_DblInit64(1, 0);

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
    len = Utl_UStrLen(strTemp);

    if (dot >= 0)
    {
        result.Power = (T_S16)(result.Power - (len - dot - 1));
        Utl_UStrDel(strTemp, dot, 1);
        len--;
    }

    redun = Fwl_DblGetAvaiLen(strTemp, AK_FALSE) - MAX_DBL_LEN64;

    Fwl_Print(C3, M_STANDBY, "strlenth is %d,  availen is %d", len, Fwl_DblGetAvaiLen(strTemp, AK_FALSE) );
    Fwl_Print(C3, M_STANDBY, "redundance num is %d", redun);
    
    if (redun > 0)
    {
        Utl_UStrDel(strTemp, (T_U16)(len-redun-1), redun);
        result.Power = (T_S16)(result.Power + redun);
    }

    result.Number = Utl_UAtoi64(strTemp);   // Utl_UAtoi(strTemp);

    strTemp = Fwl_Free(strTemp);

    return result;
	#else
	return result;
	#endif
}


#if 0
/**
 * @brief Get the available byte of a double string.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @param T_BOOL zeroOK: AK_TRUE: begin with 0, AK_FALSE: begin with 1--9
 * @return T_DBL64 
 * @retval
 */
T_U16 Fwl_DblGetAvaiLen64(T_pCWSTR string, T_BOOL zeroOK)
{
    T_S16    i = 0;
    T_U16    avai = 0;

    AK_ASSERT_PTR(string, "Fwl_DblGetAvaiLen64()", 0);

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

#endif


/**
 * @brief Get exp value of a double stirng.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param T_pSTR string: double string
 * @return T_S16
 * @retval
 */
 
T_S64 Fwl_DblGetExp64(T_pWSTR string, T_pWSTR expStr)
{
#if 0
    T_S16        exp;

    AK_ASSERT_PTR(string, "Fwl_DblGetExp64()", 0);
    AK_ASSERT_PTR(expStr, "Fwl_DblGetExp64()", 0);

    exp = Utl_UStrFndChr(string, AK_EXP_CHR, 0);
    if (exp >= 0)
    {
        Utl_UStrMid(expStr, string, (T_U16)(exp+1), (T_U16)(Utl_UStrLen(string)-1));
        return Utl_UAtoi64(expStr);
    }

    expStr[0] = (T_WCHR)('\0');
#endif
    return 0;
}


/**
 * @brief Get size of a integer number, not include sign.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_S16 num:
 * @return T_VOID
 * @retval
 */
T_S16 Fwl_DblGetIntLen64(T_S64 num)
{
    T_S16 size = 0;

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
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 *num: double operator
 * @param T_S16 power:
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblIntTidyUp64(T_DBL64 *num, T_S16 power)
{
    AK_ASSERT_PTR(num, "Fwl_DblIntTidyUp64()", Fwl_DblInit64(1, 0));

    num->Power = (T_S16)(num->Power - power);
    if (power < 0)
    {
        while (power < 0)
        {
            if (power == -1)    /* round in the last time */
            {
                if (Fwl_Abs64(num->Number) % 10 >= 5)
                {
                    if (num->Number > 0)
                        num->Number = num->Number / 10 + 1;
                    else
                        num->Number = num->Number / 10 - 1;
                    if (Fwl_Abs64(num->Number) % 10 == 0)        /* 999999 ==> 1000000 */
                    {
                        Fwl_DblTidyUp64(num);
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
            if (Fwl_DblGetIntLen64(num->Number < MAX_DBL_LEN64))
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
 * 1. size of the integer number less or equal than MAX_DBL_LEN64
 * 2. the last digital of the integer is not zero
 * 3. overflow judged
 *
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 *num: double operator
 * @return T_DBL64
 * @retval
 */
T_DBL64 Fwl_DblTidyUp64(T_DBL64 *num)
{
    T_U8 len;

    AK_ASSERT_PTR(num, "Fwl_DblTidyUp64()", Fwl_DblInit64(1, 0));

    if (num->Number == 0)
    {
        num->Power = 0;
        return *num;
    }

    len = Fwl_DblGetIntLen64(num->Number);
    if (len > MAX_DBL_LEN64)
        Fwl_DblIntTidyUp64(num, (T_S16)(MAX_DBL_LEN64 - len));

    while ((num->Number != 0) && ((num->Number % 10) == 0))
    {
        num->Number /= 10;
        num->Power++;
    }

    if (num->Power > MAX_DBL_POWER64){
        num->Power = MAX_DBL_POWER64;
    }
    else if(num->Power < (-1) * MAX_DBL_POWER64){
        num->Power = (-1) * MAX_DBL_POWER64;
    }

//    if (num.Number % 10 == 1 && (num.Number - 1) % 1000000 == 0)    /* x.000001 */
//    {
//        num.Number -= 1;
//    }

    return *num;
}



/**
 * @brief Didy up two double numbers to have same power number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param T_DBL64 *num1:
 * @param T_DBL64 *num2:
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_DblPowerMatch64(T_DBL64 *num1, T_DBL64 *num2)
{
    T_S16    gap;
    T_U8    len;

    T_S64     ExpDiff;
    T_S64     ExpLen1, ExpLen2;    

    AK_ASSERT_PTR_VOID(num1, "Fwl_DblPowerMatch64()");
    AK_ASSERT_PTR_VOID(num2, "Fwl_DblPowerMatch64()");

    ExpLen1 = Fwl_DblGetIntLen64(num1->Number) + num1->Power;
    ExpLen2 = Fwl_DblGetIntLen64(num2->Number) + num2->Power;
    
    ExpDiff = Fwl_Abs64(ExpLen1 - ExpLen2);


    gap = (T_S16)(num1->Power - num2->Power);
    if (gap == 0)
        return;

    if (num1->Number == 0)
    {
        num1->Power =0; //num2->Power;
        return;
    }
    else if (num2->Number == 0)
    {
        num2->Power = 0;// num1->Power;
        return;
    }

    //neither num1 nor num2 is zero
     if(ExpDiff > MAX_DBL_LEN64){
        if(ExpLen1 > ExpLen2){
            num2->Number = 0;
            num2->Power = 0;
        }
        else{
            num1->Number = 0;
            num1->Power = 0;
        }
        return;
    }   

    if (gap > 0)    /* num1->Power > num2->Power */
    {
        len = Fwl_DblGetIntLen64(num1->Number);      
        
        if (len + gap > MAX_DBL_LEN64)
        {
            Fwl_DblIntTidyUp64(num2, (T_S16)(MAX_DBL_LEN64 - (len + gap)));
            Fwl_DblPowerMatch64(num1, num2);
        }
        else
        {
            Fwl_DblIntTidyUp64(num1, gap);
        }
    }
    else            /* num2->Power > num1->Power */
    {
        gap *= (-1);
        len = Fwl_DblGetIntLen64(num2->Number);
        if (len + gap > MAX_DBL_LEN64)
        {
            Fwl_DblIntTidyUp64(num1, (T_S16)(MAX_DBL_LEN64 - (len + gap)));
            Fwl_DblPowerMatch64(num1, num2);
        }
        else
        {
            Fwl_DblIntTidyUp64(num2, gap);
        }
    }
    return;
}


/**
 * @brief Get cos value of an angle.
 * 
 * @author Jassduke
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S64
 * @retval cos value
 */
T_S64 Fwl_Cos64(T_S16 angle)
{
#ifdef OS_WIN32
    return (T_S64)(cos(angle/10000.) * 10000);
#else
    return 0;
#endif
}


/**
 * @brief
 * Get sin value of an angle.
 * @author @b
 * 
 * @author Jassduke
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S64
 * @retval sin value
 */
T_S64 Fwl_Sin64(T_S16 angle)
{
#ifdef OS_WIN32
    return (T_S64)(sin(angle/10000.) * 10000);
#else
    return 0;
#endif
}



/**
 * @brief
 * Get absolute value of 'num'.
 * @author @b
 * 
 * @author Jassduke
 * @date 2001-06-18
 * @param T_S16 num
 * @return T_S16
 * @retval The absolute value of 'num'
 */
T_S64 Fwl_Abs64(T_S64 num)
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
 * @author Jassduke
 * @date 2001-06-18
 * @param T_S16 num
 * @param T_U8 power
 * @return T_S16
 * @retval
 */
T_S64 Fwl_Power64(T_S64 num, T_U8 power)
{
    T_S64 result = num;

    if (num == 0)
        return 0;

    if(0 == power){
        return 1;
    }

    while(power > 1){
        result *= num;
        //Fwl_DblTidyUp64(&result);
        power--;
    }    
    
    return result ;
}


#if 0

static T_U32 randSeed;

T_VOID Fwl_RandSeed64(T_VOID)
{
    randSeed = Fwl_GetTickCount();
}


/**
 create a random number 
 author: wjw
 date: 2003.11.19
 **/
T_U32 Fwl_GetRand64(T_U32 maxVal)
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

#endif



/**
 * @brief calculate the GCD of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-1
 * @param T_S64 num1: num1 to calculate GCD
 * @param T_S64 num2: num2 to calculate GCD
 * @return T_S64
 * @retval
 */
T_S64 Fwl_GCD64(T_S64 num1, T_S64 num2)
{
    T_S64 swp = 0;
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



T_S64 Fwl_LCM64(T_S64 num1, T_S64 num2, T_S64 base)
{
    T_S64 mul = num1*num2;
    T_S64 i = 0;
    if(mul >= base)
        return mul / (Fwl_GCD64(num1, num2));
    else
    {
        if(0==base%num1 && 0==base%num2)
            return base;    
        //make num1 is the larger
        if(num1 < num2)
        {
            T_S64 swp = num2;
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
            AK_ASSERT_VAL(i+num1 >= i, "Fwl_LCM64 i overflow", 0);
        }while(AK_TRUE);
    }
    AK_ASSERT_VAL(AK_FALSE, "Fwl_LCM64 never run here!", 0);
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
T_S64   *Fwl_QuicklySort64(T_S64 *pHead, T_S64 count)
{
    T_S64 lFlag = 0, rFlag = count;
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

    Fwl_QuicklySort64(pHead,rFlag);
    Fwl_QuicklySort64(pHead+rFlag+1,count-rFlag-1);
    return pHead;
}


#if 0

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
    AK_ASSERT_VAL((!((size64->high == 0) && (size64->low < size32))), "U64subU32: size64 is less than size32, fail done!", AK_FALSE)

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
    AK_ASSERT_VAL(((num >= 0) && (num <= 32)), "U64RightShift: error! bit num of right-shifting is not valid!", AK_FALSE);

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

/* end of files */

#endif


