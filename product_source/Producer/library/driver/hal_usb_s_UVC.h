/**
*@file hal_usb_s_uvc.h
*@brief provide operations of how to use USB Video Class device.
*
*This file describe frameworks of USB Video Class device
*Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
*@author Huang Xin
*@date 2010-07-10
*@version 1.0
*/

#ifndef	_HAL_USB_S_UVC_H_
#define	_HAL_USB_S_UVC_H_

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_UVC USB_UVC
 *	@ingroup USB
 */
/*@{*/

#define     UVC_FRAME_MAX_WIDTH         640
#define     UVC_FRAME_MAX_HEIGHT        480
#define     UVC_FRAME_BUF_SIZE          (UVC_FRAME_MAX_WIDTH*UVC_FRAME_MAX_HEIGHT*2)
#define     UVC_YUV_BUF_SIZE            (UVC_FRAME_MAX_WIDTH*UVC_FRAME_MAX_HEIGHT*2)
#define     UVC_MJPEG_BUF_SIZE          (UVC_FRAME_MAX_WIDTH*UVC_FRAME_MAX_HEIGHT/2)
#define     UVC_PAYLOAD_BUF_SIZE        (UVC_FRAME_MAX_WIDTH*UVC_FRAME_MAX_HEIGHT*3)  
#define     UVC_FRAME_BUF_NUM           3
#define     UVC_FRAME_NUM               4



/**
 * @brief Define uvc controls implemented
 */

typedef enum
{
    UVC_CTRL_BRIGHTNESS,         ///< uvc device brightness control selector
    UVC_CTRL_CONTRAST,           ///< uvc device contrast control selector
    UVC_CTRL_SATURATION,         ///< uvc device saturation control selector 
    UVC_CTRL_RESOLUTION ,        ///< uvc device resolution control selector
    UVC_CTRL_NUM                 ///< uvc device control number
}T_eUVC_CONTROL;

/**
 * @brief Define uvc frame format
 */

typedef enum
{
    UVC_STREAM_YUV,             ///< uncompressed format
    UVC_STREAM_MJPEG            ///< motion jpeg format
}T_eUVC_STREAM_FORMAT;

typedef enum
{
    YUV_FORMAT_422,
    YUV_FORMAT_420
}T_eUVC_YUV_FORMAT;
/**
 * @brief Frame setting structure
 */

typedef struct _UVC_FRAME
{            
    T_U16   unWidth;       
    T_U16   unHeight;          
} T_UVC_FRAME_RES, *T_pUVC_FRAME_RES;

typedef T_VOID (*T_pUVC_VC_CTRL_CALLBACK)(T_eUVC_CONTROL dwControl, T_U32 value1,T_U32 value2);
typedef T_VOID (*T_pUVC_VS_CTRL_CALLBACK)(T_eUVC_CONTROL dwControl,T_U32 value1,T_U32 value2);
typedef T_VOID (*T_pUVC_FRAME_SENT_CALLBACK)(T_VOID);


/**
* @brief get uvc frame resolution
* @author Huang Xin
* @date 2010-07-10
* @param[in] pFrameRes uvc frame resolution struct
* @param[in] FrameId uvc frame id
* @return T_VOID
*/
T_VOID uvc_get_frame_res(T_pUVC_FRAME_RES pFrameRes,T_U32 FrameId);

/** 
 * @brief set UVC callback
 *
 *This function is called by application level to set control callback after uvc_init successful.
 * @author Huang Xin
 * @date 2010-07-10
 * @param[in]  vc_ctrl_callback implement video control interface controls 
 * @param[in] vs_ctrl_callback implement video stream interface  controls 
 * @param[in] frame_sent_callback used to notify that a frame was sent completely
 * @return T_VOID
 */
T_VOID uvc_set_callback(T_pUVC_VC_CTRL_CALLBACK vc_ctrl_callback, 
                            T_pUVC_VS_CTRL_CALLBACK vs_ctrl_callback,  
                           T_pUVC_FRAME_SENT_CALLBACK frame_sent_callback);

/**
* @brief	USB Video Class setup the advanced control features
*
* Implemented controls,such as brightness,contrast,saturation,called after uvc_init successful
* @author Huang Xin
* @date	2010-07-10
* @param[in] dwControl The advanced feature  control selector
* @param[in] ulMin The min value
* @param[in] ulMax The max value
* @param[in] ulDef The  def value
* @param[in] unRes The res value
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TURE means successful
*/
T_BOOL uvc_set_ctrl(T_eUVC_CONTROL dwControl, T_U32 ulMin, T_U32 ulMax, T_U32 ulDef, T_U32 unRes);

/**
* @brief Initialize uvc descriptor, MUST be called after uvc_set_ctrl
* @author Huang Xin
* @date	2010-07-10
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TURE means successful
*/
T_BOOL uvc_init_desc(T_VOID);

/**
* @brief Initialize uvc descriptor,yuv buffer,and map msg
* @author Huang Xin
* @date	2010-07-10
* @param[in] mode usb2.0 or usb1.0 
* @param[in] format The UVC frame format
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TURE means successful
*/
T_BOOL uvc_init(T_U32 mode,T_U8 format);


/**
* @brief	USB Video Class payload packing function
* @author Huang Xin
* @date	2010-07-10
* @param[out] pPayload The buffer to store the payload
* @param[in] pData The original frame data to be packed
* @param[in] dwSize The size of the frame
* @return T_U32
* @retval The size of the payload
*/
T_U32 uvc_payload(T_U8* pPayload, T_U8* pData, T_U32 dwSize);

/**
* @brief USB Video Class  parse yuv function
* @author Huang Xin
* @date	2010-07-10
* @param[out] pYUV The buffer to store the YUV frame
* @param[in] y The original y param addr
* @param[in] u The original u param addr
* @param[in] v The original v param addr
* @param[in] width The width of the yuv frame
* @param[in] height The height of the yuv frame
* @param[in] yuv_format The format of  yuv,yuv422 or yuv420
* @return T_U32
* @retval The size of the yuv frame
*/
T_U32 uvc_parse_yuv(T_U8 *pYUV,T_U8 *y,T_U8 *u,T_U8 *v,T_U32 width,T_U32 height,T_U8 yuv_format);


/**
* @brief	Send frame data via usb
* @author Huang Xin
* @date	2010-07-10
* @param[in] data_buf : buffer to be send. 
* @param[in] length: length of the buffer
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TURE means successful

*/
T_BOOL uvc_send(T_U8 *data_buf, T_U32 length);

/**
* @brief	Start UVC
* @author Huang Xin
* @date	2010-07-10
* @return T_BOOL
*/
T_BOOL uvc_start(T_VOID);

/**
* @brief	Stop UVC
* @author Huang Xin
* @date	2010-07-10
* @return	VOID
*/
T_VOID uvc_stop(T_VOID);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif

