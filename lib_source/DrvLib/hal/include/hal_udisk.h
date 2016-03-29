/**@file hal_udisk.h
 * @brief Implement  operations of how to use usb disk.
 *
 * This file describe msc protocol  and ufi cmd process of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-24
 * @version 1.0
 */

#ifndef __HAL_UDISK_H__
#define __HAL_UDISK_H__

#include "anyka_types.h"
#include "hal_usb_mass.h"
#include "hal_usb_s_disk.h"
#include "hal_udisk_mass.h"

#ifdef __cplusplus
extern "C" {
#endif


#define UDISK_DESC_TOTAL_LEN                    0x0020
#define UDISK_MAXIMUM_LUN                       15
#define UDISK_BUFFER_NUM                        2
#define UDISK_BUFFER_LEN                        (64*1024)
#define UDISK_READ_BUF_LEN                      (64*1024)
#define UDISK_WRITE_BUF_LEN                     (64*1024)
#define UDISK_FORMAT_CAPACITY_LEN                12
#define UDISK_CAPACITY_LEN                       8
#define UDISK_SENSE_LEN                          18
#define UDISK_MODE_PARAMETER_LEN                 4
#define UDISK_INQ_STR_LEN                        36

#define EVENT_USB_RX_FINISH(buf_id)             (1<<(buf_id))
#define EVENT_USB_TX_FINISH(buf_id)             (1<<(buf_id+UDISK_BUFFER_NUM))
#define EVENT_STATUS_STAGE                      (1<<UDISK_BUFFER_NUM*2)


#define USB_MASSCLASS_COMMAND_SCSI              0x06
#define USB_MASSCLASS_TRAN_BO                   0x50

typedef enum
{
    READ_COMMAND = 0,
    WRITE_COMMAND,
    SUSPEND_COMMAND
}T_eTASK_COMMAND;

typedef enum
{
    TRANS_IDLE,
    TRANS_TX,
    TRANS_RX,
    TRANS_SUSPEND_TX,
    TRANS_SUSPEND_RX
}T_eTRANS_STATE;

typedef enum
{
    UDISK_STOP_MEDIA,
    UDISK_START_MEDIA,
    UDISK_EJECT_MEDIA
}T_eUDISK_START_STOP;

typedef struct _SCSI_MSG
{
    T_U8            ucCmd;
    T_U8            ucLun;
    T_U32           ulParam1;
    T_U32           ulParam2;  
}T_SCSI_MSG,*T_pSCSI_MSG;

typedef struct _BUFFER_TRANS
{
    T_BOOL bValidate;
    T_U8 *pBuffer;
}T_BUFFER_TRANS, *T_pBUFFER_TRANS;

typedef struct _UDISK_TRANS
{
    T_U32               ulTransBytes;
    T_U8                ucReadIndex;
    T_U8                ucWriteIndex;
    T_U8                ucTransIndex; 
    T_eTRANS_STATE      enmTransState;
    T_BUFFER_TRANS      tBuffer[UDISK_BUFFER_NUM];
}T_UDISK_TRANS,*T_pUDISK_TRANS;

typedef struct _UDISK_DEV
{
    T_U32               ulMode;
    T_U8                ucLunNum;
    T_LUN_INFO          tLunInfo[UDISK_MAXIMUM_LUN];
    T_UDISK_TRANS       tTrans;
    T_U8                Sense[UDISK_SENSE_LEN];
    T_U32 pre_read_sector_num;
    T_U32 pre_read_start_sector;
    T_U8 pre_read_lun;
}T_UDISK_DEV ,*T_pUDISK_DEV;

typedef struct _MBOOT
{
    T_fMBOOT_HANDLE_CMD fMbootCmdCb;
    T_fMBOOT_HANDLE_SEND fMbootSendCb;
    T_fMBOOT_HANDLE_RCV fMbootRcvCb;
}T_MBOOT,*T_pMBOOT;


/**
 * @brief   scsi cmd 'inquiry' process
 *
 * send SCSI_INQUIRY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_inquiry(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'test unit ready' process
 *
 * send SCSI_TEST_UNIT_READY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_test_unit_ready(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'read format capacity' process
 *
 * send SCSI_READ_FORMAT_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_read_format_capacity(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'read capacity' process
 *
 * send SCSI_READ_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_read_capacity(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'read10' or 'read12' process
 *
 * send SCSI_READ_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_read(T_U8 lun,T_U32 start_sector,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'write10' or 'write12' process
 *
 * send SCSI_WRITE_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_write(T_U8 lun,T_U32 start_sector,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'mode sense' or 'mode sense6' process
 *
 * send SCSI_MODESENSE_6 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_mode_sense6(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'verify' process
 *
 * send SCSI_VERIFY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_verify(T_U8 lun);
/**
 * @brief   scsi cmd 'start stop' process
 *
 * send SCSI_START_STOP msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_stop[in] start ,stop or eject disk expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_start_stop(T_U8 lun,T_U32 start_stop);
/**
 * @brief   scsi cmd 'request sense' process
 *
 * send SCSI_REQUEST_SENSE msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_req_sense(T_U8 lun,T_U32 expected_bytes);
/**
 * @brief   scsi cmd 'prevent remove' process
 *
 * send 'prevent remove' msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_prevent_remove(T_U8 lun);

/**
 * @brief   scsi cmd unsupported process
 *
 * send SCSI_UNSUPPORTED msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_unsupported(T_U8 lun,T_U32 expected_bytes,T_U8 *scsi);

/**
 * @brief   set sense that contain error information
 *
 * called when scsi cmd parse or process is finish
 * @author Huang Xin
 * @date 2010-08-04
 * @param sense[in] error information
 * @return  T_VOID
 */
T_VOID udisk_set_sense(T_U32 sense);

/**
 * @brief   get udisk lun num
 *
 * called when scsi cmd parse
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_U8
 */
T_U8 udisk_lun_num(T_VOID);

/**
 * @brief   anyka mass boot cmd  process
 *
 * send mass boot msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] scsi struct
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL udisk_anyka_mass_boot(T_U8* scsi_data,T_U32 expected_bytes);



#ifdef __cplusplus
}
#endif

#endif 


