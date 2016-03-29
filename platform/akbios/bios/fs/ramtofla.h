/**
 * @file 
 * @brief Media access control
 *
 * This file provides all the APIs of accessing storage  media¡­
 * 1.chage page/block address access to byte address access (ram flash disk) 
 * 2.change usb packet read/write(64 byte per packet) to page/block access
 * 3.read the data from media to sdram buffer,or write the data from sdram buffer to meida
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Deng Jian
 * @date 2005-08-05
 * @version 1.0
 */


#ifndef 				__RAMTOFLA_H__
#define 				__RAMTOFLA_H__




/** @defgroup SD 
 *	@ingroup M3PLATFORM
 */
/*@{*/

#include "anyka_types.h"



/** @brief Access media type define
 *
 *	define media types here
 */
typedef enum {

    NO_MEDIA = 0 ,
    //SDRAM_DISK   ,
    NANDFLASH_DISK,
    SDCARD_DISK,
    NANDRESERVE_ZONE,
    UHOST_DISK,
    DUMMYDISK,
	MMC_DISK,
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
    SDCAED_ZONE,
#endif	
}T_ACCESS_MEDIUM; 


/**
 * @brief: set the access media type, pc access disk without mbr
 *           only can access 1 disk
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] type_index: media type
 * @return T_BOOL
 * @retval  success return AK_TURE; else AK_FALSE
 */
T_BOOL set_access_type(T_ACCESS_MEDIUM type_index);



/**
 * @brief: set current media type
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] T_ACCESS_MEDIUM set_val: media type
 * @return T_VOID
 */
T_VOID set_current_media(T_ACCESS_MEDIUM set_val);

/**
 * @brief: set current media type and logic block size
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] T_ACCESS_MEDIUM set_val: media type
 * @param[in] T_U32 byte_per_logic: logic block size
 * @return T_VOID
 */
T_VOID set_current_media_info(T_ACCESS_MEDIUM set_val,T_U32 byte_per_logic);

T_U8 Usb_FlushAllDisk(T_BOOL NeedFlush);
T_VOID Usb_MediumInfoInit(T_VOID);
T_VOID Usb_DeInitFs(T_VOID);
T_VOID Usb_InitFs(T_VOID);


/**
 * @brief:  mount the access media with logic unit number,max support 16 LUN
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] T_ACCESS_MEDIUM disk_type: media type
 * @return T_BOOL
 * @retval  success return AK_TURE; else AK_FALSE
 */
T_BOOL usb_MountLUN (T_ACCESS_MEDIUM disk_type);

/**
 * @brief: set the sequential page data informtion that would to be read
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] :start_block_address: the start block address of read
 * @param[in] :block_count: the sequential block count which we want to read
 * @return T_VOID
 */
 T_VOID set_read_page_info(T_U32 start_block_address,T_U32 block_count);
 
 
 
 
 
 /**
 * @brief: set the sequential page data informtion that would to be write
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] :start_block_address: the start block address of write
 * @param[in] :block_count: the sequential block count which we want to write
 * @return T_VOID
 */
 T_VOID set_write_page_info(T_U32 start_block_address,T_U32 block_count);
 
 
 
 
 /**
 * @brief: get the bulk in data which would be transfered in USB cable(or other transfer)  
 * 
 *	in this module, we define a static array,static T_U32 buffer_data_in[DEF_BUF32_IN_SIZE],
 *          get the transfer data from the buffer,if out the buffer, refresh buffer.  
 * @author Deng Jian
 * @date 2005-08-04
 * @param[out] :*buf, the data we want to get
 * @param[in] :usb_pkt_size32, the 32 bit size of usb packet(64 byte =16).( it can be use in other transfer
 * @return T_VOID
 */
T_VOID get_bulkin_data( T_U32 **buf,T_U32 usb_pkt_size32);
 
 
 
 
 
 
 /**
 * @brief: save the bulk out data which would transfered in USB cable(or other transfer)  
 * 
 *	 in this module, we define tow static array for ping/pong transfer,
 *	   static T_U32 buffer_data_out1[DEF_BUF32_OUT_SIZE];
 *	   static T_U32 buffer_data_out2[DEF_BUF32_OUT_SIZE];
 *	   save the transfer data to the buffer,if full the buffer or tranfer end,save buffer to media.  
 * @author Deng Jian
 * @date 2005-08-04
 * @param[in] :fifo_addr, the data we want to save
 * @param[in] :usb_pkt_size32, the 32 bit size of usb packet(64 byte =16).( it can be use in other transfer
 * @return T_VOID
 */
 T_VOID put_bulkout_data( T_U32 fifo_addr,T_U32 usb_pkt_size32);
 
 /**
 * @brief: start or end prevent remove media
 * @author Deng Jian
 * @date 2005-08-04
 * @return T_VOID
 */
T_VOID prevent_remove_media(T_VOID);

/** @} */

#endif //__RAMTOFLA_H__


