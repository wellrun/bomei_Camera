/**
 * @filename hal_freqmgr.h
 * @brief: frequency manager handle file
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  luheshan
 * @date    2012-05-16
 * @version 1.0
 * @ref
 */

#ifndef __HAL_FREQ_MANAGER_H__
#define __HAL_FREQ_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FREQ_INVALID_HANDLE             (-1) //invalid handle

typedef   T_S32   T_hFreq;//driver handle

typedef T_U32 (*FreqMgr_Callback)(T_VOID);


//********************************************************************

/**
 * @brief frequency manager mode to initial.
 *        1.will init asic frequency and driver list, 
 *        2.start A timer to check cpu usage factor
 *        3.set the pll and defaule asic,cpu 2X.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] cpu_max_freq: input the cpu max value,37xx chip the max is 280000000 (hz) 
 * @param[in] asic_max_freq: input the asic min value,(hz)
 * @param[in] FreqMgr_Callback: get cpu usage factor call back function.
 * @return T_BOOL
 * @retval values of  succeeds, the return value is AK_TRUE.
 */
T_BOOL FreqMgr_Init(T_U32 cpu_max_freq, T_U32 asic_min_freq, FreqMgr_Callback CpuCb);


/**
 * @brief frequency manager mode to request asic min frequency for the specifically driver.
 *        1.if a driver is need a specifically asic frequency for runing, this function will be call
 *        2.if call this function,must be call FreqMgr_CancelFreq function to cancel.
 *        3.when the driver is runing and can't to change frequency, we will call FreqMgr_Lock to lock.
 *    notes: if request asic > current asic,and is lock,will return fail.
 *         The DrvA request the least frequency frist,and then,DrvB request.them need runing the some time.
 *         If the DrvA must be locked,the DrvB request frequency success,but will no to adjust asic frequency.
 *         If DrvA'least frequency less then DrvB,the system may be make a mistake.
 *         so ,the driver request frequency and need lock,the max value asic must be request frist.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] ReqFreq: input the driver specifically min frequency for runing.
 * @return T_hFreq:request frequency handle.
 * @retval values of  fail, the return value is FREQ_INVALID_HANDLE.
 */
T_hFreq FreqMgr_RequestFreq(T_U32 ReqFreq);


/**
 * @brief frequency manager mode to cancle asic min frequency what had requested for the specifically driver.
 *        1.A driver had requested specifically asic frequency this function will be call by cancle request.
 *        2.If the driver had locked, when cancel,we will call FreqMgr_Lock to UnLock.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] hFreq: input a handle what request freq return the handle.
 * @return T_BOOL
 * @retval values of  succeeds, the return value is AK_TRUE.
 */
T_BOOL FreqMgr_CancelFreq(T_hFreq hFreq);


/**
 * @brief frequency manager mode to lock asic frequency not to be changed.
 *        1.if a driver when is runing and can't to change asic frequency, this function will be call
 *        2.if call this function,must be keep lock and unlock one by one.
 * @author luheshan
 * @date 2012-05-18
 * @paramT_VOID:
 * @return T_VOID:
 * @retval values of  T_VOID.
 */
T_VOID FreqMgr_Lock(T_VOID);

/**
 * @brief frequency manager mode to UnLock asic frequency.
 *        1.must be keep lock and unlock one by one.
 * @author luheshan
 * @date 2012-05-18
 * @paramT_VOID:
 * @return T_VOID:
 * @retval values of  T_VOID.
 */
T_VOID FreqMgr_UnLock(T_VOID);

/**
 * @brief frequency manager mode to get lock status
 * @author luheshan
 * @date 2012-05-22
 * @paramT_VOID:
 * @return T_BOOL:
 * @retval values of  TRUE is lock.
 */
T_BOOL FreqMgr_IsLock(T_VOID);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
