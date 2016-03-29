
#include "Fwl_public.h"
#include "Eng_MsgQueue.h"
#include "Fwl_pfAudio.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

/*---------------------- BEGIN OF STATE s_pub_pre_message ------------------------*/

typedef struct {
    T_MSGBOX        pubmsgbox;
    T_U8            *lcd_buffer;
    T_RECT          msgRect;
} T_PRE_MSG_PARM;

static T_PRE_MSG_PARM *pPre_Msg_Parm;
static T_BOOL pub_pre_message_failed = AK_FALSE;

extern T_GLOBAL gb;
extern T_VOID Image_DecodeGIFPause(T_VOID);
extern T_VOID Image_DecodeGIFResume(T_VOID);


void initpub_pre_message(void)
{
    T_MSG_DATA msgData;
    //T_RECT msgRect;
    T_U8  *pLcdBuf = AK_NULL;
    T_S16 x, y;
    int iIndex = 0;
    int k = 0;    

    pPre_Msg_Parm = (T_PRE_MSG_PARM *)Fwl_Malloc(sizeof(T_PRE_MSG_PARM));
    if (pPre_Msg_Parm == AK_NULL)
    {
        pub_pre_message_failed = AK_TRUE;
        AK_ASSERT_PTR_VOID(pPre_Msg_Parm, "initpub_pre_message(): pPre_Msg_Parm malloc fail\n");
    }

#ifdef OS_ANYKA
    /*pPre_Msg_Parm->lcd_buffer = (T_U8 *)Fwl_hMalloc(Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
    if (pPre_Msg_Parm->lcd_buffer == AK_NULL)
    {
        pub_pre_message_failed = AK_TRUE;
        return;
    }

    memcpy((void *)pPre_Msg_Parm->lcd_buffer, (void *)Fwl_GetDispMemory(), Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
    */
#endif
#ifdef OS_WIN32
    /*pPre_Msg_Parm->lcd_buffer = (T_U8 *)Fwl_hMalloc(Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
    if (pPre_Msg_Parm->lcd_buffer == AK_NULL)
    {
        pub_pre_message_failed = AK_TRUE;
        return;
    }

    memcpy((void *)pPre_Msg_Parm->lcd_buffer, (void *)Fwl_GetDispMemory(), Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
    */
#endif

    if (!MsgQu_Pop(&msgData))
    {
        pub_pre_message_failed = AK_TRUE;
        AK_DEBUG_OUTPUT("initpub_pre_message(): pre message fail!");
        return;
    }

    MsgBox_InitStr(&pPre_Msg_Parm->pubmsgbox, 0, msgData.msgboxTitle, msgData.msgboxContent, msgData.msgboxType);
    MsgBox_SetDelay(&pPre_Msg_Parm->pubmsgbox, msgData.delayTime);
    MsgBox_GetRect(&pPre_Msg_Parm->pubmsgbox, &pPre_Msg_Parm->msgRect);

#ifdef OS_ANYKA    
#ifdef LCD_MODE_565
    /**if default display buffer is RGB565*/
    pPre_Msg_Parm->lcd_buffer = (T_U8 *)Fwl_Malloc(pPre_Msg_Parm->msgRect.width*pPre_Msg_Parm->msgRect.height*2 + 32);
    if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
    {
        pLcdBuf = (T_U8 *)Fwl_GetDispMemory565();
        for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
        {
            iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 2;
            for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
            {
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
            }
        }
    }
#else
    /**if default display buffer is RGB888*/
    pPre_Msg_Parm->lcd_buffer = (T_U8 *)Fwl_Malloc(pPre_Msg_Parm->msgRect.width*pPre_Msg_Parm->msgRect.height*3 + 32);
    if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
    {
        pLcdBuf = (T_U8 *)Fwl_GetDispMemory();
        for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
        {
            iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 3;
            for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
            {
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
            }
        }
    }
#endif
#endif
#ifdef OS_WIN32
    pPre_Msg_Parm->lcd_buffer = (T_U8 *)Fwl_Malloc(pPre_Msg_Parm->msgRect.width*pPre_Msg_Parm->msgRect.height*3 + 32);
    if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
    {
        pLcdBuf = (T_U8 *)Fwl_GetDispMemory();
        for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
        {
            iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 3;
            for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
            {
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
                pPre_Msg_Parm->lcd_buffer[k++] = pLcdBuf[iIndex++];
            }
        }
    }
#endif

    MsgQu_FreeMsg(&msgData);
    gb.InPublicMessage = AK_TRUE;

    //Image_DecodeGIFPause();
    pub_pre_message_failed = AK_FALSE;

    return;
}

