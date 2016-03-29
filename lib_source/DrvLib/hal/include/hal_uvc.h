/**
* @FILENAME uvc_video.h
* @BRIEF USB Video Class type definition & data structure
* Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR Tommy
* @DATE 2007-12-10
* @UPDATE 2007-12-11
* @VERSION 0.0.4
* @REF USB Video Class 1.0a Spec
*/

#ifndef _UVC_H_
#define _UVC_H_

#include "anyka_types.h"

//uvc desc type
#define INTERFACE_ASSOCIATION_DESC_TYPE                     0x0B


//  Video Interface Class Code
#define     CC_VIDEO                                        0x0E

//  Video Interface Subclass Codes
#define     SC_UNDEFINED                                    0x00
#define     SC_VIDEOCONTROL                                 0x01
#define     SC_VIDEOSTREAMING                               0x02
#define     SC_VIDEO_INTERFACE_COLLECTION                   0x03

//  Video Interface Protocol Codes
#define     PC_PROTOCOL_UNDEFINED                           0x00

//  Video Class-Specific Descriptor Types
#define     CS_UNDEFINED                                    0x20
#define     CS_DEVICE                                       0x21
#define     CS_CONFIGURATION                                0x22
#define     CS_STRING                                       0x23
#define     CS_INTERFACE                                    0x24
#define     CS_ENDPOINT                                     0x25

//  Video Class-Specific VC Interface Descriptor Subtypes
#define     VC_DESCRIPTOR_UNDEFINED                         0x00
#define     VC_HEADER                                       0x01
#define     VC_INPUT_TERMINAL                               0x02
#define     VC_OUTPUT_TERMINAL                              0x03
#define     VC_SELECTOR_UNIT                                0x04
#define     VC_PROCESSING_UNIT                              0x05
#define     VC_EXTENSION_UNIT                               0x06

//  Video Class-Specific VS Interface Descriptor Subtypes
#define     VS_UNDEFINED                                    0x00
#define     VS_INPUT_HEADER                                 0x01
#define     VS_OUTPUT_HEADER                                0x02
#define     VS_STILL_IMAGE_FRAME                            0x03
#define     VS_FORMAT_UNCOMPRESSED                          0x04
#define     VS_FRAME_UNCOMPRESSED                           0x05
#define     VS_FORMAT_MJPEG                                 0x06
#define     VS_FRAME_MJPEG                                  0x07
#define     VS_FORMAT_MPEG2TS                               0x0A
#define     VS_FORMAT_DV                                    0x0C
#define     VS_COLORFORMAT                                  0x0D
#define     VS_FORMAT_VENDOR                                0x0E
#define     VS_FRAME_VENDOR                                 0x0F
#define     VS_FORMAT_FRAME_BASED                           0x10
#define     VS_FRAME_FRAME_BASED                            0x11
#define     VS_FORMAT_STREAM_BASED                          0x12

//  Video Class-Specific Endpoint Descriptor Subtypes
#define     EP_UNDEFINED                                    0x00
#define     EP_GENERAL                                      0x01
#define     EP_ENDPOINT                                     0x02
#define     EP_INTERRUPT                                    0x03

//  Video Class-Specific Request Codes
#define     RC_UNDEFINED                                    0x00
#define     SET_CUR                                         0x01
#define     GET_CUR                                         0x81
#define     GET_MIN                                         0x82
#define     GET_MAX                                         0x83
#define     GET_RES                                         0x84
#define     GET_LEN                                         0x85
#define     GET_INFO                                        0x86
#define     GET_DEF                                         0x87

//  VideoControl Interface Control Selectors
#define     VC_CONTROL_UNDEFINED                            0x00
#define     VC_VIDEO_POWER_MODE_CONTROL                     0x01
#define     VC_REQUEST_ERROR_CODE_CONTROL                   0x02

//  Terminal Control Selectors
#define     TE_CONTROL_UNDEFINED                            0x00

