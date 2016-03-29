#ifdef   OS_WIN32
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include "akos_api.h"

#define         HISR_ID             0x51554557UL

typedef struct
{
	void* task_ptr;
	T_OPTION priority;
	T_VOID *stack_address;
	T_U32 stack_size;
    T_U32 hisr_id;
} AK_HISR;

typedef struct
{
    void* lisr_entry;
    T_U32 vector;
} AK_LISR;

AK_LISR L_GPIO = {AK_NULL, 0};
AK_LISR L_UART = {AK_NULL, 0};

T_hHisr AK_Create_HISR(T_VOID (*hisr_entry)(T_VOID), T_U8 *name, T_OPTION priority,
                         T_VOID *stack_address, T_U32 stack_size)
{
	AK_HISR *hisr;
    hisr = malloc(sizeof(AK_HISR));
    if(AK_NULL == hisr)
        return AK_MEMORY_CORRUPT;
    if ( priority < 0 || priority > 2 )
        return AK_INVALID_PRIORITY;
    hisr->task_ptr      =   hisr_entry;
    hisr->priority      =   priority;
    hisr->stack_address =   stack_address;
    hisr->stack_size    =   stack_size;
    hisr->hisr_id            =   HISR_ID;
    return (T_hHisr)hisr;
}

T_S32 AK_Delete_HISR(T_hHisr hisr)
{
    AK_HISR *temp;
    temp = (AK_HISR*)hisr;
    if ((AK_NULL == (void*)hisr) || (temp->hisr_id != HISR_ID))
        return AK_INVALID_HISR;

    free(temp);
    temp = AK_NULL;

    return AK_SUCCESS;
}

T_S32 AK_Activate_HISR(T_hHisr hisr)
{
	AK_HISR *temp;
	T_S32 (*fn)();

	temp = (AK_HISR*)hisr;
	if(temp != AK_NULL && temp->hisr_id == HISR_ID)
	{
		(T_S32*)fn = (void*)temp->task_ptr;
		return fn();
	}
	else
		return AK_INVALID_HISR;
/*	
    T_U32 CreationFlag;
    T_S32 task;
    AK_HISR *temp;
    T_U32 new_priority;
    
    temp = (AK_HISR*)hisr;
    CreationFlag = CREATE_SUSPENDED;
    
    task = (T_hTask)CreateThread( 
                        AK_NULL,                     // no security attributes 
                        temp->stack_size,            // use specified stack size  
                        temp->task_ptr,              // thread function 
                        AK_NULL,                     // argument to thread function 
                        CreationFlag,                // use default creation flags 
                        AK_NULL);                    // no thread identifier?

    //if ( 0 == temp->priority )
        new_priority = 0; // THREAD_PRIORITY_TIME_CRITICAL;
    //else
      //  new_priority = 1; // THREAD_PRIORITY_HIGHEST;
    AK_Change_Priority(task, (T_U8)new_priority);

    AK_Resume_Task(task);
	AK_Sleep(1);
	return AK_SUCCESS;
	*/
}

T_S32 AK_Register_LISR(T_S32 vector, T_VOID (*list_entry)(T_S32), T_VOID (**old_lisr)(T_S32))
{
    if (vector != 9 || vector != 6)
        return AK_INVALID_VECTOR;

    *old_lisr = L_GPIO.lisr_entry;

    L_GPIO.vector = vector;
    L_GPIO.lisr_entry = list_entry;

	return AK_SUCCESS;
}

#endif
