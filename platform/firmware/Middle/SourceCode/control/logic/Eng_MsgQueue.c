/**
 * @file Eng_Queue.c
 * @brief ANYKA software
 * a queue operation support
 * @author pyxue
 * @date 2001-07-27
 * @version 1.0
 */

#include "Eng_MsgQueue.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Akos_api.h"

#define MAX_MAILBOX_NUMBER  10

typedef struct
{
    T_MSG_DATA  stack[MAX_MAILBOX_NUMBER];
    T_U8        top;
} T_MSG_QUEUE;

static T_MSG_QUEUE s_MessageQueue;
static T_hSemaphore s_SemMsgQu;
/**
 * @brief init message queue
 * @author 
 * @date 2001-07-27
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID MsgQu_Init(T_VOID)
{
    s_MessageQueue.top = 0;
    s_SemMsgQu = AK_Create_Semaphore(1, AK_PRIORITY);
}

/**
 * @brief get message number of the message queue
 * @author 
 * @date 2001-07-27
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_U8 MsgQu_GetNum(T_VOID)
{
 	T_U8 ret;

 	AK_Obtain_Semaphore(s_SemMsgQu, AK_SUSPEND);
    ret = s_MessageQueue.top;
    AK_Release_Semaphore(s_SemMsgQu);

    return ret;
}

/**
 * @brief push the event into message queue
 * @author 
 * @date 2001-07-27
 * @param T_U8 *MessageTitle
 * @param T_U8 *MessageContent
 * @param T_U8 MessageType
 * @param T_S16 delayTime
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgQu_Push(T_pCWSTR msgboxTitle, T_pCWSTR msgboxContent, T_U8 msgboxType, T_S16 delayTime)
{
	AK_Obtain_Semaphore(s_SemMsgQu, AK_SUSPEND);
	
    if (s_MessageQueue.top >= MAX_MAILBOX_NUMBER)
    {
    	AK_Release_Semaphore(s_SemMsgQu);
    	return AK_FALSE;
    }

    s_MessageQueue.stack[s_MessageQueue.top].msgboxType = msgboxType;
    s_MessageQueue.stack[s_MessageQueue.top].delayTime = delayTime;
    s_MessageQueue.stack[s_MessageQueue.top].msgboxTitle = (T_U16*)Fwl_Malloc((Utl_UStrLen(msgboxTitle) + 1) << 1);
    if (s_MessageQueue.stack[s_MessageQueue.top].msgboxTitle == AK_NULL)
    {
    	AK_Release_Semaphore(s_SemMsgQu);
        return AK_FALSE;
    }
    
    Utl_UStrCpy(s_MessageQueue.stack[s_MessageQueue.top].msgboxTitle, msgboxTitle);
    s_MessageQueue.stack[s_MessageQueue.top].msgboxContent = (T_U16*)Fwl_Malloc((Utl_UStrLen(msgboxContent) + 1) << 1);
    if (s_MessageQueue.stack[s_MessageQueue.top].msgboxContent == AK_NULL)
    {
        Fwl_Free(s_MessageQueue.stack[s_MessageQueue.top].msgboxContent);
        AK_Release_Semaphore(s_SemMsgQu);
        return AK_FALSE;
    }
    Utl_UStrCpy(s_MessageQueue.stack[s_MessageQueue.top].msgboxContent, msgboxContent);
   
    s_MessageQueue.top++;

    AK_Release_Semaphore(s_SemMsgQu);

    return AK_TRUE;
}

/**
 * @brief pop an event from message queue
 * @author 
 * @date 2001-07-27
 * @param T_MSG_DATA *msgData
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgQu_Pop(T_MSG_DATA *msgData)
{
	AK_Obtain_Semaphore(s_SemMsgQu, AK_SUSPEND);
	
    if (s_MessageQueue.top == 0)
    {
    	AK_Release_Semaphore(s_SemMsgQu);
        return AK_FALSE;
    }

    s_MessageQueue.top--;
    *msgData = s_MessageQueue.stack[s_MessageQueue.top];

    AK_Release_Semaphore(s_SemMsgQu);

    return AK_TRUE;
}

/**
 * @brief free message data
 * @author 
 * @date 2001-07-27
 * @param T_MSG_DATA *msgData
 * @return T_BOOL
 * @retval
 */
T_VOID MsgQu_FreeMsg(T_MSG_DATA *msgData)
{
    msgData->msgboxTitle = Fwl_Free(msgData->msgboxTitle);
    msgData->msgboxContent = Fwl_Free(msgData->msgboxContent);
    return;
}

/////////////////////////////////