//  Selector Unit Control Selectors
#define     SU_CONTROL_UNDEFINED                            0x00
#define     SU_INPUT_SELECT_CONTROL                         0x01
#define     CT_CONTROL_UNDEFINED                            0x00
#define     CT_SCANNING_MODE_CONTROL                        0x01
#define     CT_AE_MODE_CONTROL                              0x02
#define     CT_AE_PRIORITY_CONTROL                          0x03
#define     CT_EXPOSURE_TIME_ABSOLUTE_CONTROL               0x04
#define     CT_EXPOSURE_TIME_RELATIVE_CONTROL               0x05
#define     CT_FOCUS_ABSOLUTE_CONTROL                       0x06
#define     CT_FOCUS_RELATIVE_CONTROL                       0x07
#define     CT_FOCUS_AUTO_CONTROL                           0x08
#define     CT_IRIS_ABSOLUTE_CONTROL                        0x09
#define     CT_IRIS_RELATIVE_CONTROL                        0x0A
#define     CT_ZOOM_ABSOLUTE_CONTROL                        0x0B
#define     CT_ZOOM_RELATIVE_CONTROL                        0x0C
#define     CT_PANTILT_ABSOLUTE_CONTROL                     0x0D
#define     CT_PANTILT_RELATIVE_CONTROL                     0x0E
#define     CT_ROLL_ABSOLUTE_CONTROL                        0x0F
#define     CT_ROLL_RELATIVE_CONTROL                        0x10
#define     CT_PRIVACY_CONTROL                              0x11

//  Processing Unit Control Selectors
#define     PU_CONTROL_UNDEFINED                            0x00
#define     PU_BACKLIGHT_COMPENSATION_CONTROL               0x01
#define     PU_BRIGHTNESS_CONTROL                           0x02
#define     PU_CONTRAST_CONTROL                             0x03
#define     PU_GAIN_CONTROL                                 0x04
#define     PU_POWER_LINE_FREQUENCY_CONTROL                 0x05
#define     PU_HUE_CONTROL                                  0x06
#define     PU_SATURATION_CONTROL                           0x07
#define     PU_SHARPNESS_CONTROL                            0x08
#define     PU_GAMMA_CONTROL                                0x09
#define     PU_WHITE_BALANCE_TEMPERATURE_CONTROL            0x0A
#define     PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL       0x0B
#define     PU_WHITE_BALANCE_COMPONENT_CONTROL              0x0C
#define     PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL         0x0D
#define     PU_DIGITAL_MULTIPLIER_CONTROL                   0x0E
#define     PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL             0x0F
#define     PU_HUE_AUTO_CONTROL                             0x10
#define     PU_ANALOG_VIDEO_STANDARD_CONTROL                0x11
#define     PU_ANALOG_LOCK_STATUS_CONTROL                   0x12

//  Extension Unit Control Selectors
#define     XU_CONTROL_UNDEFINED                            0x00

//  VideoStreaming Interface Control Selectors
#define     VS_CONTROL_UNDEFINED                            0x00
#define     VS_PROBE_CONTROL                                0x01
#define     VS_COMMIT_CONTROL                               0x02
#define     VS_STILL_PROBE_CONTROL                          0x03
#define     VS_STILL_COMMIT_CONTROL                         0x04
#define     VS_STILL_IMAGE_TRIGGER_CONTROL                  0x05
#define     VS_STREAM_ERROR_CODE_CONTROL                    0x06
#define     VS_GENERATE_KEY_FRAME_CONTROL                   0x07
#define     VS_UPDATE_FRAME_SEGMENT_CONTROL                 0x08
#define     VS_SYNCH_DELAY_CONTROL                          0x09

//  USB Terminal Types
#define     TT_VENDOR_SPECIFIC                              0x0100
#define     TT_STREAMING                                    0x0101

//  Input Terminal Types
#define     ITT_VENDOR_SPECIFIC                             0x0200
#define     ITT_CAMERA                                      0x0201
#define     ITT_MEDIA_TRANSPORT_INPUT                       0x0202

//  Output Terminal Types
#define     OTT_VENDOR_SPECIFIC                             0x0300
#define     OTT_DISPLAY                                     0x0301
#define     OTT_MEDIA_TRANSPORT_OUTPUT                      0x0302

