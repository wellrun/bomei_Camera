#ifdef   OS_WIN32
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include "akos_api.h"
#include "anyka_types.h"
#include "eng_debug.h"
#ifdef WIN32

//typedef DWORD WINAPI (*ThreadFunc)(LPVOID lpParam );

T_hTask AK_Create_Task(T_VOID *task_entry,T_U8 *name,T_U32 argc, T_VOID *argv,
                        T_VOID *stack_address, T_U32 stack_size, 
                        T_OPTION priority, T_U32 time_slice, 
                        T_OPTION preempt, T_OPTION auto_start)
{
	T_U32 CreationFlag;
    T_S32 task;

    if (auto_start == AK_NO_START)
    	CreationFlag = CREATE_SUSPENDED;
	else if(AK_START == auto_start)
    	CreationFlag = 0;
    else
        return AK_INVALID_START;
    
    task = (T_hTask)CreateThread( 
                        NULL,                        // no security attributes 
                        stack_size,                  // use specified stack size  
                        task_entry,                  // thread function 
                        argv,                // argument to thread function 
                        CreationFlag,                // use default creation flags 
                        NULL);                       // no thread identifier?

	AK_Change_Priority((T_hTask)task, priority);

	if (NULL == (T_VOID*)task)
    	return AK_EFAILED;
	else
    	return task;
}

T_S32 AK_Delete_Task(T_hTask task)
{
    if(CloseHandle((HANDLE)task))
    {
        return AK_SUCCESS;
    }
    else
        return AK_INVALID_TASK;
}

T_S32 AK_Suspend_Task(T_hTask task)
{
    return SuspendThread((HANDLE)task);
}

T_S32 AK_Resume_Task(T_hTask task)
{
	return ResumeThread((HANDLE)task);
}

T_S32 AK_Terminate_Task(T_hTask task)
{
    DWORD lpExitCode;
    
    if (GetExitCodeThread((HANDLE)task, &lpExitCode))
    {
        if (TerminateThread((HANDLE)task, lpExitCode))
            return AK_SUCCESS;
        else
            return AK_INVALID_TASK;
    }
    else// fail to get exit code
    {
        return AK_INVALID_TASK;
    }
}

T_S32 AK_Task_Information(T_hTask task, T_CHR *name, T_OPTION *task_status, T_U32 *scheduled_count,
                          T_OPTION *priority, T_OPTION *preempt, T_U32 *time_slice,
                          T_VOID **stack_base, T_U32 *stack_size, T_U32 *minimum_stack)
{
    return AK_SUCCESS;
}

T_VOID AK_Sleep(T_U32 ticks)
{
    Sleep(ticks);
}

T_VOID AK_Relinquish(T_VOID)
{
    Sleep(0);
}
T_S32 AK_Reset_Task(T_hTask task, T_U32 argc, T_VOID *argv)
{
	return AK_SUCCESS;
}

T_S32 AK_Task_Status(T_hTask task)
{
    return AK_SUCCESS;
}

T_U32 AK_Check_Task_Stack(T_VOID)
{
	return 0;
}

T_OPTION AK_Change_Priority(T_hTask task, T_OPTION new_priority)
{
    T_S32 npriority;
 
    if ((new_priority<0) || (new_priority >255))
        return AK_INVALID_PRIORITY;
    else if(new_priority < 35)
        npriority = THREAD_PRIORITY_TIME_CRITICAL;
    else if(new_priority < 55)
        npriority = THREAD_PRIORITY_HIGHEST;
    else if(new_priority < 75)
        npriority = THREAD_PRIORITY_ABOVE_NORMAL;
    else if(new_priority < 95)
        npriority = THREAD_PRIORITY_NORMAL;
    else if(new_priority < 110)
        npriority = THREAD_PRIORITY_BELOW_NORMAL;
    else if(new_priority < 200)
        npriority = THREAD_PRIORITY_LOWEST;
    else 
        npriority = THREAD_PRIORITY_IDLE;

    if (SetThreadPriority((HANDLE)task, npriority))
        return AK_SUCCESS;
    else
        return AK_INVALID_TASK;
}

