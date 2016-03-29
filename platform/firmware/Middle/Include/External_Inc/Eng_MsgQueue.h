/**
 * @file Eng_Queue.h
 * @brief ANYKA software
 * Queue Header file 
 */


#ifndef __ENG_MESSAGE_QUEUE_H__
#define __ENG_MESSAGE_QUEUE_H__ 

#include "anyka_types.h"

typedef struct
{
    T_U16   msgboxType;
    T_S16   delayTime;
    T_U16   *msgboxTitle;
    T_U16   *msgboxContent;
} T_MSG_DATA;

T_VOID MsgQu_Init(T_VOID);
T_U8   MsgQu_GetNum(T_VOID);
T_BOOL MsgQu_Push(T_pCWSTR msgboxTitle, T_pCWSTR msgboxContent, T_U8 msgboxType, T_S16 delayTime);
T_BOOL MsgQu_Pop(T_MSG_DATA *msgData);
T_VOID MsgQu_FreeMsg(T_MSG_DATA *msgData);

#endif