//  External Terminal Types
#define     EXTERNAL_VENDOR_SPECIFIC                        0x0400
#define     COMPOSITE_CONNECTOR                             0x0401
#define     SVIDEO_CONNECTOR                                0x0402
#define     COMPONENT_CONNECTOR                             0x0403

#define UVC_GUID_FORMAT_YUY2  {0x59, 0x55, 0x59, 0x32, 0x00, 0x00, 0x10, 0x00,\
                              0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_NV12  {0x4e, 0x56, 0x31, 0x32, 0x00, 0x00, 0x10, 0x00,\
                              0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}



//  Standard Video Interface Collection IAD
//  Table 3-1
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_INTERFACE_ASSOCIATION_DESCRIPTOR
{
    T_U8    bLength;            //size of this descriptor, in bytes: 8
    T_U8    bDescriptorType;    //INTERFACE ASSOCIATION Descriptor
    T_U8    bFirstInterface;    //Interface number of the first VideoControl interface that is associated with this function.
    T_U8    bInterfaceCount;    //Number of contiguous VideoStreaming interfaces that are associated with this function.
    T_U8    bFunctionClass;     //CC_VIDEO. Video Interface Class code
    T_U8    bFunctionSubClass;  //SC_VIDEO_INTERFACE_COLLECTION. Video Interface Subclass code.
    T_U8    bFunctionProtocol;  //Not used. Must be set to PC_PROTOCOL_UNDEFINED.
    T_U8    iFunction;          //Index of a string descriptor that describes this interface.
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR, *T_pUVC_INTERFACE_ASSOCIATION_DESCRIPTOR;

//  Class-specific VC Interface Header Descriptor
//  Table 3-3
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR
{
    T_U8    bLength;            //Size of this descriptor, in bytes: 12+n
    T_U8    bDescriptorType;    //CS_INTERFACE descriptor type
    T_U8    bDescriptorSubType; //VC_HEADER descriptor subtype
    T_U16    bcdVDC;            //Video Device Class Specification release number in binary-coded decimal.
    T_U16    wTotalLength;      //Total number of bytes returned for the class-specific VideoControl interface descriptor.
    T_U32   dwClockFrequency;   //The device clock frequency in Hz.
    T_U8    bInCollection;      //The number of VideoStreaming interfaces in the Video Interface Collection to which this VideoControl interface belongs: n
    T_U8    baInterfaceNr1;     //Interface number of the first VideoStreaming interface in the Collection
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR, *T_pUVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR;

//  Input Terminal Descriptor
//  Table 3-4
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_INPUT_TERMINAL_DESCRIPTOR
{
    T_U8    bLength;            //Size of this descriptor, in bytes: 8 (+ x)
    T_U8    bDescriptorType;    //CS_INTERFACE descriptor type
    T_U8    bDescriptorSubtype; //VC_INPUT_TERMINAL descriptor subtype
    T_U8    bTerminalID;        //A non-zero constant that uniquely identifies the Terminal within the video function.
    T_U16    wTerminalType;     //Constant that characterizes the type of Terminal.
    T_U8    bAssocTerminal;     //ID of the Output Terminal to which this Input Terminal is associated, or zero (0) if no such association exists.
    T_U8    iTerminal;          //Index of a string descriptor, describing the Input Terminal.
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_INPUT_TERMINAL_DESCRIPTOR, *T_pUVC_INPUT_TERMINAL_DESCRIPTOR;

//  Output Terminal Descriptor
//  Table 3-5
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_OUTPUT_TERMINAL_DESCRIPTOR
{
    T_U8    bLength;            //Size of this descriptor, in bytes: 9 (+ x)
    T_U8    bDescriptorType;    //CS_INTERFACE descriptor type
    T_U8    bDescriptorSubtype; //VC_OUTPUT_TERMINAL descriptor subtype
    T_U8    bTerminalID;        //A non-zero constant that uniquely identifies the Terminal within the video function.
    T_U16    wTerminalType;     //Constant that characterizes the type of Terminal.
    T_U8    bAssocTerminal;     //Constant, identifying the Input Terminal to which this Output Terminal is associated, or zero (0) if no such association exists.
    T_U8    bSourceID;          //ID of the Unit or Terminal to which this Terminal is connected.
    T_U8    iTerminal;          //Index of a string descriptor, describing the Output Terminal.
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_OUTPUT_TERMINAL_DESCRIPTOR, *T_pUVC_OUTPUT_TERMINAL_DESCRIPTOR;

//  Camera Terminal Descriptor
//  Table 3-6
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_CAMERA_TERMINAL_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bTerminalID;
    T_U16    wTerminalType;
    T_U8    bAssocTerminal;
    T_U8    iTerminal;
    T_U16    wObjectiveFocalLengthMin;
    T_U16    wObjectiveFocalLengthMax;
    T_U16    wOcularFocalLength;
    T_U8    bControlSize;
    T_U16    bmControls;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_CAMERA_TERMINAL_DESCRIPTOR, *T_pUVC_CAMERA_TERMINAL_DESCRIPTOR;

//  Selector Unit Descriptor
//  Table 3-7
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_SELECTOR_UNIT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bUnitID;
    T_U8    bNrInPins;
    T_U8    baSourceID1;
    T_U8    baSourceID2;
    T_U8    iSelector;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_SELECTOR_UNIT_DESCRIPTOR, *T_pUVC_SELECTOR_UNIT_DESCRIPTOR;

//  Processing Unit Descriptor
//  Table 3-8
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_PROCESSING_UNIT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bUnitID;
    T_U8    bSourceID;
    T_U16    wMaxMultiplier;
    T_U8    bControlSize;
    T_U16    bmControls;
    T_U8    iProcessing;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_PROCESSING_UNIT_DESCRIPTOR, *T_pUVC_PROCESSING_UNIT_DESCRIPTOR;

//  Extension Unit Descriptor
//  Table 3-9
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_EXTENSION_UNIT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bUnitID;
    T_U8    guidExtensionCode[16];
    T_U8    bNumControls;
    T_U8    bNrInPins;
    T_U8    baSourceID1;
    T_U8    baSourceID2;
    T_U8    bControlSize;
    T_U8    bmControls;
    T_U8    iExtension;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_EXTENSION_UNIT_DESCRIPTOR, *T_pUVC_EXTENSION_UNIT_DESCRIPTOR;

//  Class-specific VS Interface Input Header Descriptor
//  Table 3-13
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bNumFormats;
    T_U16    wTotalLength;
    T_U8    bEndpointAddress;
    T_U8    bmInfo;
    T_U8    bTerminalLink;
    T_U8    bStillCaptureMethod;
    T_U8    bTriggerSupport;
    T_U8    bTriggerUsage;
    T_U8    bControlSize;
    T_U8    bmaControls;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR, *T_pUVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR;

#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_STILL_IMAGE_FRAME_DESCRIPTOR
{
    T_U8 bLength;
    T_U8 bDescriptorType;
    T_U8 bDescriptorSubtype;
    T_U8 bEndpointAddress;
    T_U8 bNumImageSizePatterns;
    T_U16 wWidth1;
    T_U16 wHeight1;
    T_U16 wWidth2;
    T_U16 wHeight2;
    T_U16 wWidth3;
    T_U16 wHeight3;
    T_U16 wWidth4;
    T_U16 wHeight4;
    T_U8  bNumCompressionPattern;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_STILL_IMAGE_FRAME_DESCRIPTOR,*T_pUVC_STILL_IMAGE_FRAME_DESCRIPTOR;

#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_COLOR_MATCHING_DESCRIPTOR
{
    T_U8 bLength;
    T_U8 bDescriptorType;
    T_U8 bDescriptorSubtype;
    T_U8 bColorPrimaries;
    T_U8 bTransferCharacteristics;
    T_U8 bMatrixCoefficients;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_COLOR_MATCHING_DESCRIPTOR,*T_pUVC_COLOR_MATCHING_DESCRIPTOR;


//  Class-specific VS Format Descriptor for Motion-JPEG
//  Table 3-1 in Payload MJPEG
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bFormatIndex;
    T_U8    bNumFrameDescriptors;
    T_U8    bmFlags;
    T_U8    bDefaultFrameIndex;
    T_U8    bAspectRatioX;
    T_U8    bAspectRatioY;
    T_U8    bmInterlaceFlags;
    T_U8    bCopyProtect;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR, *T_pUVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR;

//  Class-specific VS Format Descriptor for Uncompressed
//  Table 3-1 in Payload Uncompressed
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bFormatIndex;
    T_U8    bNumFrameDescriptors;
    T_U8    guidFormat[16];
    T_U8    bBitsPerPixel;
    T_U8    bDefaultFrameIndex;
    T_U8    bAspectRatioX;
    T_U8    bAspectRatioY;
    T_U8    bmInterlaceFlags;
    T_U8    bCopyProtect;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR, *T_pUVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR;

//  Class-specific VS Frame Descriptor
//  Table 3-2 in Payload MJPEG
//  Table 3-2 in Payload Uncompressed
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOSTREAM_FRAME_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubtype;
    T_U8    bFrameIndex;
    T_U8    bmCapabilities;
    T_U16    wWidth;
    T_U16    wHeight;
    T_U32   dwMinBitRate;
    T_U32   dwMaxBitRate;
    T_U32   dwMaxVideoFrameBufSize;
    T_U32   dwDefaultFrameInterval;
    T_U8    bFrameIntervalType;
    T_U32   dwMinFrameInterval;
    T_U32   dwMaxFrameInterval;
    T_U32   dwFrameIntervalStep;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR, *T_pUVC_VIDEOSTREAM_FRAME_DESCRIPTOR;

//  Class-specific VC Interrupt Endpoint Descriptor
//  Table 3.8.2.2
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR
{
    T_U8    bLength;
    T_U8    bDescriptorType;
    T_U8    bDescriptorSubType;
    T_U16    wMaxTransferSize;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR, *T_pUVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR;

//  Color Matching Descriptor
#ifdef __CC_ARM
__packed
#endif
typedef struct _USB_VIDEO_CMD
{
    //  6
    T_U8    bLength;
    //  CS_INTERFACE type
    T_U8    bDescriptorType;
    //  VS_COLORFORMAT
    T_U8    bDescriptorSubtype;
    //  This defines the color primaries and the reference white.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT.709, sRGB (default)
    //  2: BT.470-2 (M)3: BT.470-2 (B, G)
    //  4: SMPTE 170M
    //  5: SMPTE 240M
    //  6-255: Reserved
    T_U8    bColorPrimaries;
    //  This field defines the optoelectronic transfer characteristic of the source picture also called the gamma function.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT.709 (default)
    //  2: BT.470-2 M
    //  3: BT.470-2 B, G
    //  4: SMPTE 170M
    //  5: SMPTE 240M
    //  6: Linear (V = Lc)
    //  7: sRGB (very similar to BT.709)
    //  8-255: Reserved
    T_U8    bTransferCharacteristics;
    //  Matrix used to compute luma and chroma values from the color primaries.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT. 709
    //  2: FCC
    //  3: BT.470-2 B, G
    //  4: SMPTE 170M (BT.601, default)
    //  5: SMPTE 240M
    //  6-255: Reserved
    T_U8    bMatrixCoefficients;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
USB_VIDEO_CMD;

#ifdef __CC_ARM
__packed
#endif
typedef struct _VIDEO_PROBE_COMMIT_CONTROLS
{
    T_U16    bmHint;
    T_U8    bFormatIndex;
    T_U8    bFrameIndex;
    T_U32   dwFrameInterval;
    T_U16    wKeyFrameRate;
    T_U16    wPFrameRate;
    T_U16    wCompQuality;
    T_U16    wCompWindowSize;
    T_U16    wDelay;
    T_U32   dwMaxVideoFrameSize;
    T_U32   dwMaxPayloadTransferSize;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_VIDEO_PROBE_COMMIT_CONTROLS,*T_pVIDEO_PROBE_COMMIT_CONTROLS;


#endif  //  _UVC_H_