void exitpub_pre_message(void)
{
     T_U8  *pLcdBuf = AK_NULL;
    T_S16 x, y;
    int iIndex = 0;
    int k = 0;

    if (pPre_Msg_Parm != AK_NULL)
    {
        //Image_DecodeGIFResume();

#ifdef OS_ANYKA
        /*if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
        {
            memcpy((void *)Fwl_GetDispMemory(), (void *)pPre_Msg_Parm->lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
            pPre_Msg_Parm->lcd_buffer = Fwl_Free(pPre_Msg_Parm->lcd_buffer);
        }
        */
#ifdef LCD_MODE_565
        /**if default display buffer is RGB565*/
        if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
        {
            pLcdBuf = (T_U8 *)Fwl_GetDispMemory565();
            for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
            {
                iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 2;
                for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
                {
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                }
            }
            
            pPre_Msg_Parm->lcd_buffer = Fwl_Free(pPre_Msg_Parm->lcd_buffer);
        }
#else     
        /**if default display buffer is RGB888*/
        if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
        {
            pLcdBuf = (T_U8 *)Fwl_GetDispMemory();
            for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
            {
                iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 3;
                for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
                {
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                }
            }
            pPre_Msg_Parm->lcd_buffer = Fwl_Free(pPre_Msg_Parm->lcd_buffer);
        }
#endif        
#endif
#ifdef OS_WIN32
        /*if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
        {
            memcpy((void *)Fwl_GetDispMemory(), (void *)pPre_Msg_Parm->lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
            pPre_Msg_Parm->lcd_buffer = Fwl_Free(pPre_Msg_Parm->lcd_buffer);
        }
        */
        if (pPre_Msg_Parm->lcd_buffer != AK_NULL)
        {
            pLcdBuf = (T_U8 *)Fwl_GetDispMemory();
            for(x = pPre_Msg_Parm->msgRect.top; x < pPre_Msg_Parm->msgRect.top + pPre_Msg_Parm->msgRect.height; x++)
            {
                iIndex = (x * Fwl_GetLcdWidth() + pPre_Msg_Parm->msgRect.left) * 3;
                for(y = pPre_Msg_Parm->msgRect.left; y < pPre_Msg_Parm->msgRect.left + pPre_Msg_Parm->msgRect.width; y++)
                {
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                    pLcdBuf[iIndex++] = pPre_Msg_Parm->lcd_buffer[k++];
                }
            }
            pPre_Msg_Parm->lcd_buffer = Fwl_Free(pPre_Msg_Parm->lcd_buffer);
        }
#endif

        pPre_Msg_Parm = Fwl_Free(pPre_Msg_Parm);
    }

    if (MsgQu_GetNum() > 0)
        VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, (vUINT32)AK_NULL); // TO GET ANOTHER COMMON MESSAGE
    else
        gb.InPublicMessage = AK_FALSE;

    /**avoid overload s_pub_pre_message*/
    gb.PubMsgAllow = AK_TRUE;

    // Before leaving, it should refresh LCD's buffer to previous contents.
    // Calling Fwl_RefreshDisplay() to refresh the buffer of 565.
    Fwl_RefreshDisplay();
}

void paintpub_pre_message(void)
{
    T_RECT msgRect;

    MsgBox_Show(&pPre_Msg_Parm->pubmsgbox);
    MsgBox_GetRect(&pPre_Msg_Parm->pubmsgbox, &msgRect);
	Fwl_RefreshDisplay();
	Fwl_RefreshDisplay();//SW10A00001511暂再调用一次刷新接口进行数据缩放
}

unsigned char handlepub_pre_message(T_EVT_CODE event, T_EVT_PARAM* pEventParm)
{    
    T_eBACK_STATE   mboxRet;

    if(M_EVT_Z05COM_MSG == event)
    {
        return 0;
    }

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pPre_Msg_Parm->pubmsgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (pub_pre_message_failed == AK_TRUE)
        m_triggerEvent(M_EVT_PRE_EXIT, pEventParm);

    mboxRet = MsgBox_Handler(&pPre_Msg_Parm->pubmsgbox, event, pEventParm);
    switch (mboxRet)
    {
    case eReturn:
    case eNext:
        m_triggerEvent(M_EVT_PRE_EXIT, pEventParm);
        break;
    case eHome:
        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
        break;
    default:
        break;
    }

    return 0;
}

