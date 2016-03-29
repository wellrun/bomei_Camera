/**
 * @filename usb_slave_std.h
 * @brief: AK3223M standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-12-13
 * @version 1.0
 */

#ifndef __USB_STD_H__
#define __USB_STD_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB Framework USB Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************
#define SETUP_PKT_SIZE          8   //setup packet must be 8 bytes

/**descriptor type*/
#define DEVICE_DESC_TYPE          0x01
#define CONFIG_DESC_TYPE          0x02
#define STRING_DESC_TYPE          0x03
#define IF_DESC_TYPE              0x04
#define EP_DESC_TYPE              0x05
#define DEVICE_QUALIFIER_DESC_TYPE 0x06
#define OTHER_SPEED_CONFIGURATION_DESC_TYPE 0x07

/**USB Device Class Code (UNKNOWN is set 0)*/
#define USB_DEVICE_CLASS_RESERVED                   0x00
#define USB_DEVICE_CLASS_AUDIO                      0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS             0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE            0x03
#define USB_DEVICE_CLASS_MONITOR                    0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE         0x05
#define USB_DEVICE_CLASS_POWER                      0x06
#define USB_DEVICE_CLASS_PRINTER                    0x07
#define USB_DEVICE_CLASS_STORAGE                    0x08
#define USB_DEVICE_CLASS_HUB                        0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC            0xFF

/**interface descriptor length*/
#define INTERFACE_DESC_LEN        9
#define CONFIGURE_DESC_LEN        9
#define ENDPOINT_DESC_LEN         7
#define ENDPOINT_NUMBER           3

#define ENDPOINT_TYPE_CONTROL     0
#define ENDPOINT_TYPE_SYNCH       1
#define ENDPOINT_TYPE_BULK        2
#define ENDPOINT_TYPE_INTR        3

#define ENDPOINT_DIR_IN           0x80
#define ENDPOINT_DIR_OUT          0

#define USB_STD_GETSTATUS           0
#define USB_STD_CLEARFEATURE        1
#define USB_STD_SETFEATURE          3
#define USB_STD_SETADDRESS          5
#define USB_STD_GETDESCRIPTOR       6
#define USB_STD_SETDESCRIPTOR       7
#define USB_STD_GETCONFIG           8
#define USB_STD_SETCONFIG           9
#define USB_STD_GETINTERFACE        10
#define USB_STD_SETINTERFACE        11
#define USB_STD_SYCHFAME            12
#define USB_STD_END                 16

#define USB_STD_DIR_HOST2DEV        0
#define USB_STD_DIR_DEV2HOST        (1 << 7)
#define USB_STD_REQTYPE_STD         0
#define USB_STD_REQTYPE_CLS         (1 << 5)
#define USB_STD_REQTYPE_VEN         (1 << 6)
#define USB_STD_REQTYPE_RES         (USB_STD_REQTYPE_CLS | USB_STD_REQTYPE_VEN)
#define USB_STD_REC_DEVICE          0
#define USB_STD_REC_INTERFACE       1
#define USB_STD_REC_ENDPOINT        (1 << 1)
#define USB_STD_REC_ELSE            3

/**@Name Test Mode Seletors
 */
#define Test_J                 0x1
#define Test_K                 0x2
#define Test_SE0_NAK           0x3
#define Test_Packet            0x4
#define Test_Force_Enable      0x5

 /**@Name Recipient define
  */
#define RECIPIENT_DEVICE        0       // Recipient
#define RECIPIENT_INTERFACE     1       // Recipient
#define RECIPIENT_ENDPOINT      2       // Recipient
#define RECIPIENT_OTHER         3       // Recipient

/////////////////////////////////////////////////////////////////////
// Feature selectors
#define DEVICE_REMOTE_WAKEUP    0x0001          //
#define ENDPOINT_HALT           0x0000          //
#define TEST_MODE               0x0002          //

