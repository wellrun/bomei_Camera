/**
 * @filename usb_slave_disk.c
 * @brief: AK3223M how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "fwl_usb_transc.h"
#include "transc.h"

static volatile T_U8 s_AnykaCmd = 0;
static volatile T_U32 s_AnykaCmdParams[2];

//********************************************************************
static T_ANYKA_TRANSC *m_pTrans = AK_NULL;
static T_U32 m_cntTrans = 0;

static volatile T_U32 m_cntBuffs = 0;


void Fwl_Usb_Set_Trans(T_ANYKA_TRANSC *pTrans, T_U32 count)
{
    if(AK_NULL == pTrans)
    {
        count = 0;
    }

    m_pTrans = pTrans;
    m_cntTrans = count;

    {
        T_U32 i;
        for(i = 0; i < count; i++)
        {
            printf("[%d]", pTrans[i].cmd);
        }
    }
}


T_BOOL ausb_enable(T_U32 mode)
{
    usb_slave_device_enable(USB_MODE_20);

    return AK_TRUE;
}
//********************************************************************
T_VOID ausb_disable(T_VOID)
{
    usb_slave_set_state(USB_NOTUSE);
    
    usb_slave_device_disable();
}

T_ANYKA_TRANSC* ausb_get_transc(T_U8 cmd)
{
    T_U32 i;
    
    //check for command
    for(i = 0; i < m_cntTrans; i++)
    {
       // printf("<%d, %d>", m_pTrans[i].cmd, pCmd->cmd);
        if(m_pTrans[i].cmd == cmd)
        {
            return (m_pTrans + i);
        }
    }

    return AK_NULL;
}

T_BOOL ausb_recv(T_U32 buf, T_U32 count)
{
    T_ANYKA_TRANSC *pTrans = AK_NULL;

    pTrans = ausb_get_transc(s_AnykaCmd);
    if(AK_NULL == pTrans)
    {
        return AK_FALSE;
    }
	
	if(AK_NULL == pTrans->fRcv)
    {
        return AK_TRUE;
    }

    if(!pTrans->fRcv(buf, count))
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}

T_BOOL ausb_send(T_U32 buf, T_U32 count)
{
    T_ANYKA_TRANSC *pTrans = AK_NULL;

    pTrans = ausb_get_transc(s_AnykaCmd);
    if(AK_NULL == pTrans)
    {
        printf("error get transc: %d\r\n", s_AnykaCmd);
        return AK_FALSE;
    }
	
	if(AK_NULL == pTrans->fSnd)
    {
        return AK_TRUE;
    }
    
    if(!pTrans->fSnd(buf, count))
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}

T_BOOL HandleCmd(T_U8* scsi_data, T_U32 data_len)
{
    T_CMD_RESULT Rslt = {0};
    T_ANYKA_TRANSC *pTrans = AK_NULL;
    T_U32 i;

   if(AK_NULL == scsi_data)
   {
        if(TRANS_SWITCH_USB == s_AnykaCmd || TRANS_RESET == s_AnykaCmd)
        {
            printf("#");
            pTrans = ausb_get_transc(s_AnykaCmd);
            if(AK_NULL == pTrans)
            {
                return AK_FALSE;
            }

            Rslt.data_count = data_len;
            if(!pTrans->fCmd(s_AnykaCmdParams, sizeof(s_AnykaCmdParams), &Rslt))
            {
                //exit loop if need
                return AK_FALSE;
            }
            return AK_TRUE;
        }
        return AK_FALSE;
   }
   s_AnykaCmd =  *(scsi_data+1);

   if(s_AnykaCmd > 128)
   {
        s_AnykaCmd -= 128;
   }
   else
   {
        s_AnykaCmd = 0;
        return AK_TRUE;
   }
   s_AnykaCmdParams[0] = (( *(scsi_data + 5) ) 
                        | ( *(scsi_data + 6) << 8 ) 
                        | ( *(scsi_data + 7) << 16 ) 
                        | ( *(scsi_data + 8) << 24 ));

   s_AnykaCmdParams[1] = (( *(scsi_data + 9) ) 
                        | ( *(scsi_data + 10) << 8 ) 
                        | ( *(scsi_data + 11) << 16 ) 
                        | ( *(scsi_data + 12) << 24 ));


    pTrans = ausb_get_transc(s_AnykaCmd);
    if(AK_NULL == pTrans)
    {
        return AK_FALSE;
    }

    Rslt.data_count = data_len;

    if (TRANS_SWITCH_USB != s_AnykaCmd && TRANS_RESET != s_AnykaCmd)
    {
        //parse command
        if(!pTrans->fCmd(s_AnykaCmdParams, sizeof(s_AnykaCmdParams), &Rslt))
        {
            //exit loop if need
            return AK_FALSE;
        }
    }

    return AK_TRUE;
    
}

void Fwl_Usb_Main()
{
    T_U32 mode = 0;
    
    mode = usb_slave_get_mode();
    printf("usb_slave_get_mode:%d \r\n", mode);
    
    usbdisk_mboot_init(mode);
    usbdisk_mboot_set_cb(HandleCmd,ausb_send,ausb_recv);

    usbdisk_init(mode);
    if(!usbdisk_start())
    {
        printf("usbdisk start fail\r\n");
        while(1);
    }
    
    while(1)
    {
        usbdisk_proc();
    }
}


