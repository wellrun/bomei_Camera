/**
 * @file eng_math64.h
 * @brief This header file is for OS related function prototype
 * 
 *
 */
#ifndef __FWL_MATH64_H__
/**
 * @def __FWL_MATH64_H__
 *
 */
 
#define __FWL_MATH64_H__


#define MAX_DBL_LEN64    18        /** <= 17 is legal, and 10 is overflow */
#define MAX_EXP_LEN64    3        /**< 3 is legal, and 4 is overflow */
#define MAX_DBL_POWER64    1000    /**< 999 is legal, and 1000 is overflow */



#ifndef  __FWL_MATH_H__


#define AK_EXP_CHR    (T_TCHR)('e')

#define BYTE_INC(var)           if ((var) < 0xff) (var)++
#define WORD_INC(var)           if ((var) < 0xffff) (var)++
#define DOUBLE_WORD_INC(var)    if ((var) < 0xffffffff) (var)++

///double numer compare operator 
typedef enum {
    coLESSTHAN,
    coLESSEQUAL,
    coGREATTHAN,
    coGREATEQUAL,
    coEQUAL,
    coNOTEQUAL
} T_eCOMP_OPER;            

#endif


///double number 
typedef struct {
    T_S64    Number;        /**< format: xxxxxxx */
    T_S16    Power;        /**< -999 -- 999 */
} T_DBL64;

/** @defgroup MATH Math interface 
 * @ingroup ENG
 */
/*@{*/                
/**
 * @brief Initial a two double number.
 * 
 * @author Jasduke
 * @date 2008-12-31
 * @param[in] number number
 * @param[in] power     power
 * @return T_DBL64
 * @retval
 */


T_DBL64    Fwl_DblInit64(T_S64 number, T_S16 power);
/**
 * @brief Copy a double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num num to be copied
 * @return a copy of input
 * @retval
 */
T_DBL64    Fwl_DblCopy64(T_DBL64 num);



/**
 * @brief Add two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 add number 1
 * @param[in] num2 add number 2
 * @return add result
 * @retval
 */
T_DBL64    Fwl_DblAdd64(T_DBL64 num1, T_DBL64 num2);


/**
 * @brief Minus two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 minus number 1
 * @param[in] num2 minus number 2
 * @return minus result
 * @retval
 */
T_DBL64    Fwl_DblMinus64(T_DBL64 num1, T_DBL64 num2);

/**
 * @brief Multiply two double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 multiply number 1
 * @param[in] num2 multiply number 2
 * @return  multiply result
 * @retval
 */
T_DBL64    Fwl_DblMulti64(T_DBL64 num1, T_DBL64 num2);
/**
 * @brief Double number divide.
 * 
 * @author Jassduke ZhuSiZhe
 * @date 2008-12-31 2005-12-04
 * @param[in] num1 divide number 1
 * @param[in] num2 divide number 2
 * @return  divide result
 * @retval
 */
T_DBL64    Fwl_DblDivide64(T_DBL64 num1, T_DBL64 num2);

/**
 * @brief Compare two double numbers.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 compare number 1
 * @param[in] num2 compare number 2
 * @return compare result
 * @retval 1  num1 > num2
 * @retval 0  num1 = num2
 * @retval -1  num1 < num2
 */
T_S8    Fwl_DblCompare64(T_DBL64 num1, T_DBL64 num2);


/**
 * @brief Compare two double numbers with specific operator
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 compare number 1
 * @param[in] oper operator
 * @param[in] num2 compare number 2
 * @return T_BOOL
 * @retval
 */
T_BOOL    Fwl_DblComOper64(T_DBL64 num1, T_eCOMP_OPER oper, T_DBL64 num2);


/**
 * @brief Get square root of a double number.
 * 
 * @author Jassduke
 * @date 2002-11-03
 * @param[in] num     input number
 * @return the input's square root
 * @retval
 */
T_DBL64    Fwl_DblSqrt64(T_DBL64 num);




/**
 * @brief Get factorial value of a double number.
 * 
 * @author Jassduke
 * @date 2002-11-05
 * @param[in] num     input number
 * @return the input's factorial value
 * @retval
 */
T_DBL64    Fwl_DblFact64(T_DBL64 num);




/**
 * @brief Get power value of a double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num     input number
 * @param[in] power     power value
 * @return power value of input
 * @retval
 */
T_DBL64    Fwl_DblPower64(T_DBL64 num, T_S16 power);




/**
 * @brief Get absolute value of a double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num     input number
 * @return absolute value of input
 * @retval
 */
T_DBL64    Fwl_DblAbs64(const T_DBL64 *num);


/**
 * @brief Delete the last digital of the double number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num     input number
 * @return Delete result.
 * @retval
 */
T_DBL64    Fwl_DblBackspace64(T_DBL64 *num);


/**
 * @brief Check the double number is overflow or not.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num     input number to check
 * @return AK_TRUE for overflow, AK_FALSE for not.
 * @retval
 */
T_BOOL    Fwl_DblOverflow64(const T_DBL64 *num);


/**
 * @brief set the double number to overflow.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num     input number to set
 * @return set result
 * @retval
 */
