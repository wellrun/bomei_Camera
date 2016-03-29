/**
 * @file eng_math.h
 * @brief This header file is for OS related function prototype
 * 
 *
 */
#ifndef __FWL_MATH_H__
/**
 * @def __FWL_MATH_H__
 *
 */
#define __FWL_MATH_H__

#include "anyka_types.h"

#define MAX_DBL_LEN    8        /**< 9 is legal, and 10 is overflow */
#define MAX_EXP_LEN    3        /**< 3 is legal, and 4 is overflow */
#define MAX_DBL_POWER    1000    /**< 999 is legal, and 1000 is overflow */
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

///double number 
typedef struct {
    T_S32    Number;        /**< format: xxxxxxx */
    T_S16    Power;        /**< -999 -- 999 */
} T_DBL;

/** @defgroup MATH Math interface 
 * @ingroup ENG
 */
/*@{*/                
/**
 * @brief Initial a two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] number number
 * @param[in] power     power
 * @return T_DBL
 * @retval
 */
T_DBL    Fwl_DblInit(T_S32 number, T_S16 power);
/**
 * @brief Copy a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num num to be copied
 * @return a copy of input
 * @retval
 */
T_DBL    Fwl_DblCopy(T_DBL num);
/**
 * @brief Add two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 add number 1
 * @param[in] num2 add number 2
 * @return add result
 * @retval
 */
T_DBL    Fwl_DblAdd(T_DBL num1, T_DBL num2);
/**
 * @brief Minus two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 minus number 1
 * @param[in] num2 minus number 2
 * @return minus result
 * @retval
 */
T_DBL    Fwl_DblMinus(T_DBL num1, T_DBL num2);
/**
 * @brief Multiply two double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 multiply number 1
 * @param[in] num2 multiply number 2
 * @return  multiply result
 * @retval
 */
T_DBL    Fwl_DblMulti(T_DBL num1, T_DBL num2);
/**
 * @brief Double number divide.
 * 
 * @author ZouMai ZhuSiZhe
 * @date 2002-11-02 2005-12-04
 * @param[in] num1 divide number 1
 * @param[in] num2 divide number 2
 * @return  divide result
 * @retval
 */
T_DBL    Fwl_DblDivide(T_DBL num1, T_DBL num2);
/**
 * @brief Compare two double numbers.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 compare number 1
 * @param[in] num2 compare number 2
 * @return compare result
 * @retval 1  num1 > num2
 * @retval 0  num1 = num2
 * @retval -1  num1 < num2
 */
T_S8    Fwl_DblCompare(T_DBL num1, T_DBL num2);
/**
 * @brief Compare two double numbers with specific operator
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 compare number 1
 * @param[in] oper operator
 * @param[in] num2 compare number 2
 * @return T_BOOL
 * @retval
 */
T_BOOL    Fwl_DblComOper(T_DBL num1, T_eCOMP_OPER oper, T_DBL num2);
/**
 * @brief Get square root of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-03
 * @param[in] num     input number
 * @return the input's square root
 * @retval
 */
T_DBL    Fwl_DblSqrt(T_DBL num);
/**
 * @brief Get factorial value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-05
 * @param[in] num     input number
 * @return the input's factorial value
 * @retval
 */
T_DBL    Fwl_DblFact(T_DBL num);
/**
 * @brief Get power value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num     input number
 * @param[in] power     power value
 * @return power value of input
 * @retval
 */
T_DBL    Fwl_DblPower(T_DBL num, T_S16 power);
/**
 * @brief Get absolute value of a double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num     input number
 * @return absolute value of input
 * @retval
 */
T_DBL    Fwl_DblAbs(const T_DBL *num);
/**
 * @brief Delete the last digital of the double number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num     input number
 * @return Delete result.
 * @retval
 */
T_DBL    Fwl_DblBackspace(T_DBL *num);
/**
 * @brief Check the double number is overflow or not.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num     input number to check
 * @return AK_TRUE for overflow, AK_FALSE for not.
 * @retval
 */
T_BOOL    Fwl_DblOverflow(const T_DBL *num);
/**
 * @brief set the double number to overflow.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num     input number to set
 * @return set result
 * @retval
 */
T_DBL    Fwl_DblSetOverflow(T_DBL *num);
/**
 * @brief Convert a double number to a string.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num        double number
 * @param[out] string    string for return
 * @param[in] maxlen    maximum length of the string
 * @return T_pDATA 
 * @retval
 */
T_pWSTR Fwl_Dbl2String(T_DBL num, T_pWSTR string, T_U8 maxlen);
/**
 * @brief Get the available byte of a double string.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param[in] string  double string
 * @param[in] zeroOK  AK_TRUE: begin with 0, AK_FALSE: begin with 1--9
 * @return T_DBL 
 * @retval
 */
T_U16    Fwl_DblGetAvaiLen(T_pCWSTR string, T_BOOL zeroOK);
/**
 * @brief Get exp value of a double string.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param[in] string  double string
 * @return T_S16
 * @retval
 */
T_S16    Fwl_DblGetExp(T_pWSTR string, T_pWSTR expStr);
/**
 * @brief Get size of a integer number, not include sign.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] T_S16 num input number
 * @return size
 * @retval
 */
