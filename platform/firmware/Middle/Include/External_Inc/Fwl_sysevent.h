#ifndef __FWL_SYSEVENT_H__
#define __FWL_SYSEVENT_H__

#include "anyka_types.h"

/*======================================================================================================*/
#define AK_SYS_EVT_BASE    0xffff0000

//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef T_BOOL (*T_pfnEvtCmp)(const T_VOID *pMailBox1, const T_VOID *pMailBox2);

/*======================================================================================================*/

/*系统事件掩码定义*/
enum
{
    SYS_EVT_MSK_NONE     = 0x00000000,
    SYS_EVT_COMM_MSK     = 0x00000001,
    SYS_EVT_APP_MSK      = 0x00000002,
    SYS_EVT_INPUT_MSK    = 0x00000004, 
    SYS_EVT_VATC_MSK     = 0x00000008, 
    SYS_EVT_AUDIO_MSK    = 0x00000010, 
    SYS_EVT_MMI_MSK      = 0x00000020,    
    SYS_EVT_BT_MSK       = 0x00000040,
    SYS_EVT_XXX_MSK      = 0x00000080
};

/*系统事件枚举*/
typedef enum 
{
    /* 通用系统事件*/
    SYS_EVT_COMM_MSK_BEGIN   = AK_SYS_EVT_BASE,
    SYS_EVT_TIMER,
    SYS_EVT_SUBTHREAD_NOTIFY,
    SYS_EVT_SUBTHREAD_FINISH,
    SYS_EVT_COMM_MSK_END,
    //=============================================
    /* 应用系统事件*/
    SYS_EVT_APP_MSK_BEGIN    = SYS_EVT_COMM_MSK_END,
    SYS_EVT_APP_START,
    SYS_EVT_APP_ACTIVE,
    SYS_EVT_APP_DEACTIVE,
    SYS_EVT_APP_STOP,
    SYS_EVT_APP_MSK_END,
    //=============================================
    /* 输入系统事件*/
    SYS_EVT_INPUT_MSK_BEGIN  = SYS_EVT_APP_MSK_END,
    SYS_EVT_TSCR,
    SYS_EVT_KEYDOWN,
    SYS_EVT_KEYPRESS,
    SYS_EVT_KEYUP,        
    SYS_EVT_INPUT_MSK_END,
    //=============================================
    /* 串口系统事件*/
    SYS_EVT_VATC_MSK_BEGIN   = SYS_EVT_INPUT_MSK_END,
    SYS_EVT_VATC_MSK_END,
    //=============================================
    /* 音频系统事件*/
    SYS_EVT_AUDIO_MSK_BEGIN  = SYS_EVT_VATC_MSK_END,
    SYS_EVT_MEDIA,
    SYS_EVT_SDCB_MESSAGE,
    SYS_EVT_AUDIO_ABPLAY,
    SYS_EVT_AUDIO_MSK_END,
        
    //=============================================
    /* MMI系统事件*/
    SYS_EVT_MMI_MSK_BEGIN  = SYS_EVT_AUDIO_MSK_END,
    SYS_EVT_CTL_KILLFOCUS,
    SYS_EVT_Z99COM_SMS_ME,
    SYS_EVT_Z99COM_CDS_ME,
    SYS_EVT_Z98COM_SMS_SIM,
    SYS_EVT_Z04COM_SMS_CLASS0,
    SYS_EVT_Z05COM_MSG,
    SYS_EVT_PUB_TIMER,
    SYS_EVT_GPS_DATA_READY,
    SYS_EVT_PINIO,
	SYS_EVT_SD_PLUG,
	SYS_EVT_SDIO_PLUG,
	SYS_EVT_SDMMC_PLUG,
	SYS_EVT_USB_DETECT,
    SYS_EVT_USB_PLUG,
    SYS_EVT_USB_SEND_REQUEST,
    SYS_EVT_V24_DATA,
    SYS_EVT_USER_KEY,
    SYS_EVT_Z10COM_KEY_UNLOCK_HINT,
    SYS_EVT_Z99COM_STK,
    SYS_EVT_AUDIO_RESUME_PLAY,
    SYS_EVT_Z11CHARGER_GPIO,
    SYS_EVT_RTC,
    SYS_EVT_PRE_EXIT,
    SYS_EVT_PASTE_EXIT,
    SYS_EVT_MMI_MSK_END,
   
    //=============================================
    /*BT系统事件 */
    SYS_EVT_BT_MSK_BEGIN = SYS_EVT_MMI_MSK_END, 
    SYS_EVT_BLUETOOTH,
    SYS_EVT_BT_MSK_END,
    //=============================================
    /* 扩展系统事件*/  
    SYS_EVT_XXX_MSK_BEGIN = SYS_EVT_BT_MSK_END,    
    SYS_EVT_XXX_MSK_END
} T_SYS_EVENTID;
/*为了兼容以前的MMI，目前定义的系统事件不是很合理，以后需要优化。*/
/*====================================================================================================*/