T_DBL64    Fwl_DblSetOverflow64(T_DBL64 *num);


/**
 * @brief Convert a double number to a string.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num        double number
 * @param[out] string    string for return
 * @param[in] maxlen    maximum length of the string
 * @return T_pDATA 
 * @retval
 */
T_pWSTR Fwl_Dbl2String64(T_DBL64 num, T_pWSTR string, T_U16 maxlen);


#if 0
/**
 * @brief Get the available byte of a double string.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param[in] string  double string
 * @param[in] zeroOK  AK_TRUE: begin with 0, AK_FALSE: begin with 1--9
 * @return T_DBL64 
 * @retval
 */
T_U16    Fwl_DblGetAvaiLen64(T_pCWSTR string, T_BOOL zeroOK);
#endif

/**
 * @brief Get exp value of a double string.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param[in] string  double string
 * @return T_S16
 * @retval
 */

T_S64    Fwl_DblGetExp64(T_pWSTR string, T_pWSTR expStr);


/**
 * @brief Get size of a integer number, not include sign.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] T_S16 num input number
 * @return size
 * @retval
 */
T_S16    Fwl_DblGetIntLen64(T_S64 num);


/**
 * @brief Convert a string to a double number.
 * 
 * @author Jassduke
 * @date 2002-11-04
 * @param[in] string  double string
 * @return double number
 * @retval
 */
T_DBL64    Fwl_DblFromString64(T_pWSTR string);


/**
 * @brief tidy up integer number of a double, the result of num will not change.
 * If power < 0, num->Number decrease, num->Power increase
 * If power > 0, num->Number increase, num->Power decrease
 * If power = 0, num do not change
 *
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num  double operator
 * @param[in] power power operator
 * @return T_DBL64
 * @retval
 */
T_DBL64    Fwl_DblIntTidyUp64(T_DBL64 *num, T_S16 power);

/**
 * @brief Didy up a double number
 *
 * feature of the result:
 * 1. size of the integer number less or equal than MAX_DBL_LEN
 * 2. the last digital of the integer is not zero
 * 3. overflow judged
 *
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num  double operator
 * @return T_DBL64
 * @retval
 */
T_DBL64    Fwl_DblTidyUp64(T_DBL64 *num);


 
/**
 * @brief tidy up two double numbers to have same power number.
 * 
 * @author Jassduke
 * @date 2008-12-31
 * @param[in] num1 input number 1
 * @param[in] num2 input number 2
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_DblPowerMatch64(T_DBL64 *num1, T_DBL64 *num2);


/**
 * @brief
 * Get absolute value of 'num'.
 * @author Jassduke
 * @date 2001-06-18
 * @param[in] num input number
 * @return T_S16
 * @retval The absolute value of 'num'
 */
T_S64    Fwl_Abs64(T_S64 num);


/**
 * @brief
 * Get Get power value of num
 * 
 * @author Jassduke
 * @date 2001-06-18
 * @param[in] num    input number
 * @param[in] power power value
 * @return T_S16
 * @retval
 */
T_S64    Fwl_Power64(T_S64 num, T_U8 power);


#if 0

/**
 * @brief create a  seed
 * 
 * @author wjw
 * @date 2003.11.19
 */
T_VOID Fwl_RandSeed64(T_VOID);




/**
 * @brief  create a random number 
 * 
 * @author wjw
 * @date 2003.11.19
 * @param[in]  maxVal the max value of random number created
 * @return  a random number
 * @retval
 */
T_U32 Fwl_GetRand64(T_U32 maxVal);



/**
 * @brief calculate the GCD of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-1
 * @param[in] num1  num1 to calculate GCD
 * @param[in] num2  num2 to calculate GCD
 * @return T_S64
 * @retval
 */

#endif
 
T_S64 Fwl_GCD64(T_S64 num1, T_S64 num2);


/**
 * @brief calculate the LCM of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @date 2005-12-1
 * @param[in] num1  num1 to calculate GCD
 * @param[in] num2  num2 to calculate GCD
 * @return T_S64
 * @retval
 */
T_S64 Fwl_LCM64(T_S64 num1, T_S64 num2, T_S64 base);


/**
 * @brief Get cos value of an angle.
 * 
 * @author Jassduke
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S64
 * @retval cos value
 */
T_S64 Fwl_Cos64(T_S16 angle);


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
T_S64 Fwl_Sin64(T_S16 angle);



/*@}*/


/**
 * @brief to sort number from *pHead to *(pHead+count) quickly
 * @author Junhua Zhao
 * @date 2005-08-30
 * @param T_S64 *pHead: the head address of number array
 * @param T_S64 count: the count of number
 * @return T_S64    *
 * @retval
 */

T_S64   *Fwl_QuicklySort64(T_S64 *pHead, T_S64 count);

#if 0
T_BOOL  U64subU32(T_U64_INT *size64, T_U32 size32);
T_S8    U64cmpU32(T_U64_INT *size64, T_U32 size32);
T_BOOL  U64RightShift(T_U64_INT *size64, T_U32 num);

#endif

#endif




