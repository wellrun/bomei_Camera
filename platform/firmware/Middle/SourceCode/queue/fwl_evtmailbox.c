/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºfwl_evtmailbox.c
 * Function£ºdefined some function to handle event queue
 *
 * Author£ºXC
 * Date£º2004-07-12
 * Version£º1.0		  
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "fwl_vme.h"
#include <stdio.h>
#include "Gbl_MacroDef.h"
#include "Fwl_EvtMailBox.h"
#include "Eng_Debug.h"
#include "akos_api.h"
#include "arch_interrupt.h"

#ifdef OS_WIN32
#include "winvme.h"
#endif

#define MB_MAX_MSG_NUMBER       32
#define MB_MAX_INPUT_EVT_NUMBER 8


/*===========================================================================*/
volatile vINT32	g_SysMsgQueueHead = 0;
volatile vINT32	g_SysMsgQueueTail = 0;
volatile vINT32	g_InputSysEvtQueueHead = 0;
volatile vINT32	g_InputSysEvtQueueTail = 0;
volatile T_SYS_MAILBOX g_MB_SysMessageQueue[MB_MAX_MSG_NUMBER];
volatile T_SYS_MAILBOX g_MB_Input_SysEventQueue[MB_MAX_INPUT_EVT_NUMBER];
/*===========================================================================*/


/////////////////////////////////////////////////////////////////////////////////////////////////////
void MB_Init(void)
{
	int	i;

	g_SysMsgQueueHead = 0;
    g_SysMsgQueueTail = 0;

	g_InputSysEvtQueueHead = 0;
    g_InputSysEvtQueueTail = 0;

	for( i=0; i<MB_MAX_MSG_NUMBER; i++ )
	{
		g_MB_SysMessageQueue[i].param.w.Param1 = g_MB_SysMessageQueue[i].param.w.Param2 = 0;
	}

	for( i=0; i<MB_MAX_INPUT_EVT_NUMBER; i++ )
	{
		g_MB_Input_SysEventQueue[i].param.w.Param1 = g_MB_Input_SysEventQueue[i].param.w.Param2 = 0;
	}

	return;
}



vBOOL MB_PeekEvent(T_SYS_MAILBOX *mailbox, T_BOOL bIsInputSysEvent)
{
    if (bIsInputSysEvent)
    {
        if( g_InputSysEvtQueueHead == g_InputSysEvtQueueTail )
    	{
    		return vFALSE;
    	}
#ifdef OS_ANYKA
        store_all_int();
#endif        
        *mailbox = g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead];
#ifdef OS_ANYKA
        restore_all_int();
#endif

    }
    else
    {        
        if( g_SysMsgQueueHead == g_SysMsgQueueTail )
    	{
    		return vFALSE;
    	}
#ifdef OS_ANYKA
       store_all_int();
#endif        
        *mailbox = g_MB_SysMessageQueue[g_SysMsgQueueHead];
#ifdef OS_ANYKA
        restore_all_int();
#endif
    }

    return vTRUE;
}


vBOOL MB_DeleteEvent(T_BOOL bIsInputSysEvent)
{
    if (bIsInputSysEvent)
    {
        if( g_InputSysEvtQueueHead == g_InputSysEvtQueueTail )
    	{
    		return vFALSE;
    	}
#ifdef OS_ANYKA
        store_all_int();
#endif        
        g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead].param.w.Param1 = g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead].param.w.Param2 = 0;
        g_InputSysEvtQueueHead++;
        g_InputSysEvtQueueHead &= ( MB_MAX_INPUT_EVT_NUMBER - 1 );
#ifdef OS_ANYKA
        restore_all_int();
#endif

    }
    else
    {        
        if( g_SysMsgQueueHead == g_SysMsgQueueTail )
    	{
    		return vFALSE;
    	}
#ifdef OS_ANYKA
       store_all_int();
#endif        
        g_MB_SysMessageQueue[g_SysMsgQueueHead].param.w.Param1 = g_MB_SysMessageQueue[g_SysMsgQueueHead].param.w.Param2 = 0;
        g_SysMsgQueueHead++;
        g_SysMsgQueueHead &= ( MB_MAX_MSG_NUMBER - 1 );
#ifdef OS_ANYKA
        restore_all_int();
#endif
    }

    return vTRUE;
}


