/**
 * @file hal_usb_std.c
 * @brief: standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-09-01
 * @version 1.0
 */

#ifdef OS_ANYKA
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "usb_slave_drv.h"
#include "hal_usb_s_std.h"
#include "usb_common.h"
#include "hal_usb_s_state.h"
#include "drv_api.h" 

static T_BOOL usb_slave_std_reserve(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_get_status(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_clear_feature(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_set_feature(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_set_address(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_get_descriptor(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_set_descriptor(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_get_configuration(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_set_configuration(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_get_interface(T_CONTROL_TRANS *pTrans);
static T_BOOL usb_slave_std_set_interface(T_CONTROL_TRANS *pTrans);
  
static T_BOOL usb_slave_std_callback(T_CONTROL_TRANS *pTrans);


//global usb slave stuct
T_USB_SLAVE_STANDARD Usb_Slave_Standard;

//device address
volatile T_U8 m_device_addess = 0;
volatile T_U8 m_config_value = 0;

T_BOOL m_hard_stall = AK_FALSE;

T_fUSB_CONTROL_CALLBACK m_usb_std_req[] = 
{
    //...
    usb_slave_std_get_status,
    usb_slave_std_clear_feature,
    usb_slave_std_reserve,
    usb_slave_std_set_feature,
    usb_slave_std_reserve,
    usb_slave_std_set_address,
    usb_slave_std_get_descriptor,
    usb_slave_std_set_descriptor,
    usb_slave_std_get_configuration,
    usb_slave_std_set_configuration,
    usb_slave_std_get_interface,
    usb_slave_std_set_interface,
    usb_slave_std_reserve,
    usb_slave_std_reserve,
    usb_slave_std_reserve
};


//********************************************************************
/**
 * @brief  init  usb standard request module
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  T_VOID
 */
T_VOID usb_slave_std_init()
{
    usb_slave_set_ctrl_callback(REQUEST_STANDARD, usb_slave_std_callback);
}

/**
 * @brief   change the clear stall condition in clear feature
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param bSet [in]: clear stall in clear feature or not 
 * @return  T_VOID
 */
T_VOID usb_slave_std_hard_stall(T_BOOL bSet)
{
    m_hard_stall = bSet;
}

/**
 * @brief  callback function of usb standard request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_callback(T_CONTROL_TRANS *pTrans)
{
    T_U8 req_type;

    req_type = (pTrans->dev_req.bmRequestType >> 5) & 0x3;
    if(req_type != REQUEST_STANDARD)
        return AK_FALSE;

    return m_usb_std_req[pTrans->dev_req.bRequest & 0x0F](pTrans);
    
}

T_BOOL usb_slave_std_reserve(T_CONTROL_TRANS *pTrans)
{
    return AK_FALSE;
}

/**
 * @brief  get status request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_get_status(T_CONTROL_TRANS *pTrans)
{
    T_U8 recipient;
    T_U32 ep_num = 0;
    T_U16 status;
    
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
        return AK_TRUE;

    //wValue should be zero, wLength should be 2
    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wLength != 2)
        return AK_FALSE;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;

    memset(pTrans->buffer, 0, pTrans->buf_len);
    pTrans->data_len = pTrans->dev_req.wLength;

    switch(recipient)
    {
        case RECIPIENT_DEVICE:
            if(pTrans->dev_req.wIndex != 0)
            {
                return AK_FALSE;
            }
            break;
            
        case RECIPIENT_INTERFACE:
            break;

        case RECIPIENT_ENDPOINT:
            ep_num = pTrans->dev_req.wIndex & 0x7F;
            status = usb_slave_get_ep_status(ep_num);
            memcpy(pTrans->buffer, &status, 2);
            break;
    }

    return AK_TRUE;
}

/**
 * @brief  clear feature request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_clear_feature(T_CONTROL_TRANS *pTrans)
{
    T_U8 recipient;
    T_U32 feature_selector, ep_num;

    //only handle in status stage
    if(pTrans->stage != CTRL_STAGE_STATUS)
        return AK_TRUE;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;
    feature_selector = pTrans->dev_req.wValue;

    if(feature_selector != ENDPOINT_HALT)
    {
        return AK_FALSE;
    }
    else if(recipient != RECIPIENT_ENDPOINT)
    {
        return AK_FALSE;
    }
    
    ep_num = pTrans->dev_req.wIndex & 0x7F;
    if(!m_hard_stall)
    {
        usb_slave_ep_clr_stall(ep_num);
    }
    return AK_TRUE;
}

/**
 * @brief  set feature request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_set_feature(T_CONTROL_TRANS *pTrans)
{
    T_U8 recipient;
    T_U32 feature_selector, ep_num;
    T_U32 test_mode;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;
    feature_selector = pTrans->dev_req.wValue;

    if(pTrans->stage == CTRL_STAGE_SETUP)  //setup stage
    {
        if(RECIPIENT_DEVICE == recipient)
        {
            if(feature_selector != TEST_MODE)
            {
                return AK_FALSE;
            }
            else if((pTrans->dev_req.wIndex & 0xFF) != 0)
            {
                return AK_FALSE;
            }
        }
        else if(RECIPIENT_ENDPOINT == recipient)
        {
            ep_num = pTrans->dev_req.wIndex & 0x7F;
            usb_slave_ep_stall(ep_num);
        }
    }

    if(pTrans->stage == CTRL_STAGE_STATUS)  //status stage
    {
        if((RECIPIENT_DEVICE == recipient) && (feature_selector == TEST_MODE))
        {        
            test_mode = pTrans->dev_req.wIndex >> 8;
            usb_slave_enter_testmode(test_mode);
        }
    }

    return AK_TRUE;
}

/**
 * @brief  set address request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_set_address(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage == CTRL_STAGE_STATUS)
    {
        m_device_addess = pTrans->dev_req.wValue;
        usb_slave_set_address((T_U8)pTrans->dev_req.wValue);
    }

    return AK_TRUE;
}

/**
 * @brief  get descriptor request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_get_descriptor(T_CONTROL_TRANS *pTrans)
{
    T_U8 des_type, des_index;
    T_U32 cnt;
    T_U8 *data;
    
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }
    
    des_index = pTrans->dev_req.wValue & 0xFF;
    des_type = pTrans->dev_req.wValue >> 8;

    pTrans->data_len = pTrans->dev_req.wLength;

    switch( des_type )
    {
        case DEVICE_DESC_TYPE:           //01
            if(Usb_Slave_Standard.usb_get_device_descriptor != AK_NULL)
            {
                data = Usb_Slave_Standard.usb_get_device_descriptor(&cnt);
            }
            else
            {
                return AK_FALSE;
            }
            break;

        case CONFIG_DESC_TYPE:          //02
            if(Usb_Slave_Standard.usb_get_config_descriptor != AK_NULL)
            {
                data = Usb_Slave_Standard.usb_get_config_descriptor(&cnt);
            }
            else
            {
                return AK_FALSE;
            }
            break;
            
        case STRING_DESC_TYPE:         //03
            if(Usb_Slave_Standard.usb_get_string_descriptor != AK_NULL)
            {
                data = Usb_Slave_Standard.usb_get_string_descriptor(des_index, &cnt);
            }
            else
            {
                return AK_FALSE;
            }
            break;

        case DEVICE_QUALIFIER_DESC_TYPE: //06
            if(Usb_Slave_Standard.usb_get_device_qualifier_descriptor != AK_NULL)
            {
                data = Usb_Slave_Standard.usb_get_device_qualifier_descriptor(&cnt);
            }
            else
            {
                return AK_FALSE;
            }
            break;
            
        case OTHER_SPEED_CONFIGURATION_DESC_TYPE: //07
            if(Usb_Slave_Standard.usb_get_other_speed_config_descriptor != AK_NULL)
            {
                data = Usb_Slave_Standard.usb_get_other_speed_config_descriptor(&cnt);
            }
            else
            {
                return AK_FALSE;
            }
            break;

        default:
            return AK_FALSE;

    }

    if(cnt < pTrans->data_len)
    {
        pTrans->data_len = cnt;
    }
    
    memcpy(pTrans->buffer, data, pTrans->data_len);

    return AK_TRUE;
}

/**
 * @brief  set descriptor request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_set_descriptor(T_CONTROL_TRANS *pTrans)
{
    return AK_FALSE;
}


/**
 * @brief  get configuration request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_get_configuration(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }

    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wIndex != 0 || pTrans->dev_req.wLength != 1)
    {
        return AK_FALSE;
    }

    pTrans->data_len = pTrans->dev_req.wLength;
    pTrans->buffer[0] = m_config_value;

    return AK_TRUE;
}

/**
 * @brief  set configuration request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_set_configuration(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }

    if(pTrans->dev_req.wIndex != 0 || pTrans->dev_req.wLength != 0)
    {
        return AK_FALSE;
    }

    m_config_value = pTrans->dev_req.wValue;

    usb_slave_set_state(USB_OK);

    usb_slave_clr_toggle();

    return AK_TRUE;
}

/**
 * @brief  get interface request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_get_interface(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }

    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wLength != 1)
    {
        return AK_FALSE;
    }
    
    pTrans->data_len = pTrans->dev_req.wLength;
    pTrans->buffer[0] = 0;

    return AK_TRUE;
}

/**
 * @brief  set interface request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  T_BOOL
 */
T_BOOL usb_slave_std_set_interface(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }

    usb_slave_clr_toggle();

    return AK_TRUE;
}

 
#endif