/*事件参数定义*/
typedef union                                         
{  
    struct                                             
    {                                                
        T_U8 Param1;                                       
        T_U8 Param2;                                      
        T_U8 Param3;                                      
        T_U8 Param4;                                      
        T_U8 Param5;                                      
        T_U8 Param6;                                       
        T_U8 Param7;                                       
        T_U8 Param8;                                     
    } c;                                               
                                         
    struct                                              
    {                                                  
        T_U16 Param1;                         
        T_U16 Param2;                         
        T_U16 Param3;                         
        T_U16 Param4;                         
    } s;                                               

    struct                                              
    {                                                
        T_U32 Param1;                               
        T_U32 Param2;                               
    } w;      
    T_pVOID     lpParam;                                      
    T_U32       lParam; 
} T_SYS_PARAM;    
/*****************************************************************************************************
注:此事件参数定义为联合体，要注意最后两个变量是和其它结构体一样，
公用一块空间的，即T_SYS_PARAM大小为8字节。
******************************************************************************************************/

/*事件标识定义*/
typedef T_U32  T_SYS_EVTID;

/*消息机构体定义*/
typedef struct
{
    /*为了兼MMI，下面两项跟以前是一致的，需要赋值*/
	T_SYS_EVTID event;   /*事件标识*/
	T_SYS_PARAM param;   /*事件参数*/
    /*##############################################*/
    /*以下是内部使用的数据变量，用户不需要赋值。*/
    T_BOOL      bIsUnique;
    T_BOOL      bIsHead;
    T_pfnEvtCmp fnCmp;
    /*##############################################*/
}T_SYS_MAILBOX;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************************/
/**
===================================================================================================================
Function:      T_S32 AK_PostEventEx(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                                    T_BOOL bIsUnique, T_BOOL bIsHead,
                                    T_BOOL bIsTerminalInput);
Description:   发送系统事件至系统队列，并激活高级中断服务程序来派发系统事件至应用队列。
Parameters:
   pMailBox   [in] 指向要发送消息的指针。
   pfnCmp     [in] 消息比较函数，在发送唯一消息时会用到，其它情况忽略。
   bIsUnique  [in] 标识发送的消息是否在队列中要唯一。
   bIsHead    [in] 标识发送的消息是否要放在队列的头。
   bIsTerminalInput [in] 标识发送的消息是输入事件还是非输入事件。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
   AK_PostEvent(T_SYS_MAILBOX *pMailBox);
   DES: 发送一个非输入事件至队列尾。
   AK_PostEventToHead(T_SYS_MAILBOX *pMailBox);
   DES: 发送一个非输入事件至队列头。
   AK_PostUniqueEvent(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp);
   DES: 发送一个非输入事件至队列尾，如果队列已有该事件则丢弃该事件。
   AK_PostUniqueEventToHead(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp);
   DES: 发送一个非输入事件至队列头，如果队列已有该事件则丢弃该事件。
   AK_PostTerminalInputEvent(T_SYS_MAILBOX *pMailBox);
   DES: 发送一个输入事件至队列尾。
   AK_PostTerminalInputEventEx(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                               T_BOOL bIsUnique, T_BOOL bIsHead);
   DES: 发送一个输入事件至队列，通过参数bIsUnique和bIsHead可以让事件在队列中唯一和放在
        队列头。
====================================================================================================================
*/
/******************************************************************************************************************/

T_S32 AK_PostEventEx(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                     T_BOOL bIsUnique, T_BOOL bIsHead,
                     T_BOOL bIsTerminalInput);

#define AK_PostEvent(m)                  AK_PostEventEx(m, AK_NULL, AK_FALSE, AK_FALSE, AK_FALSE)
#define AK_PostEventToHead(m)            AK_PostEventEx(m, AK_NULL, AK_FALSE, AK_TRUE, AK_FALSE)
#define AK_PostUniqueEvent(m,cmp)        AK_PostEventEx(m, cmp, AK_TRUE, AK_FALSE, AK_FALSE)
#define AK_PostUniqueEventToHead(m,cmp)  AK_PostEventEx(m, cmp, AK_TRUE, AK_TRUE, AK_FALSE)
/*===============================================================================*/
//T_S32 AK_PostTerminalInputEventEx(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, T_BOOL bIsUnique, T_BOOL bIsHead);
#define AK_PostTerminalInputEventEx(m,cmp,u,h)   AK_PostEventEx(m, cmp, u, h, AK_TRUE)
#define AK_PostTerminalInputEvent(m)             AK_PostEventEx(m, AK_NULL, AK_FALSE, AK_FALSE, AK_TRUE)

#endif //__FWL_SYSEVENT_H__