vBOOL MB_GetEvent( T_SYS_MAILBOX *mailbox, T_BOOL bIsInputSysEvent)
{
    vBOOL ret = AK_FALSE;

    if (bIsInputSysEvent)
    {
#ifdef OS_ANYKA
        store_all_int();
#endif
        if( g_InputSysEvtQueueHead != g_InputSysEvtQueueTail )
        {
            *mailbox = g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead];
            g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead].param.w.Param1 = g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead].param.w.Param2 = 0;
            g_InputSysEvtQueueHead++;
            g_InputSysEvtQueueHead &= ( MB_MAX_INPUT_EVT_NUMBER - 1 );
            
            ret = AK_TRUE;
        }

#ifdef OS_ANYKA
        restore_all_int();
#endif
    }
    else
    {        
#ifdef OS_ANYKA
        store_all_int();
#endif
        if( g_SysMsgQueueHead != g_SysMsgQueueTail )
        {
            *mailbox = g_MB_SysMessageQueue[g_SysMsgQueueHead];
            g_MB_SysMessageQueue[g_SysMsgQueueHead].param.w.Param1 = g_MB_SysMessageQueue[g_SysMsgQueueHead].param.w.Param2 = 0;
            g_SysMsgQueueHead++;
            g_SysMsgQueueHead &= ( MB_MAX_MSG_NUMBER - 1 );
            
            ret = AK_TRUE;
        }
#ifdef OS_ANYKA
        restore_all_int();
#endif
    }

    return ret;	
}

#ifdef OS_ANYKA
//#pragma Otime
#endif
vBOOL MB_PushEvent( T_SYS_MAILBOX* mailbox, T_pfnEvtCmp compareFun, 
                        T_BOOL bIsUnique, T_BOOL bIsHead, T_BOOL bIsInputSysEvent)
{
    int	i;

    if (AK_NULL == mailbox)
    {
        return vFALSE;
    }
#ifdef OS_ANYKA
    store_all_int();
#endif

    if (bIsInputSysEvent)
    {
        if (bIsUnique && AK_NULL != compareFun)
        {            
            for(i=g_InputSysEvtQueueHead; i!=g_InputSysEvtQueueTail; i=(i+1)&( MB_MAX_INPUT_EVT_NUMBER - 1 ))
        	{
        	    if(compareFun(mailbox,(T_VOID*)&g_MB_Input_SysEventQueue[i]))
				{
				    memcpy((T_pVOID)&g_MB_Input_SysEventQueue[i], mailbox, sizeof(T_SYS_MAILBOX));
					goto PUSH_EVENT_ERROR_EXIT;
				}
        	}	       
        }

        if (bIsHead)
        {
            if( g_InputSysEvtQueueHead == 0 )
        	{
        		g_InputSysEvtQueueHead = MB_MAX_INPUT_EVT_NUMBER - 1;
        	}
        	else
        	{
        		g_InputSysEvtQueueHead--;
        	}
            g_MB_Input_SysEventQueue[g_InputSysEvtQueueHead] = *mailbox;
            
            if( g_InputSysEvtQueueTail == g_InputSysEvtQueueHead )
        	{
        		if( g_InputSysEvtQueueTail == 0 )
        		{
        			g_InputSysEvtQueueTail = MB_MAX_INPUT_EVT_NUMBER - 1;
        		}
        		else
        		{
        			g_InputSysEvtQueueTail--;
        		}
        	}
        }
        else
        {
            if( ((g_InputSysEvtQueueTail + 1) & ( MB_MAX_INPUT_EVT_NUMBER - 1 )) == g_InputSysEvtQueueHead )
        	{
        		goto PUSH_EVENT_ERROR_EXIT;
        	}
            
            g_MB_Input_SysEventQueue[g_InputSysEvtQueueTail] = *mailbox;
        	g_InputSysEvtQueueTail++;
        	g_InputSysEvtQueueTail &= ( MB_MAX_INPUT_EVT_NUMBER - 1 );
        }
    }
    else
    {
        if (bIsUnique && AK_NULL != compareFun)
        {            
            for(i=g_SysMsgQueueHead; i!=g_SysMsgQueueTail; i=(i+1)&( MB_MAX_MSG_NUMBER - 1 ))
        	{
        	    if(compareFun(mailbox, (T_VOID*)&g_MB_SysMessageQueue[i]))
				{
				    memcpy((T_pVOID)&g_MB_SysMessageQueue[i], mailbox, sizeof(T_SYS_MAILBOX));
					goto PUSH_EVENT_ERROR_EXIT;
				}
        	}	   	
        }

        if (bIsHead)
        {
            if( g_SysMsgQueueHead == 0 )
        	{
        		g_SysMsgQueueHead = MB_MAX_MSG_NUMBER - 1;
        	}
        	else
        	{
        		g_SysMsgQueueHead--;
        	}
            g_MB_SysMessageQueue[g_SysMsgQueueHead] = *mailbox;
            
            if( g_SysMsgQueueTail == g_SysMsgQueueHead )
        	{
        		if( g_SysMsgQueueTail == 0 )
        		{
        			g_SysMsgQueueTail = MB_MAX_MSG_NUMBER - 1;
        		}
        		else
        		{
        			g_SysMsgQueueTail--;
        		}
        	}
        }
        else
        {
            if( ((g_SysMsgQueueTail + 1) & ( MB_MAX_MSG_NUMBER - 1 )) == g_SysMsgQueueHead )
        	{
        		goto PUSH_EVENT_ERROR_EXIT;
        	}
            
            g_MB_SysMessageQueue[g_SysMsgQueueTail] = *mailbox;
        	g_SysMsgQueueTail++;
        	g_SysMsgQueueTail &= ( MB_MAX_MSG_NUMBER - 1 );
        }
    }
#ifdef OS_WIN32
	winvme_ScheduleVMEEngine();
#endif

#ifdef OS_ANYKA
    restore_all_int();
#endif
	return vTRUE;

PUSH_EVENT_ERROR_EXIT: 
#ifdef OS_ANYKA
    restore_all_int();
#endif
    return vFALSE;
}
#ifdef OS_ANYKA
//#pragma no_Otime
#endif


