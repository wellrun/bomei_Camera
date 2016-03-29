/**
 * @filename hal_usb_host_h_std.c
 * @brief: standard protocol of usb host.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-23
 * @version 1.0
 * @ref
 */

#ifdef OS_ANYKA

#include "hal_usb_h_std.h"
#include "usb_bus_drv.h"
#include "usb_host_drv.h"

/**
 * @brief  fill urb struct for standard request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pURB [out]: pointer to urb struct
 * @param pDevReq [in]: pointer to setup packet data
 * @param data [in]: address of data to be transfer
 * @param data_len [in]: data length
 * @return  T_VOID
 */
static T_VOID fill_urb(T_URB *pURB, T_UsbDevReq *pDevReq, T_U8 *data, T_U32 data_len)
{
    memset(pURB, 0, sizeof(T_URB));
    memcpy(&pURB->dev_req, pDevReq, sizeof(T_UsbDevReq));

    pURB->trans_type = TRANS_CTRL;

    if(pDevReq->bmRequestType | USB_STD_DIR_DEV2HOST)
    {
        pURB->trans_dir = TRANS_DATA_IN;
    }
    else
    {
        pURB->trans_dir = TRANS_DATA_OUT;
    }
    
    pURB->data = data;
    pURB->buffer_len = data_len;
    pURB->data_len = data_len;

    pURB->timeout = URB_MAX_WAIT_TIME;
}

//********************************************************************
/**
 * @brief  get status request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_get_status(T_VOID)
{
    return AK_TRUE;
}

//********************************************************************
/**
 * @brief  clear feature request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param EP_Index [in]: which ep need to clear feature
 * @return  T_BOOL
 */
T_BOOL usb_host_std_clear_feature(T_U8 EP_Index)
{
    T_URB urb;
    T_URB_HANDLE hURB = AK_NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_ENDPOINT;
    dev_req.bRequest = USB_STD_CLEARFEATURE;
    dev_req.wValue = 0;
    dev_req.wIndex = EP_Index;
    dev_req.wLength = 0;

    //fill urb struct
    fill_urb(&urb, &dev_req, AK_NULL, 0);

    //commit urb
    hURB = usb_bus_commit_urb(&urb);
    if(AK_NULL == hURB)
    {
        return AK_FALSE;
    }

    //waiting for urb completion
    if(usb_bus_wait_completion(hURB) < 0)
    {
        return AK_FALSE;
    }
    if (EP_Index & 0x80)
    {
        usb_host_clear_data_toggle(EP1_INDEX);
    }
    else
    {
        usb_host_clear_data_toggle(EP2_INDEX);
    }
    return AK_TRUE;
}

//********************************************************************
/**
 * @brief  set feature request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_set_feature(T_VOID)
{
    return AK_TRUE;
}

//********************************************************************
/**
 * @brief  set  address request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param dev_addr [in]: address to be set for device
 * @return  T_BOOL
 */
T_BOOL usb_host_std_set_address(T_U32 dev_addr)
{
    T_URB urb;
    T_URB_HANDLE hURB = AK_NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest = USB_STD_SETADDRESS;
    dev_req.wValue = dev_addr;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;

    fill_urb(&urb, &dev_req, AK_NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(AK_NULL == hURB)
    {
        return AK_FALSE;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return AK_FALSE;
    }
    
    usb_host_set_address(dev_addr);
    
    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  get descriptor request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param desc_type [in]: decriptor type
 * @param desc_index [in]: decriptor index, only validate for string descriptor
 * @param lang_id [in]: lang id, only validate for string descriptor
 * @param data [out]: buffer for descriptor data
 * @param data [out]: length of buffer
 * @return  T_S32 size of the data received
 */
T_S32 usb_host_std_get_descriptor(T_U8 desc_type, T_U8 desc_index, T_U32 lang_id, T_U8 data[], T_U32 len)
{
    T_URB urb;
    T_URB_HANDLE hURB = AK_NULL;
    T_UsbDevReq dev_req;
    T_S32 ret;
    
    dev_req.bmRequestType = USB_STD_DIR_DEV2HOST | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest = USB_STD_GETDESCRIPTOR;
    dev_req.wValue = (desc_type << 8) | (desc_index);
    dev_req.wIndex = 0;
    dev_req.wLength = len;

    fill_urb(&urb, &dev_req, data, len);
    
    hURB = usb_bus_commit_urb(&urb);
    if(AK_NULL == hURB)
    {
        return URB_ERROR;
    }

    ret = usb_bus_wait_completion(hURB);
    return ret;
}
//********************************************************************
/**
 * @brief  set  descriptor request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_set_descriptor(T_VOID)
{
    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  get  configuration request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_get_configuration(T_VOID)
{
    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  set  configuration request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param conf_val [in]: config value
 * @return  T_BOOL
 */
T_BOOL usb_host_std_set_configuration(T_U8 conf_val)
{
    T_URB urb;
    T_URB_HANDLE hURB = AK_NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest =  USB_STD_SETCONFIG;
    dev_req.wValue = conf_val;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;
    
    fill_urb(&urb, &dev_req, AK_NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(AK_NULL == hURB)
    {
        return AK_FALSE;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  get  interface request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_get_interface(T_VOID)
{
    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  set interface request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_set_interface(T_VOID)
{
    T_URB urb;
    T_URB_HANDLE hURB = AK_NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_INTERFACE;
    dev_req.bRequest =  USB_STD_SETINTERFACE;
    dev_req.wValue = 0;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;
    
    fill_urb(&urb, &dev_req, AK_NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(AK_NULL == hURB)
    {
        return AK_FALSE;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}
//********************************************************************
/**
 * @brief  sych frame request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_BOOL
 */
T_BOOL usb_host_std_sych_frame(T_VOID)
{
    return AK_TRUE;
}
//********************************************************************
#endif
