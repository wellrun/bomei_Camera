/**@file drv_sccb.h
 * @brief sccb interface driver header file
 *
 * This file provides SCCB APIs: SCCB initialization, write to SCCB & read data from SCCB.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @note refer to AK chip technical manual.
 */

#ifndef __DRV_SCCB_H__
#define __DRV_SCCB_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SCCB_Interface SCCB group
 *  @ingroup Drv_Lib
 */

/*@{*/
/**
 * @brief SCCB interface initialize function
 *
 * setup SCCB interface
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return T_VOID
 */
T_VOID sccb_init(T_U32 pin_scl, T_U32 pin_sda);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return T_BOOL return write success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_write_data(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return T_BOOL return write success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_write_data3(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return T_BOOL return write success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_write_data4(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @return T_U8
 * @retval read-back data
 */
T_U8 sccb_read_data(T_U8 daddr, T_U8 raddr);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return T_BOOL return read success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_read_data2(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return T_BOOL return read success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_read_data3(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return T_BOOL return read success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_read_data4(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif    /* end of __ARCH_SCCB_H__ */