#ifdef OS_ANYKA

vVOID VME_ReTriggerEvent(vT_EvtSubCode event, vUINT32 param)
{
    T_SYS_MAILBOX mailbox;

    mailbox.event = event;
    mailbox.param.lParam = param;
    AK_PostEventToHead( &mailbox );
}


vVOID VME_ReTriggerUniqueEvent(vT_EvtSubCode event, vUINT32 param)
{
    T_SYS_MAILBOX mailbox;

    mailbox.event = event;
    mailbox.param.lParam = param;
    AK_PostUniqueEventToHead( &mailbox, AK_NULL);
}

#if 0
/** struction type definition of event mailbox*/
typedef struct
{
	/** event code*/
	vT_EvtCode event;
	/** event parameter*/
	vT_EvtParam param;
}T_EVENTMAILBOX;


vBOOL PushEventMailbox( T_EVENTMAILBOX* mailbox )
{
    T_S32 ret;
    T_SYS_MAILBOX t_mailbox;

    t_mailbox.event = ((T_U32)mailbox->event & 0xffff);
    memcpy(&(t_mailbox.param), &(mailbox->param), sizeof(T_SYS_PARAM));
    ret = AK_PostEvent( &t_mailbox );

    return ((ret == AK_SUCCESS) ? AK_TRUE : AK_FALSE);
}

vBOOL PushEventMailboxToHead( T_EVENTMAILBOX* mailbox )
{
    T_S32 ret;
    T_SYS_MAILBOX t_mailbox;

    t_mailbox.event = ((T_U32)mailbox->event & 0xffff);
    memcpy(&(t_mailbox.param), &(mailbox->param), sizeof(T_SYS_PARAM));
    ret = AK_PostEventToHead( &t_mailbox );

    return ((ret == AK_SUCCESS) ? AK_TRUE : AK_FALSE);
}
#endif // #if 0


#endif
