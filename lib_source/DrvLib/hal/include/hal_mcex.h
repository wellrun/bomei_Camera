/**
 * @FILENAME: hal_mcex.h
 * @BRIEF mcex implement file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liaozhijun
 * @DATE 2008-04-29
 * @VERSION 1.0
 * @REF
 */
#ifndef __HAL_MCEX_H__
#define __HAL_MCEX_H__

#define MCEX_WRITE_SEC_CMD	        35	/* adtc 		R1 */
#define MCEX_READ_SEC_CMD	        34	/* adtc			R1 */
#define MCEX_SEND_PSI		        36	/* adtc			R1 */
#define MCEX_CONTROL_TRM            37  /* ac           R1b */
#define MCEX_DIRECT_SECURE_READ     50  /* adtc         R1 */
#define MCEX_DIRECT_SECURE_WRITE    57  /* adtc         R1 */

#define MCEX_PSI_SR         0
#define MCEX_PSI_PR         4
#define MCEX_PSI_RNR        6

typedef enum tagMCEX_STATUS
{
    eMCEX_status_idle,
    eMCEX_status_cmd_progess,
    eMCEX_status_cmd_complete,
    eMCEX_statu_cmd_abort
}MCEX_STATUS;

typedef enum tagMCEX_ERROR
{
    eMCEX_error_none,
    eMCEX_error_auth,
    eMCEX_error_area_not_found,
    eMCEX_error_range_over,
    eMCEX_condition
}
MCEX_ERROR;

/**
 * @brief mcex init
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE init success
 * @retval AK_FALSE fail to init
 */
T_BOOL mcex_init(T_VOID);

/**
 * @brief mcex close
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_VOID
 */
 T_VOID mcex_close(T_VOID);

/**
 * @brief check if the present card support mcex function or not
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE the present card support mcex
 * @retval AK_FALSE the present card doesn't support mcex
 */
T_BOOL mcex_check(T_VOID);

/**
 * @brief open mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE open success
 * @retval AK_FALSE fail to open
 */
T_BOOL mcex_open(T_VOID);

/**
 * @brief reset mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE reset success
 * @retval AK_FALSE fail to reset
 */
T_BOOL mcex_reset(T_VOID);

/**
 * @brief get psi
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param type [in]: psi type
 * @param data [out]: data buffer
 * @return T_BOOL
 * @retval AK_TRUE get psi success
 * @retval AK_FALSE fail to get psi
 */
T_BOOL mcex_get_psi(T_U32 type, T_U8 *data);

/**
 * @brief get timeout for mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param read_timeout [in]: timeout for read operation
 * @param write_timeout [in]: timeout for write operation
 * @return T_BOOL
 * @retval AK_TRUE reset success
 * @retval AK_FALSE fail to reset
 */
T_BOOL mcex_get_timeout(T_U32 *read_timeout, T_U32 *write_timeout);

/**
 * @brief write data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param mode [in]: 
 * @param data [in]: data to be written
 * @param blk_size [in]: block size
 * @return T_BOOL
 * @retval AK_TRUE write success
 * @retval AK_FALSE fail to write
 */
 T_BOOL mcex_write(T_U32 mode, T_U8 *data, T_U32 blk_size);
 
/**
 * @brief read data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param data [out]: data to be read
 * @param blk_size [in]: block size
 * @return T_BOOL
 * @retval AK_TRUE read success
 * @retval AK_FALSE fail to read
 */
 T_BOOL mcex_read(T_U8 *data, T_U32 blk_size);


/** @} */

#endif

