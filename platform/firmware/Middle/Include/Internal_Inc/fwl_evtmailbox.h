/** @file 
 * @brief header file for interface and definition to handle event
 *        queue.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __FWL_EVTMAILBOX_H__
#define __FWL_EVTMAILBOX_H__


#include "Lib_event.h"
#include "string.h"
#include "Fwl_sysevent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/*===========================================================================*/

 /**
 * @brief Initialize the event mailbox
 * @param[in] void
 * @return void
 */
void MB_Init(void);

/**
 * @brief Peek the event from the event queue
 * @param[in] mailbox   event queue
 * @return whether get event to process or not
 */
vBOOL MB_PeekEvent(T_SYS_MAILBOX *mailbox, T_BOOL bIsInputSysEvent);

/**
 * @brief Peek the event from the event queue
 * @param[in] bIsInputSysEvent   event queue
 * @return whether get event to process or not
 */
vBOOL MB_DeleteEvent(T_BOOL bIsInputSysEvent);

/**
 * @brief Get the event from the event queue
 * @param[in] mailbox   event queue
 * @return whether get event to process or not
 */
vBOOL MB_GetEvent( T_SYS_MAILBOX *mailbox, T_BOOL bIsInputSysEvent);

/**
 * @brief push appointed event to the tail of event queue 
 * @param[in] mailbox   event to be pushed 
 * @return success or not
 */
vBOOL MB_PushEvent(T_SYS_MAILBOX* mailbox, T_pfnEvtCmp compareFun, 
                       T_BOOL bIsUnique, T_BOOL bIsHead, T_BOOL bIsInputSysEvent);

#endif


