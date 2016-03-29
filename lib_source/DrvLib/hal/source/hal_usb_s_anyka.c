/**
 * @filename usb_anyka.c
 * @brief how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-26
 */

#ifdef OS_ANYKA
#include    "anyka_cpu.h"
#include    "anyka_types.h"
#include    "usb_slave_drv.h"
#include    "hal_usb_s_anyka.h"
#include    "hal_usb_s_std.h"
#include    "usb_common.h"
#include    "interrupt.h"
#include    "drv_api.h"
#include    "drv_module.h"

//define message for usb anyka
#define MESSAGE_RX_NOTIFY 1
#define MESSAGE_RX_FINISH 2
#define MESSAGE_TX_FINISH 3

//********************************************************************
static T_VOID Fwl_Usb_Anyka_Send_Finish(T_VOID);
static T_VOID Fwl_Usb_Anyka_Notify(T_VOID);
static T_VOID Fwl_Usb_Anyka_Receive_Finish(T_VOID);
static T_VOID Fwl_Usb_Anyka_EnumOK(T_VOID);

static T_VOID Usb_Anyka_USB_Reset(T_U32 mode);

static T_U8 *usbanyka_getdevicedescriptor(T_U32 *count);
static T_U8 *usbanyka_getconfigdescriptor(T_U32 *count);
static T_U8 *usbanyka_getstringdescriptor(T_U8 index, T_U32 *count);

static T_VOID usbanyka_rx_notify(T_U32 *param, T_U32 len);
static T_VOID usbanyka_rx_finish(T_U32 *param, T_U32 len);
static T_VOID usbanyka_tx_finish(T_U32 *param, T_U32 len);

//********************************************************************
static const T_USB_DEVICE_DESCRIPTOR device_desc =
{
    18,                         // length 18 bytes
    DEVICE_DESC_TYPE,           // device descriptor type
    0x0110,                     // USB Specification Release Number in
    0xff,                       // Class code (assigned by the USB) 0x00?.
    0xff,                       // Subclass code (assigned by the USB)0x00?.
    0xff,                       // Protocol code (assigned by the USB)0x00?.
    EP0_MAX_PAK_SIZE,           // Maximum packet size for endpoint zero
    0x0471,                     // Vendor ID (assigned by the USB)//0x0471?
    0x0666,                     // Product ID (assigned by the manufacturer)
    0x0100,                     // Device release number in binary-coded
    0x00,                       // Index of string descriptor describing manufacturer
    0x00,                       // Index of string descriptor describing produc
    0x00,                       // Index of string descriptor describing the device's serial number
    0x01
};

static const T_USB_CONFIGURATION_DESCRIPTOR config_desc =
{
    9,                          //Size of this descriptor in bytes
    CONFIG_DESC_TYPE,           //CONFIGURATION Descriptor Type//02
    DEF_DESC_LEN,               //Total length of data returned for this
    0x01,                       //Number of interfaces supported by this configuration
    0x01,                       //Value to use as an argument to the
    0x00,                       //Index of string descriptor describing this configuration
    0xC0,
    0x01                        //Maximum power consumption of the USB
};

static const T_USB_INTERFACE_DESCRIPTOR if_desc =
{
    9,                          //Size of this descriptor in bytes
    IF_DESC_TYPE,               //INTERFACE Descriptor Type//04
    0x00,                       //Number of interface. Zero-based value
    0x00,                       //Value used to select alternate setting for
    0x03,                       //Number of endpoints used by this
    0xff,                       //Class code (assigned by the USB).
    0xff,                       //Subclass code (assigned by the USB).
    0x00,                       //Protocol code (assigned by the USB).
    0x00                        //Index of string descriptor describing this interface
};

static const T_USB_ENDPOINT_DESCRIPTOR ep1_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type//0x05
    0x81,                      //10000001
    0x03,                      //00000011
    EP1_BUF_MAX_LEN,           //0x08 0x00 Maximum packet size this endpoint
    0x0A                       //0xFFInterval for polling endpoint for data
};

static T_USB_ENDPOINT_DESCRIPTOR ep2_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type
    0x82,
    0x02,
    EP2_BUF_MAX_LEN,           //Maximum packet size this endpoint
    0x00                       //Interval for polling endpoint for data
};

static T_USB_ENDPOINT_DESCRIPTOR ep3_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type
    0x03,
    0x02,
    EP3_BUF_MAX_LEN,           //0x40 00 Maximum packet size this endpoint
    0x00                       //0x00 Interval for polling endpoint for data
};

