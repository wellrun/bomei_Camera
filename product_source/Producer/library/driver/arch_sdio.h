/**
 * @file arch_sdio.h
 * @brief list SDIO card operation interfaces.
 *
 * This file define and provides functions of SDIO card
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-06-17
 * @version 2.0 for AK88xx
 */

#ifndef __ARCH_SDIO_H__
#define __ARCH_SDIO_H__

/** @addtogroup MMC_SD_SDIO
 *  @ingroup Drv_Lib
 */
/*@{*/

typedef T_VOID (*T_SDIO_INT_HANDLER)(T_VOID);



/**
 * @brief initial sdio or combo card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] bus_mode bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
 * @return T_BOOL
 * @retval AK_TRUE  set initial successful, card type is sdio or combo
 * @retval AK_FALSE set initial fail,card type is not sdio or combo
 */
T_BOOL sdio_initial(T_U8 bus_mode);


/**
 * @brief enable specifical fuction in sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to enable
 * @return T_BOOL
 * @retval AK_TRUE enable successfully
 * @retval AK_FALSE enable failed
 */
T_BOOL sdio_enable_func(T_U8 func);


/**
 * @brief set block length to sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to set block length
 * @param[in] block_len  block length to set
 * @return T_BOOL
 * @retval AK_TRUE enable successfully
 * @retval AK_FALSE enable failed
 */
T_BOOL sdio_set_block_len(T_U8 func, T_U32 block_len);


/**
 * @brief  set sdio interrupt callback function
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] cb callback function
 * @return T_BOOL
 * @retval AK_TRUE set successfully
 * @retval AK_FALSE set failed
 */
T_BOOL sdio_set_int_callback(T_SDIO_INT_HANDLER cb);

 
/**
 * @brief read one byte  from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] addr register address to read
 * @param[in] rdata data buffer for read data
 * @return T_BOOL
 * @retval AK_TRUE read successfully
 * @retval AK_FALSE read failed
 */
T_BOOL sdio_read_byte(T_U8 func, T_U32 addr,  T_U8 *rdata);

 
/**
 * @brief write one byte to sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to write
 * @param[in] addr register address to write
 * @param[in] wdata the write byte
 * @return T_BOOL
 * @retval AK_TRUE write successfully
 * @retval AK_FALSE write failed
 */
T_BOOL sdio_write_byte(T_U8 func, T_U32 addr, T_U8 wdata);

 
/**
 * @brief read multiple byte or block from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] src register address to read
 * @param[in] count data size(number of byte) to read
 * @param[in] opcode fixed address or increasing address
 * @param[in] rdata data buffer for read data
 * @return T_BOOL
 * @retval AK_TRUE read successfully
 * @retval AK_FALSE read failed
 */
T_BOOL sdio_read_multi(T_U8 func, T_U32 src, T_U32 count, T_U8 opcode, T_U8 rdata[]);

 
/**
 * @brief write multiple byte or block from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] dest register address to read
 * @param[in] count data size(number of byte) to read
 * @param[in] opcode fixed address or increasing address
 * @param[in] wdata the wirte data
 * @return T_BOOL
 * @retval AK_TRUE write successfully
 * @retval AK_FALSE write failed
 */
T_BOOL sdio_write_multi(T_U8 func, T_U32 dest, T_U32 count, T_U8 opcode, T_U8 wdata[]);


/**
 * @brief select or deselect a sdio device
 *
 * the card is selected by its own relative address and gets deselected by any other address; address 0 deselects all
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] addr the rca of  the card which will be selected
 * @return T_BOOL
 * @retval AK_TRUE  select or deselect successfully
 * @retval AK_FALSE  select or deselect failed
 */
T_BOOL sdio_select_card(T_U32 addr);

/*@}*/

#endif //__ARCH_SDIO_H__