T_U8    Fwl_DblGetIntLen(T_S32 num);
/**
 * @brief Convert a string to a double number.
 * 
 * @author ZouMai
 * @date 2002-11-04
 * @param[in] string  double string
 * @return double number
 * @retval
 */
T_DBL    Fwl_DblFromString(T_pWSTR string);
/**
 * @brief tidy up integer number of a double, the result of num will not change.
 * If power < 0, num->Number decrease, num->Power increase
 * If power > 0, num->Number increase, num->Power decrease
 * If power = 0, num do not change
 *
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num  double operator
 * @param[in] power power operator
 * @return T_DBL
 * @retval
 */
T_DBL    Fwl_DblIntTidyUp(T_DBL *num, T_S16 power);
/**
 * @brief Didy up a double number
 *
 * feature of the result:
 * 1. size of the integer number less or equal than MAX_DBL_LEN
 * 2. the last digital of the integer is not zero
 * 3. overflow judged
 *
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num  double operator
 * @return T_DBL
 * @retval
 */
T_DBL    Fwl_DblTidyUp(T_DBL *num);
/**
 * @brief tidy up two double numbers to have same power number.
 * 
 * @author ZouMai
 * @date 2002-11-02
 * @param[in] num1 input number 1
 * @param[in] num2 input number 2
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_DblPowerMatch(T_DBL *num1, T_DBL *num2);
/**
 * @brief
 * Get absolute value of 'num'.
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] num input number
 * @return T_S16
 * @retval The absolute value of 'num'
 */
T_S32    Fwl_Abs(T_S32 num);
/**
 * @brief
 * Get Get power value of num
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] num    input number
 * @param[in] power power value
 * @return T_S16
 * @retval
 */
T_S32    Fwl_Power(T_S32 num, T_U8 power);

/**
 * @brief create a  seed
 * 
 * @author wjw
 * @date 2003.11.19
 */
T_VOID Fwl_RandSeed(T_VOID);

/**
 * @brief  create a random number 
 * 
 * @author wjw
 * @date 2003.11.19
 * @param[in]  maxVal the max value of random number created
 * @return  a random number
 * @retval
 */
T_U32 Fwl_GetRand(T_U32 maxVal);
/**
 * @brief calculate the GCD of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-1
 * @param[in] num1  num1 to calculate GCD
 * @param[in] num2  num2 to calculate GCD
 * @return T_S32
 * @retval
 */
T_S32 Fwl_GCD(T_S32 num1, T_S32 num2);
/**
 * @brief calculate the LCM of two numbers
 *
 * @author @b LiaoJianhua
 *
 * @date 2005-12-1
 * @param[in] num1  num1 to calculate GCD
 * @param[in] num2  num2 to calculate GCD
 * @return T_S32
 * @retval
 */
T_S32 Fwl_LCM(T_S32 num1, T_S32 num2, T_S32 base);

/**
 * @brief Get cos value of an angle.
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 angle
 * @return T_S32
 * @retval cos value
 */
T_S32 Fwl_Cos(T_S16 angle);

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
T_S32 Fwl_Sin(T_S16 angle);


/**
    * @BRIEF    64 bit  integer tanslate to Ascii string(base number is 10)
    * @AUTHOR  wang xi
    * @DATE     2010-10-28
    * @PARAM    high  /low -  64 bit integer high 32 bit  and low 32 bit
    *                   strBuf   - restore the translate result
    * @RETURN  T_S32
    * @RETVAL   0 - translate successful
    * @RETVAL   other  - Failed
*/
T_S32 U64_Int2Str(T_S8 *strBuf,T_U32 high , T_U32 low);

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
T_S32 LongInt_Add(T_U8 *sumBuf,const T_U8 *augendBuf,T_S32 msb,T_U8 base);

/*@}*/

T_U32   *Fwl_QuicklySort(T_U32 *pHead, T_S32 count);
T_BOOL  U64subU32(T_U64_INT *size64, T_U32 size32);
T_S8    U64cmpU32(T_U64_INT *size64, T_U32 size32);
T_BOOL  U64RightShift(T_U64_INT *size64, T_U32 num);
T_BOOL  U64addU32(T_U64_INT *size64, T_U32 high32,T_U32 low32);
T_U8 U64DivideU32(T_U64_INT size64, T_U32 size32, T_U32 *pQuto);
/**
    * @BRIEF    32 bit  Multiply 32 bit return T_U64
    * @AUTHOR  lu heshan
    * @DATE     2010-12-28
    * @PARAM   T_U32
    * @PARAM   T_U32
    * @RETURN   T_U64_INT
    * @RETVAL   0 is  successful anther num is fail
*/
T_U8 U32MultiplyU32_REU64(T_U32 multiplicator_0,T_U32 multiplicator_1,T_U64_INT *pQuto);

/**
    * @BRIEF    64 bit  sub 64 bit return T_U64
    * @AUTHOR  lu heshan
    * @DATE     2010-12-28
    * @PARAM   T_U64
    * @PARAM   T_U64
    * @RETURN   T_U64_INT
    * @RETVAL   1 or -1 is  successful anther num is fail
*/
T_S8 U64subU64_ReU64(T_U64_INT *size64_1, T_U64_INT size64_2);

#endif


