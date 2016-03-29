/**
 * @filename usb_udisk_mass.c
 * @brief:how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-26
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include "usb_slave_drv.h"
#include "hal_usb_s_disk.h"
#include "hal_usb_s_std.h"
#include "hal_udisk_mass.h"
#include "usb_common.h"
#include "interrupt.h"
#include "akos_api.h"
#include "drv_api.h"
#include "drv_module.h"



static volatile T_MSC_TRANS s_MscTrans={0};
static volatile T_U16 InquireCnt = 0;
static T_VOID scsi_unsupported(T_U8* scsi_data);
static T_VOID scsi_inquiry(T_VOID);
static T_VOID scsi_test_unit_ready(T_VOID);
static T_VOID scsi_read_format_capacity(T_VOID);
static T_VOID scsi_read_capacity(T_VOID);
static T_VOID scsi_read10(T_U8* scsi_data);
static T_VOID scsi_read12(T_U8* scsi_data);
static T_VOID scsi_write10(T_U8* scsi_data);
static T_VOID scsi_write12(T_U8* scsi_data);
static T_VOID scsi_verify();
static T_VOID scsi_start_stop(T_U8* scsi_data);
static T_VOID scsi_mode_sense6();
static T_VOID scsi_req_sense();
static T_VOID scsi_prevent_remove();
static T_VOID msc_scsi_handle(T_U8* scsi_data);
static T_VOID scsi_anyka_mass_boot(T_U8* scsi_data);
static T_VOID msc_mboot_handle(T_U8* scsi_data);



T_U16 msc_get_inquire_cnt(T_VOID)
{
    return InquireCnt;
}
T_VOID msc_reset_inquire_cnt(T_VOID)
{
    InquireCnt = 0;
}
/**
 * @brief parse cbw
 *
 * called when cbw is received
 * @author Huang Xin
 * @date 2010-08-04
 * @param buf[in] buf which is saving cbw
 * @param buflen[in] the cbw len
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL msc_parse_cbw(T_U8 *buf, T_U32 buflen)
{
    if (CBW_PKT_SIZE == buflen )
    {    
        memcpy((T_U8 *)(&s_MscTrans.tCbw), buf, CBW_PKT_SIZE);   
        //cbw invalid or no mean
        if (CBW_SIGNATURE != s_MscTrans.tCbw.dCBWSignature)
        {
            usb_slave_std_hard_stall(AK_TRUE);
            usb_slave_ep_stall(EP2_INDEX);
            usb_slave_ep_stall(EP1_INDEX);
            usb_slave_ep_stall(EP3_INDEX);
            return AK_FALSE;
        }
        else if(s_MscTrans.tCbw.dCBWDataTransferLength != 0)
        {
            if (0 == (s_MscTrans.tCbw.bmCBWFlags&0x80))
                s_MscTrans.enmStage = MSC_STAGE_DATA_OUT; 
            else
                s_MscTrans.enmStage = MSC_STAGE_DATA_IN;
        }     
        else
        {
            s_MscTrans.enmStage = MSC_STAGE_STATUS;
        }
        if(udisk_lun_num() > 0)
        {
            msc_scsi_handle((T_U8*)s_MscTrans.tCbw.pCBWCB);
        }
        else
        {
            msc_mboot_handle((T_U8*)s_MscTrans.tCbw.pCBWCB);
        }
        return AK_TRUE;
    }    
    //CBW length is not 31 bytes,cbw invalid
    else
    {
        usb_slave_std_hard_stall(AK_TRUE);
        usb_slave_ep_stall(EP2_INDEX);
        usb_slave_ep_stall(EP1_INDEX);
        usb_slave_ep_stall(EP3_INDEX);
        return AK_FALSE;
    }

    return AK_FALSE;
    
}

/**
 * @brief send csw
 *
 * called in status stage  
 * @author Huang Xin
 * @date 2010-08-04
 * @param status[in] pass,failed or phase error
 * @param residue[in] difference between the amount of data expected and the actual sent by the device.
 * @return  T_VOID
 */
T_VOID msc_send_csw(T_U8 status,T_U32 residue)
{
    s_MscTrans.tCsw.dCSWSignature = CSW_SIGNATURE;
    s_MscTrans.tCsw.dCSWTag = s_MscTrans.tCbw.dCBWTag;
    s_MscTrans.tCsw.dCSWDataResidue = residue;
    s_MscTrans.tCsw.bCSWStatus = status; 
    
    usb_slave_start_send(EP2_INDEX);
    usb_slave_data_in(EP2_INDEX, (T_U8 *)&s_MscTrans.tCsw, CSW_PKT_SIZE);
}

/**
 * @brief get current stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_U8
 * @retval the enum value of stage
 */
T_U8 msc_get_stage()
{
    return s_MscTrans.enmStage;
}

/**
 * @brief set stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param stage[in] the current stage to set
 * @return  T_VOID
 */