//string buffer 
static const T_U8 sAnyka_String[] = {0};

typedef struct {
    T_fUSBANYKA_RECEIVECALLBACK receive_func;
    T_fUSBANYKA_RECEIVECALLBACK receiveok_func;
    T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func;
    
    T_U32 AnykaUSBRXCount;
    T_U32 AnykaUSBTXCount;
    
    T_BOOL bDataInRXFIFO;
    
    T_BOOL bTransmitDone;

    T_U8  *AnykaUSBTXBuf;
    T_U8  *AnykaUSBRXBuf;
    
}T_USBAnyka;

volatile T_USBAnyka m_usbanyka;

static T_VOID usbanyka_rx_notify(T_U32 *param, T_U32 len)
{
    //call receive callback function
    if(m_usbanyka.receive_func != AK_NULL)
        m_usbanyka.receive_func();
}

static T_VOID usbanyka_rx_finish(T_U32 *param, T_U32 len)
{
    //call receive ok callback function
    if(m_usbanyka.receiveok_func != AK_NULL)
        m_usbanyka.receiveok_func();
}

static T_VOID usbanyka_tx_finish(T_U32 *param, T_U32 len)
{
    //call send finish callback function
    if(m_usbanyka.sendfinish_func != AK_NULL)
        m_usbanyka.sendfinish_func();
}

//********************************************************************

T_VOID usbanyka_init(T_VOID)
{
    //init global variables
    m_usbanyka.AnykaUSBRXBuf = AK_NULL;
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = AK_NULL;
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.bDataInRXFIFO = AK_FALSE;
    m_usbanyka.bTransmitDone = AK_TRUE;

    //create task
    DrvModule_Create_Task(DRV_MODULE_USB_ANYKA);

    //map message
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_NOTIFY, usbanyka_rx_notify);
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_FINISH, usbanyka_rx_finish);
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_TX_FINISH, usbanyka_tx_finish);
}

T_VOID usbanyka_set_callback(T_fUSBANYKA_RECEIVECALLBACK receive_func, T_fUSBANYKA_RECEIVEOKCALLBACK receiveok_func, T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func)
{
    m_usbanyka.receive_func = receive_func;
    m_usbanyka.receiveok_func = receiveok_func;
    m_usbanyka.sendfinish_func = sendfinish_func;
}


T_BOOL usbanyka_enable(T_VOID)
{

    Usb_Slave_Standard.Usb_Device_Type =            USB_ANYKA | USB_MODE_11;
    
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer =                     (T_U8 *)drv_malloc(4096); 
    Usb_Slave_Standard.buf_len =                    4096;

    
    Usb_Slave_Standard.usb_get_device_descriptor = usbanyka_getdevicedescriptor;
    Usb_Slave_Standard.usb_get_config_descriptor = usbanyka_getconfigdescriptor;
    Usb_Slave_Standard.usb_get_string_descriptor = usbanyka_getstringdescriptor;

    Usb_Slave_Standard.usb_reset =          Usb_Anyka_USB_Reset;
    Usb_Slave_Standard.usb_suspend =        AK_NULL;//Fwl_Usb_SlaveReserved;
    Usb_Slave_Standard.usb_resume =         AK_NULL;//AK_NULLFwl_Usb_SlaveReserved;

    //init usb driver and usb stardard request driver
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    usb_slave_std_init();

    //set callback function
    usb_slave_set_callback(Usb_Slave_Standard.usb_reset, Usb_Slave_Standard.usb_suspend, Usb_Slave_Standard.usb_resume, Fwl_Usb_Anyka_EnumOK);
    usb_slave_set_tx_callback(EP2_INDEX, Fwl_Usb_Anyka_Send_Finish);
    usb_slave_set_rx_callback(EP1_INDEX, Fwl_Usb_Anyka_Notify, Fwl_Usb_Anyka_Receive_Finish);

    //enable usb controller
    usb_slave_device_enable(Usb_Slave_Standard.Usb_Device_Type);

    return AK_TRUE;
}


static T_U8 *usbanyka_getdevicedescriptor(T_U32 *count)
{
    *count = sizeof(device_desc);
    return (T_U8 *)&device_desc;
}

static T_U8 config[100];

static T_U8 *usbanyka_getconfigdescriptor(T_U32 *count)
{
    *count = sizeof(config_desc) + sizeof(if_desc) + sizeof(ep1_desc) + sizeof(ep2_desc) + sizeof(ep3_desc);

    memcpy(config, (T_U8 *)&config_desc, sizeof(config_desc));
    memcpy(config + sizeof(config_desc), (T_U8 *)&if_desc, sizeof(if_desc));
    memcpy(config + sizeof(config_desc) + sizeof(if_desc), (T_U8 *)&ep1_desc, sizeof(ep1_desc));
    memcpy(config + sizeof(config_desc)+ sizeof(if_desc) + sizeof(ep1_desc) , (T_U8 *)&ep2_desc, sizeof(ep2_desc));
    memcpy(config + sizeof(config_desc)+ sizeof(if_desc) + sizeof(ep1_desc) + sizeof(ep2_desc) , (T_U8 *)&ep3_desc, sizeof(ep3_desc));
    
    return config;
}

static T_U8 *usbanyka_getstringdescriptor(T_U8 index, T_U32 *count)
{
    if(index == 0)
    {
        *count = sizeof(sAnyka_String);
        return ((T_U8 *)sAnyka_String);
    }
    else if(index == 1)
    {
        *count = sizeof(sAnyka_String);
        return ((T_U8 *)sAnyka_String);
    }

    return AK_NULL;
}


//********************************************************************
static T_VOID Usb_Anyka_USB_Reset(T_U32 mode)
{
    if(mode == USB_MODE_20)
    {
        //set packet size to 512 in high speed mode
        ep2_desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        ep3_desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
    }
    else
    {
        //set packet size to 64 in full speed mode
        ep2_desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        ep3_desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;   
    }
}
//********************************************************************
T_VOID usbanyka_disable(T_VOID)
{
    //terminate task
    DrvModule_Terminate_Task(DRV_MODULE_USB_ANYKA);

    //free memory
    drv_free(Usb_Slave_Standard.Buffer);
    Usb_Slave_Standard.Buffer = AK_NULL;
    Usb_Slave_Standard.buf_len = 0;

    //disable usb controller
    usb_slave_set_state(USB_NOTUSE);
    usb_slave_free();
    usb_slave_device_disable();
}

//********************************************************************
static T_VOID Fwl_Usb_Anyka_Send_Finish(T_VOID)
{
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = AK_NULL;
    m_usbanyka.bTransmitDone = AK_TRUE;

    //send MESSAGE_TX_FINISH
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_TX_FINISH, AK_NULL);
}

static T_VOID Fwl_Usb_Anyka_Notify(T_VOID)
{
    m_usbanyka.bDataInRXFIFO = AK_TRUE;

    //send MESSAGE_RX_NOTIFY
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_NOTIFY, AK_NULL);
}


static T_VOID Fwl_Usb_Anyka_Receive_Finish(T_VOID)
{
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBRXBuf = AK_NULL;
    m_usbanyka.bTransmitDone = AK_TRUE;

    //send MESSAGE_RX_FINISH
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_FINISH, AK_NULL);
}

static T_VOID Fwl_Usb_Anyka_EnumOK(T_VOID)
{
    //init global variables
    m_usbanyka.AnykaUSBRXBuf = AK_NULL;
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = AK_NULL;
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.bDataInRXFIFO = AK_FALSE;
    m_usbanyka.bTransmitDone = AK_TRUE;

    akprintf(C3, M_DRVSYS, "enum ok\r\n");
}

T_S32 usbanyka_write(T_U8 *data, T_U32 data_len)
{
    T_U32 transcount = 0;

    //check param
    if(data == AK_NULL || data_len == 0)
        return -1;

    //check trans status
    if(!m_usbanyka.bTransmitDone)
    {
        akprintf(C3, M_DRVSYS, "transmit not finish\n");
        return -1;
    }

    m_usbanyka.AnykaUSBTXBuf = data;
    m_usbanyka.bTransmitDone = AK_FALSE;

    //start send data
    usb_slave_start_send(EP2_INDEX);
    usb_slave_data_in(EP2_INDEX, data, data_len);

    return transcount;
}

T_S32 usbanyka_read(T_U8 *data, T_U32 data_len)
{
    T_U32 transcount = 0;

    //check param
    if(data == AK_NULL || data_len == 0)
        return -1;

    //check trans status
    if(!m_usbanyka.bTransmitDone)
        return -1;

    if(!m_usbanyka.bDataInRXFIFO)
        return 0;

    m_usbanyka.AnykaUSBRXBuf = data;
    m_usbanyka.bDataInRXFIFO = AK_FALSE; //must before usb_slave_data_out
    m_usbanyka.bTransmitDone = AK_FALSE;

    //start receive data
    transcount = usb_slave_data_out(EP1_INDEX, data, data_len);

    return transcount;
}

//********************************************************************
#endif