//********************************************************************
//Device Descriptor
#ifdef __CC_ARM
__packed
#endif
struct S_USB_DEVICE_DESCRIPTOR
{
    T_U8     bLength;              //length of this descriptor
    T_U8     bDescriptorType;      //descriptor type - here it is device
    T_U16    bcdUSB;               //USB Specification Release Number in Binary-Coded Decimal
    T_U8     bDeviceClass;         //Class code (assigned by the USB).
    T_U8     bDeviceSubClass;      //Subclass code (assigned by the USB).
    T_U8     bDeviceProtocol;      //Protocol code (assigned by the USB).
    T_U8     bMaxPacketSize0;      //Maximum packet size for endpoint zero
    T_U16    idVendor;             //Vendor ID (assigned by the USB)
    T_U16    idProduct;            //Product ID (assigned by the manufacturer)
    T_U16    bcdDevice;            //Device release number in binary-coded decimal
    T_U8     iManufacturer;        //Index of string descriptor describing manufacturer
    T_U8     iProduct;             //Index of string descriptor describing product
    T_U8     iSerialNumber;        //Index of string descriptor describing the device's serial number
    T_U8     bNumConfigurations;   //Number of possible configurations
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_DEVICE_DESCRIPTOR T_USB_DEVICE_DESCRIPTOR;
/** @brief Configure Descriptor
 *
*/
#ifdef __CC_ARM
__packed
#endif
struct S_USB_CONFIGURATION_DESCRIPTOR
{
    T_U8     bLength;              //Size of this descriptor in bytes
    T_U8     bDescriptorType;      //CONFIGURATION Descriptor Type
    T_U16    wTotalLength;         //Total length of data returned
    T_U8     bNumInterfaces;       //Number of interfaces supported by this configuration
    T_U8     bConfigurationValue;  //Value to use as an argument
    T_U8     iConfiguration;       //Index of string descriptor describing this configuration
    T_U8     bmAttributes;         //Configuration characteristics
    T_U8     MaxPower;             //Maximum power consumption of the USB
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_CONFIGURATION_DESCRIPTOR T_USB_CONFIGURATION_DESCRIPTOR;

/** @brief Interface Descriptor
 *
*/
#ifdef __CC_ARM
__packed
#endif
struct S_USB_INTERFACE_DESCRIPTOR
{
    T_U8     bLength;               //Size of this descriptor in bytes
    T_U8     bDescriptorType;       //INTERFACE Descriptor Type
    T_U8     bInterfaceNumber;      //Number of interface.
    T_U8     bAlternateSetting;     //Value used to select alternate setting
    T_U8     bNumEndpoints;         //Number of endpoints used
    T_U8     bInterfaceClass;       //Class code (assigned by the USB).
    T_U8     bInterfaceSubClass;    //Subclass code (assigned by the USB).
    T_U8     bInterfaceProtocol;    //Protocol code (assigned by the USB).
    T_U8     iInterface;            //Index of string descriptor describing this interface
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_INTERFACE_DESCRIPTOR T_USB_INTERFACE_DESCRIPTOR;

/** @brief ENDPOINT Descriptor
 *  ep0 need not descriptor
*/
#ifdef __CC_ARM
__packed
#endif
struct S_USB_ENDPOINT_DESCRIPTOR
{
    T_U8    bLength;                //Size of this descriptor in bytes
    T_U8    bDescriptorType;        //ENDPOINT Descriptor Type
    T_U8    bEndpointAddress;       //The address of the endpoint on the USB
    T_U8    bmAttributes;           //This field describes the endpoint
    T_U16   wMaxPacketSize;         //Maximum packet size this endpoint
    T_U8    bInterval;              //Interval for polling endpoint for data
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_ENDPOINT_DESCRIPTOR T_USB_ENDPOINT_DESCRIPTOR;

#ifdef __CC_ARM
    __packed
#endif

     struct S_USB_DEVICE_QUALIFIER_DESCRIPTOR 
    {
        T_U8 bLength;
        T_U8 bDescriptorType;
        T_U16 bcdUSB;
        T_U8 bDeviceClass;
        T_U8 bDeviceSubClass;
        T_U8 bDeviceProtocol;
        T_U8 bMaxPacketSize0;
        T_U8 bNumConfigurations;
        T_U8 bReserved;
    }
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_DEVICE_QUALIFIER_DESCRIPTOR T_USB_DEVICE_QUALIFIER_DESCRIPTOR;

#ifdef __CC_ARM
__packed
#endif
struct S_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR
{
    T_U8     bLength;              //Size of this descriptor in bytes
    T_U8     bDescriptorType;      //CONFIGURATION Descriptor Type
    T_U16    wTotalLength;         //Total length of data returned
    T_U8     bNumInterfaces;       //Number of interfaces supported by this configuration
    T_U8     bConfigurationValue;  //Value to use as an argument
    T_U8     iConfiguration;       //Index of string descriptor describing this configuration
    T_U8     bmAttributes;         //Configuration characteristics
    T_U8     MaxPower;             //Maximum power consumption of the USB
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR T_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR;



#ifdef __CC_ARM
__packed
#endif
struct  S_USB_DEVICE_REQUEST{
    T_U8    bmRequestType;  //device request type
    T_U8    bRequest;       //device request
    T_U16   wValue;         //value
    T_U16   wIndex;         //index
    T_U16   wLength;        //lenth
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

typedef struct S_USB_DEVICE_REQUEST T_UsbDevReq;
//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