// Semaphore API
T_hSemaphore AK_Create_Semaphore(T_U32 initial_count, T_OPTION suspend_type)
{
    T_hSemaphore smph;
    smph = (T_hSemaphore)CreateSemaphore(NULL,
                                   initial_count,
                                   initial_count,
                                   NULL);
    if (AK_NULL == (void*)smph)
        return AK_INVALID_SEMAPHORE;
    else
        return smph;
    
}

T_S32 AK_Delete_Semaphore(T_hSemaphore semaphore)
{
    if (CloseHandle((HANDLE)semaphore))
        return AK_SUCCESS;
    else
        return AK_INVALID_SEMAPHORE;
}

T_S32 AK_Obtain_Semaphore(T_hSemaphore semaphore, T_U32 suspend)
{
    DWORD dwMilliseconds;
    DWORD return_value;
    if (suspend < 0)
        return AK_INVALID_SUSPEND;
    else if (suspend < AK_SUSPEND)
        dwMilliseconds = suspend;
    else
        dwMilliseconds = INFINITE;
    
    return_value = WaitForSingleObject( (HANDLE)semaphore, dwMilliseconds);

    if (WAIT_OBJECT_0 == return_value)
        return AK_SUCCESS;
    else if (WAIT_TIMEOUT == return_value)
        return AK_TIMEOUT;
    else if (WAIT_ABANDONED == return_value)
        return AK_SEMAPHORE_DELETED;
    else
		return return_value;
}

T_S32 AK_Release_Semaphore(T_hSemaphore semaphore)
{
    if (ReleaseSemaphore((HANDLE)semaphore, 1, NULL))
    {
        return AK_SUCCESS;
    }
    else
    {
        return AK_INVALID_SEMAPHORE;
    }
}

T_S32 AK_Reset_Semaphore(T_hSemaphore semaphore, T_U32 initial_count)
{
	T_U32 i = 0;
	for(i=0; i<initial_count; i++)
		AK_Release_Semaphore(semaphore);
	return AK_SUCCESS;
}
/*
// Timer API
typedef struct WTimer
{
    HANDLE Queue;
    HANDLE QueueTimer;
}
WinTimer;

T_hTimer AK_Create_Timer(T_VOID (*expiration_routine)(T_U32), T_U32 id, 
                    T_U32 initial_time, T_U32 reschedule_time, T_OPTION enable)
{
    WinTimer *timer;

    timer = (WinTimer*)malloc(sizeof(WinTimer));
    if(NULL == timer)
        return AK_MEMORY_CORRUPT;
    
    timer->Queue = (T_VOID*)CreateTimerQueue();
    if (CreateTimerQueueTimer(&(timer->QueueTimer), timer->Queue, 
                        expiration_routine, NULL, initial_time, 
                        reschedule_time, 1))
        return (T_hTimer)timer;
    else
        return AK_INVALID_FUNCTION;
}

T_S32 AK_Delete_Timer(T_hTimer timer_hdl)
{
	WinTimer *timer;
	timer = (WinTimer*)timer_hdl;
    if (DeleteTimerQueueTimer(timer->Queue, timer->QueueTimer, NULL))
    {
        if (DeleteTimerQueue(timer->Queue))
            return AK_SUCCESS;
        else
            return AK_EFAILED;
    }
    else
    {
        return AK_EFAILED;
    }
    free((T_VOID*)timer);
}
*/
/*
T_hTimer AK_Create_Timer(T_VOID (*expiration_routine)(T_U32), T_U32 id, 
                    T_U32 initial_time, T_U32 reschedule_time, T_OPTION enable)
{
	return 0;
}

T_S32 AK_Delete_Timer(T_hTimer timer_hdl)
{
	return 0;
}
*/

