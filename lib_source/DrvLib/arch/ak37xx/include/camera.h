/* @file camera.h
 * @brief Define structures and functions of camera driver
 * This file provide APIs of Camera, such as open, close, capture image. etc.
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-03-30
 * @version 1.0
 */
 
#ifndef __CAMERA_H__
#define __CAMERA_H__

/** @defgroup Camera  
 *	@ingroup M3PLATFORM
 */
/*@{*/

/**
 * @page page_camera CAMERA Porting Guide
 *
 * 1	Implementation Note \n\n
 *
 * 	AK37xx has an image sensor controller. Camera driver can control the image capture\n
 * operation through it.\n\n	
 *
 * 	Camera driver control the camera device and save the image data by read/write the \n
 * registers of AK37xx. The control operation includes image capture and save etc.\n\n
 * 
 * 	Camera parameters are set by SCCB bus access. SCCB interface is supported by AK37xx \n
 * and the code is ready in 'sccb.c'. Call the functions in it to access SCCB bus and finish setting\n
 * the parameters of camera.\n\n
 * 
 *	The camera power and reset are controlled by GPIOs of AK37xx. Camera driver should\n
 * define and operate the corresponding GPIO pins to control them.\n\n
 *
 *	Please refer "AK37xx Programmer's Guide" for details. Also refer the API document\n
 * for SCCB interface information.\n\n 
 *
 * 2	Porting Note \n\n
 *
 *	When a new camera device is used, a new camera driver is needed and should be ported\n
 * to the platform code. When porting a driver, follow these point.\n\n
 *
 *	First, the functions of the new driver should be completed and the same as the original copy,\n
 * include their parameters and return value. In the codes of the functions, controlling is \n
 * implemented by access to the registers of AK37xx.\n\n
 *
 *	Parameters are set by SCCB bus. Make sure the code is modified as the hardware design: \n
 * bus connection and bus address. Bus connection can be set by macro SIO_D and SIO_C. \n
 * Bus address can be set by macro CAMERA_SCCB_ADDR.\n\n
 * 
 *	Caution: I2C bus is also used in some device. It is almost the same as SCCB bus. \n
 * The code of I2C access is ready in 'i2c.c'. Just refer to the code and document of I2C bus. \n
 * Although, only 'sccb.c'  is supplied in the platform code, 'i2c.c'is ready and will be added soon.\n\n
 * 
 *	The camera power and reset is decided by the GPIO selection of the hardware design. \n
 * Please modify the macro define of the GPIO pins in 'gpio.h'. Make it the same as the original\n
 * design is suggested.\n\n
 *
 *	If one function is not supported by the new camera device, just delete the code of the \n
 * function and return a valid value if needed. But do not delete the functions! If one new function\n
 * is supported by the new camera device, write your code and call them from the original functions.\n
 *
 *
 */

typedef T_VOID (*T_fCamera_Interrupt_CALLBACK)(T_VOID);
 
 /**
  * @brief reset camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return T_VOID
  */
 T_VOID camctrl_enable(T_VOID);

 /**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk send to camera mclk 
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL camctrl_open(T_U32 mclk);

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID camctrl_disable(T_VOID);

/**
 * @brief capture an image in YUV420 format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstY     Y buffer to save the image data 
 * @param[out] dstU     U buffer to save the image data
 * @param[out] dstV     V buffer to save the image data 
 * @param[in] srcWidth  source width, output width of camera sensor
 * @param[in] srcHeight source height, output height of camera sensor
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture
 * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_YUV(T_U8 *dstY, T_U8 *dstU, T_U8 *dstV, T_U32 srcWidth, T_U32 srcHeight, 
                    T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout, T_BOOL bIsCCIR656);

/**
 * @brief capture an image in RGB format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dst      buffer to save the image data
 * @param[in] srcWidth  source width, output width of camera sensor
 * @param[in] srcHeight source height, output height of camera sensor
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture
 * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_RGB(T_U8 *dst, T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth,
                           T_U32 dstHeight, T_U32 timeout, T_BOOL bIsCCIR656);

/**
 * @brief capture an image in JPEG format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstJPEG      buffer to save the image data 
 * @param[out] JPEGlength   jpeg line length
 * @param[in] timeout time out value for capture
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_JPEG(T_U8 *dstJPEG, T_U32 *JPEGlength, T_U32 timeout);


/**
 * @brief set interrupt callback function
 * @author xia_wenting  
 * @date 2010-12-01
 * @param
 * @return 
 * @retval 
 */
T_VOID camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func);

/**
 * @brief start camera controller to capture a frame
 * @author xia_wenting 
 * @date 2010-12-07
 * @param[out] dstY     Y buffer to save the image data 
 * @param[out] dstU     U buffer to save the image data
 * @param[out] dstV     V buffer to save the image data 
 * @return T_VOID
 */
T_VOID camctrl_capture_frame(T_U8 *dstY, T_U8 *dstU, T_U8 *dstV);

/**
 * @brief set camera controller's register,is source and dest size
 * @author xia_wenting  
 * @date 2010-12-07
 * @param[in] srcWidth  source width, output width of camera sensor
 * @param[in] srcHeight source height, output height of camera sensor
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
 * @return T_VOID
 */
T_VOID camctrl_set_info(T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, 
                        T_U32 dstHeight, T_BOOL bIsCCIR656);

/**
 * @brief read camera controller's register, and check the frame finished or occur errorred
 * @author xia_wenting   
 * @date 2010-12-06
 * @param
 * @return T_BOOL
 * @retval AK_TRUE the frame finished
 * @retval AK_FALSE the frame not finished or occur errorred
 */
T_BOOL camctrl_check_status(T_VOID);


/*@}*/

#endif //__ARCH_CAMERA_H__