T_VOID msc_set_stage(T_U8 stage)
{    
    s_MscTrans.enmStage = stage;
}

/**
 * @brief scsi cmd parse handle
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID msc_scsi_handle(T_U8* scsi_data)
{    
    //akprintf(C3, M_DRVSYS, "<%x>",*scsi_data);
    switch (*scsi_data)
    {       
        case SCSI_INQUIRY:
            scsi_inquiry();
            break;
        case SCSI_TEST_UNIT_READY:
            scsi_test_unit_ready();
            InquireCnt++;
            break;
        case SCSI_READ_FORMAT_CAPACITY:
            scsi_read_format_capacity();
            break;
        case SCSI_READ_CAPACITY:
            scsi_read_capacity();
            break;     
        case SCSI_READ_10:
            scsi_read10(scsi_data);
            break;
        case SCSI_WRITE_10:
            scsi_write10(scsi_data);
            break;
        case SCSI_MODESENSE_6:
        case SCSI_MODE_SENSE:
            scsi_mode_sense6();
            break;   
        case SCSI_VERIFY:
            scsi_verify(scsi_data);
            break;
        case SCSI_START_STOP:
            scsi_start_stop(scsi_data);
            break;
        case SCSI_REQUEST_SENSE:
            scsi_req_sense();
            break;    
        case SCSI_READ_12:
            scsi_read12(scsi_data);
            break;  
        case SCSI_WRITE_12:
            scsi_write12(scsi_data);
            break;
        case SCSI_PREVENT_REMOVE:
            scsi_prevent_remove();
            break;
        default:
            scsi_unsupported(scsi_data);
            break;
    }
}
static T_VOID msc_mboot_handle(T_U8* scsi_data)
{    
    //akprintf(C3, M_DRVSYS, "<0x%x>",*scsi_data);
    switch (*scsi_data)
    {   
        case SCSI_INQUIRY:
            scsi_inquiry();
            break;
        case SCSI_TEST_UNIT_READY:
            scsi_test_unit_ready();
            break;
        case SCSI_REQUEST_SENSE:
            scsi_req_sense();
            break;    
        case SCSI_ANYKA_MASS_BOOT:
            scsi_anyka_mass_boot(scsi_data);
            break;
        default:
            scsi_unsupported(scsi_data);
            break;
    }
}


/**
 * @brief process unsupported cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_unsupported(T_U8* scsi_data)
{    

    udisk_unsupported(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength,scsi_data);

}

/**
 * @brief process inquiry cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_inquiry(T_VOID)
{
    udisk_inquiry(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process test unit ready cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_test_unit_ready(T_VOID)
{
    udisk_test_unit_ready(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process read format capacity cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_read_format_capacity(T_VOID)
{
    udisk_read_format_capacity(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process read capacity cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_read_capacity(T_VOID)
{ 
    udisk_read_capacity(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);   
}

/**
 * @brief process read10 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_read10(T_U8* scsi_data)
{
    T_U32 log_blk_addr;
    T_U32 log_blk_len;
    
    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+7)<<8)+(*(scsi_data+8)); 
    udisk_read(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);

}

/**
 * @brief process read12 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_read12(T_U8* scsi_data)
{
    T_U32 log_blk_addr;
    T_U32 log_blk_len;

    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+6)<<24)+(*(scsi_data+7)<<16)+(*(scsi_data+8)<<8)+(*(scsi_data+9));
    udisk_read(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process write10 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_write10(T_U8* scsi_data)
{
    T_U32 log_blk_addr;
    T_U32 log_blk_len;

    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+7)<<8)+(*(scsi_data+8)); 
    udisk_write(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);

}

/**
 * @brief process write12 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_write12(T_U8* scsi_data)
{
    T_U32 log_blk_addr;
    T_U32 log_blk_len;
    
    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+6)<<24)+(*(scsi_data+7)<<16)+(*(scsi_data+8)<<8)+(*(scsi_data+9));    
    udisk_write(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process verify cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_verify()
{
    udisk_verify(s_MscTrans.tCbw.bCBWLUN);
}

/**
 * @brief process start stop cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  T_VOID
 */
static T_VOID scsi_start_stop(T_U8* scsi_data)
{   
    T_U32 start_stop;
    
    start_stop = (scsi_data[4] & 0x03);
    udisk_start_stop(s_MscTrans.tCbw.bCBWLUN,start_stop);
}

/**
 * @brief process mode sense6 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_mode_sense6()
{
    udisk_mode_sense6(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process request sense cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID scsi_req_sense()
{
    udisk_req_sense(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}


static T_VOID scsi_prevent_remove()
{
   udisk_prevent_remove(s_MscTrans.tCbw.bCBWLUN);
}


static T_VOID scsi_anyka_mass_boot(T_U8* scsi_data)
{
   udisk_anyka_mass_boot(scsi_data, s_MscTrans.tCbw.dCBWDataTransferLength);
}