typedef struct EvtGp
{
    HANDLE event[32];
    T_U32 actual_flags;
}
AKEvtGp;

T_hEventGroup AK_Create_Event_Group(T_VOID)
{
    AKEvtGp *evt;
    T_U32 i;
    // malloc the memory for the structure
    evt = (AKEvtGp*)malloc(sizeof(AKEvtGp)); 
    if(AK_NULL == evt)
        return AK_MEMORY_CORRUPT;
    
    memset((T_VOID*)evt, 0, sizeof(AKEvtGp));

    // initial the structure
    for (i=0; i<32; i++)
    {
        evt->event[i] = CreateEvent(AK_NULL, 1, 0, AK_NULL);
    }
    evt->actual_flags = 0;

    return (T_hEventGroup)evt;        

}

T_S32 AK_Delete_Event_Group(T_hEventGroup eventgroup_hdl)
{
	AKEvtGp *eventgroup;
	T_S32 state;
    T_U32 i;
	eventgroup = (AKEvtGp*)eventgroup_hdl;
    
	for(i=0; i<32; i++)
    {
        // delete events one by one
        if (CloseHandle(eventgroup->event[i]))
        {//success
			state = AK_SUCCESS;
        }
        else
        // error tips
        {
            // get last error and define return value
			Fwl_Print(C2, M_AKFRAME, "Error occur when delete eventgroup\r\n");
			state = AK_INVALID_GROUP;
        }
    }
    free(eventgroup);
	return state;
}

T_S32 AK_Set_Events(T_hEventGroup eventgroup_hdl, T_U32 event_flags, T_OPTION operation)
{
	AKEvtGp *eventgroup;
    T_U32 temp_flag, i, mask;
	eventgroup = (AKEvtGp*)eventgroup_hdl;
    mask = 0x01;
    if (AK_AND == operation)
    {
        temp_flag = eventgroup->actual_flags & event_flags;
    }
    else if (AK_OR == operation)
    {
        temp_flag = eventgroup->actual_flags | event_flags;
    }
    else
    {
        //error tips, and return error code
		Fwl_Print(C2, M_AKFRAME, "Wrong opration");
		return AK_INVALID_OPERATION;
    }
    // 
    for(i=0; i<32; i++)
    {
        if(!ResetEvent(eventgroup->event[i]))
        {
            // error tips
            Fwl_Print(C2, M_AKFRAME, "reset event error!\r\n");
            return AK_INVALID_GROUP;
        }
        // set the event that need to be set
        if (temp_flag & mask)
        {
            SetEvent(eventgroup->event[i]);
        }
        //left shift 1
        mask*=2;
    }
    eventgroup->actual_flags = event_flags;
	return AK_SUCCESS;
}

