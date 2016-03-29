/**
 * @file drv_module.h
 * @brief 
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2010-06-18
 * @version 2.0
 */

#ifndef __DRV_MODULE_H__
#define __DRV_MODULE_H__


/** @name predefined event
    define terminate and task finish event
 */
/*@{*/
#define DRVMODULE_EVENT_TERMINATE       0
#define DRVMODULE_EVENT_TASK_FINISHED   0x80000000
/*@} */

/** @name wait event result
    define the result of wait event function
 */
/*@{*/
#define DRV_MODULE_SUCCESS       0
#define DRV_MODULE_TIMEOUT      -1
#define DRV_MODULE_ERROR        -2 
/*@} */

/**
 * @brief driver module callback type define.
 */
typedef T_VOID (*T_fDRV_CALLBACK)(T_U32 *param, T_U32 len);

/**
 * @brief driver module define.
 * define all the moudle
 */
typedef enum
{
    DRV_MODULE_AD = 0,
    DRV_MODULE_DA,
    DRV_MODULE_I2S_TX,
    DRV_MODULE_I2S_RX,
    DRV_MODULE_UART0,
    DRV_MODULE_UART1,
    DRV_MODULE_UART2,  
    DRV_MODULE_CAMERA,
    DRV_MODULE_DETECT,
    DRV_MODULE_RTC,
    DRV_MODULE_KEYPAD,
    DRV_MODULE_VTIMER,
    DRV_MODULE_TOUCH_SCREEN,

    DRV_MODULE_UVC,
    DRV_MODULE_USB_DISK,
    DRV_MODULE_USB_CDC,
    DRV_MODULE_USB_CAMERA,
    DRV_MODULE_USB_ANYKA,
    DRV_MODULE_USB_CMMB,

    DRV_MODULE_UDISK_HOST,
    DRV_MODULE_UVC_HOST,

    DRV_MODULE_USB_BUS,

    DRV_MODULE_FREQ,
    DRV_MODULE_HTIMER,
    DRV_MODULE_LCD,
    DRV_MODULE_NAND,
    DRV_MODULE_SDMMC,
    DRV_MODULE_SPI,
    DRV_MODULE_SDIO,

    DRV_MODULE_MAC,
   
    DRV_MODULE_GPIO_EDGE,
    
    DRV_MODULE_JPEG_CODEC,
    
    DRV_MODULE_FREQ_MGR,

    DRV_MODULE_UNDIFINE
}
E_DRV_MODULE;

/**
* @brief init driver module
* @author liao_zhijun
* @date 2010-06-18
* @return T_BOOL
* @retval AK_TRUE init success
* @retval AK_FALSE init fail
*/
T_BOOL DrvModule_Init();

/**
* @brief create task for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return T_BOOL
* @retval AK_TRUE create task success
* @retval AK_FALSE fail to create task
*/
T_BOOL DrvModule_Create_Task(E_DRV_MODULE module);

/**
* @brief terminate task for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return T_VOID
*/
T_VOID DrvModule_Terminate_Task(E_DRV_MODULE module);

/**
* @brief map callback function for message
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the giving message
* @param callback [in]:  callback function to be mapped
* @return T_BOOL
* @retval AK_TRUE map message success
* @retval AK_FALSE fail to map message 
*/
T_BOOL DrvModule_Map_Message(E_DRV_MODULE module, T_U32 msg, T_fDRV_CALLBACK callback);

/**
* @brief  send message to giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @param param [in]:  param for the message, its size is 12 bytes
* @return T_BOOL
* @retval AK_TRUE send message success
* @retval AK_FALSE fail to send message 
*/
T_BOOL DrvModule_Send_Message(E_DRV_MODULE module, T_U32 msg, T_U32 *param);

/**
* @brief  check if message come or not, if come, call the call back func
*            mainly used in non-os situation
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @return T_BOOL
* @retval AK_TRUE send message success
* @retval AK_FALSE fail to send message 
*/
T_VOID DrvModule_Retrieve_Message(E_DRV_MODULE module, T_U32 msg);


/**
* @brief  start protection for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return T_VOID
*/
T_VOID DrvModule_Protect(E_DRV_MODULE module);

/**
* @brief  stop protection for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return T_VOID
*/
T_VOID DrvModule_UnProtect(E_DRV_MODULE module);

/**
* @brief  send event for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be send
* @return T_BOOL
* @retval AK_TRUE set event success
* @retval AK_FALSE fail to set event
*/
T_BOOL DrvModule_SetEvent(E_DRV_MODULE module, T_U32 event);

/**
* @brief  wait for sepcifical event
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be wait for
* @param timeout [in]: timeout value, if the event still not come after the giving time, a timeout will be returned 
* @return T_S32
* @retval DRV_MODULE_SUCCESS wait event success
* @retval DRV_MODULE_TIMEOUT wait event timeout
* @retval DRV_MODULE_ERROR wait event error
*/
T_S32 DrvModule_WaitEvent(E_DRV_MODULE module, T_U32 event, T_U32 timeout);

#endif