T_S32 AK_Retrieve_Events(T_hEventGroup eventgroup_hdl, T_U32 requested_events, 
                        T_OPTION operation, T_U32 *retrieved_events, T_U32 suspend)
{
	AKEvtGp *eventgroup;
    HANDLE rq_events[32];
    T_U32 imme_rq_events, i, event_count, mask, ms;//imme_rq_events:request events for immediately use
    T_S32 state;
    T_S32 return_value;
    BOOL waitall;

	eventgroup = (AKEvtGp*)eventgroup_hdl;

	/* Event group pointer is invalid, indicate in completion status.  */
    if (eventgroup == AK_NULL)
        return AK_INVALID_GROUP;
    
	// get request events, for check
    else if (retrieved_events == AK_NULL)
    
		/* Retrieved events pointer is NULL.  */
        return AK_INVALID_POINTER;

    if (AK_OR == operation || AK_OR_CONSUME == operation)
    {
        imme_rq_events = eventgroup->actual_flags | requested_events;
        waitall = FALSE;
    }
    else if (AK_AND == operation || AK_AND_CONSUME == operation)
    {
        waitall = TRUE;

        //check if the AND request 申请的在已有的之外
        //比如actual:0110, request: 1010, 通过if 操作后第一位不为0
        if (~(eventgroup->actual_flags | ~requested_events))
        {
            imme_rq_events = 0;
        }
        else
        {
            imme_rq_events = eventgroup->actual_flags & requested_events;
        }
    }
    else
    {
        
		//error tips, and return error code
        return AK_INVALID_OPERATION;
    }

    if(0 == suspend)
    {
        /*get the number of events and the queue of events, for immediate check*/
        //initiate
        // Fwl_Print(C2, M_AKFRAME, "Now suspend=0, deal with immediately check!\r\n");
        event_count = 0;
        mask = 1;
        for(i=0; i<32; i++)
        {
            if(imme_rq_events & mask)
            {
                rq_events[event_count] = eventgroup->event[i];
                event_count++;
            }
            mask*=2;
        }

        if (imme_rq_events)
        {
            //reset the consumed event!
            if (AK_AND_CONSUME == operation || AK_OR_CONSUME == operation)
            {
                    
                for(i=0; i<event_count; i++)
                {
                    if(!ResetEvent(rq_events[i]))
                        Fwl_Print(C2, M_AKFRAME, "reset event error!\r\n");
                }
            }
            *retrieved_events = eventgroup->actual_flags;
            eventgroup->actual_flags = eventgroup->actual_flags & ~requested_events;
            return AK_SUCCESS;
        }
        else
        {
            *retrieved_events = eventgroup->actual_flags;
			return AK_NOT_PRESENT;
        }
    }
    else if (AK_SUSPEND == suspend)
    {
        ms = INFINITE;
    }
    else
        ms = suspend;

    /*get the number of events and the queue of events, for waitfor multipleobjects*/
    //initiate
    event_count = 0;
    mask = 1;
	return_value = AK_SUCCESS;
    for (i=0; i<32; i++)
    {
        if (requested_events & mask)
        {
            rq_events[event_count] = eventgroup->event[i];
            event_count++;
        }
        mask*=2;
    }

    state = WaitForMultipleObjects(event_count, rq_events, waitall, ms);

    if (WAIT_TIMEOUT == state)
    {
        *retrieved_events = eventgroup->actual_flags;
        return_value = AK_TIMEOUT;
    }
    else if (WAIT_FAILED == state)//the reason of failing is unknow, hope never got this
    {
        Fwl_Print(C2, M_AKFRAME, "Failed 'cos unknow reason!\r\n");
        return_value = AK_INVALID_GROUP;
    }
    else if (state >= WAIT_ABANDONED_0)
    {
        Fwl_Print(C2, M_AKFRAME, "What hell is here? Abandened?\r\n");
        return_value = AK_SUCCESS;
    }
    else if (state >= WAIT_OBJECT_0)
    {
        // Fwl_Print(C2, M_AKFRAME, "get signal success!\r\n");
        return_value = AK_SUCCESS;
    }

    if (AK_SUCCESS == return_value &&
        (AK_AND_CONSUME == operation || AK_OR_CONSUME == operation))
    {
        // set the actual flag to actual events
        eventgroup->actual_flags = eventgroup->actual_flags & ~requested_events;
        for(i=0; i<event_count; i++)
        {
            if(!ResetEvent(rq_events[i]))
                Fwl_Print(C2, M_AKFRAME, "reset event error!\r\n");
        }
    }
   
   *retrieved_events = eventgroup->actual_flags;    

    return return_value;
}

/* system protect fuction */
T_VOID AK_System_Protect(T_VOID)
{
}
T_VOID AK_System_Unprotect(T_VOID)
{
}

T_hTask AK_GetCurrent_Task(T_VOID)
{
	return (T_hTask)GetCurrentThread();
}

#endif
#endif
// END of File
