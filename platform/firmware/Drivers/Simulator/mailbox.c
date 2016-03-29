#ifdef OS_WIN32
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include "akos_api.h"
#include "anyka_types.h"
#include "Queue.h"
#include "mailbox.h"


extern T_VOID    CSC_Place_On_List(CS_NODE **head, CS_NODE *new_node);
extern T_VOID    CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node);
extern T_VOID    CSC_Remove_From_List(CS_NODE **head, CS_NODE *node);

T_hMailbox AK_Create_Mailbox(T_OPTION suspend_type)
{
    T_S32 status, mailbox;
    
    mailbox = (T_hMailbox)malloc(sizeof(NU_MAILBOX));
    if (AK_NULL == (T_VOID*)mailbox)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)mailbox, 0, sizeof(NU_MAILBOX));
    status = MBCE_Create_Mailbox((NU_MAILBOX*)mailbox, "Mailbox", suspend_type);

    if(AK_SUCCESS == status)
    {
        return mailbox;
    }
	free((T_VOID*)mailbox);
	mailbox = (T_hMailbox)AK_NULL;
    return status;
}

T_S32 AK_Delete_Mailbox(T_hMailbox mailbox)
{
	T_S32 status;

	status = MBCE_Delete_Mailbox((NU_MAILBOX*)mailbox);
    if(AK_SUCCESS == status)
    {
        free((T_VOID*)mailbox);
		mailbox = (T_hMailbox)AK_NULL;
    }

    return status;
}


T_S32 AK_Broadcast_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return MBSE_Broadcast_To_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_S32 AK_Receive_From_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return MBCE_Receive_From_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_S32 AK_Send_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return MBCE_Send_To_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_S32  MBCE_Create_Mailbox(NU_MAILBOX *mailbox_ptr, T_U8 *name, 
                                                T_OPTION suspend_type)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
T_S32          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Check for a NULL mailbox pointer or an already created mailbox.  */
    if ((mailbox == AK_NULL) || (mailbox -> mb_id == MB_MAILBOX_ID))
    
        /* Invalid mailbox control block pointer.  */
        status =  AK_INVALID_MAILBOX;
        
    else if ((suspend_type != AK_FIFO) && (suspend_type != AK_PRIORITY))
    
        /* Invalid suspension type.  */
        status =  AK_INVALID_SUSPEND;
        
    else
    
        /* Call the actual service to create the mailbox.  */
        status =  MBC_Create_Mailbox(mailbox_ptr, name, suspend_type);
        
    /* Return completion status.  */
    return(status);
}

T_S32  MBC_Create_Mailbox(NU_MAILBOX *mailbox_ptr, T_U8 *name,
                                                T_OPTION suspend_type)
{

MB_MCB      *mailbox;                    /* Mailbox control block ptr */
T_S32             i;                          /* Working index variable    */

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* First, clear the mailbox ID just in case it is an old Mailbox
       Control Block.  */
    mailbox -> mb_id =             0;

    /* Fill in the mailbox name.  */
    for (i = 0; i < AK_MAX_NAME; i++)
        mailbox -> mb_name[i] =  name[i];

    /* Clear the message present flag- initial it to "no message present." */
    mailbox -> mb_message_present =  AK_FALSE;

    /* Setup the mailbox suspension type.  */
    if (suspend_type == AK_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        mailbox -> mb_fifo_suspend =  AK_TRUE;
    else

        /* Priority suspension is selected.  */
        mailbox -> mb_fifo_suspend =  AK_FALSE;

    /* Clear the suspension list pointer.  */
    mailbox -> mb_suspension_list =  AK_NULL;

    /* Clear the number of tasks waiting on the mailbox counter.  */
    mailbox -> mb_tasks_waiting =  0;

    /* Initialize link pointers.  */
    mailbox -> mb_created.cs_previous =    AK_NULL;
    mailbox -> mb_created.cs_next =        AK_NULL;

    /* At this point the mailbox is completely built.  The ID can now be
       set and it can be linked into the created mailbox list.  */
    mailbox -> mb_id =                     MB_MAILBOX_ID;

    /* Return successful completion.  */
    return(AK_SUCCESS);
}

T_S32  MBCE_Delete_Mailbox(NU_MAILBOX *mailbox_ptr)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
T_S32          status;                     /* Completion status         */

    
    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if the mailbox pointer is valid.  */
    if ((mailbox) && (mailbox -> mb_id == MB_MAILBOX_ID))
    
        /* Mailbox pointer is valid, call function to delete it.  */
        status =  MBC_Delete_Mailbox(mailbox_ptr);
        
    else
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;
        
    /* Return completion status.  */
    return(status);
}

T_S32  MBC_Delete_Mailbox(NU_MAILBOX *mailbox_ptr)
{

MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
MB_SUSPEND     *next_ptr;                   /* Next suspend block pointer*/

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Clear the mailbox ID.  */
    mailbox -> mb_id =  0;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  mailbox -> mb_suspension_list;

    /* Walk the chain task(s) currently suspended on the mailbox.  */

    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_MAILBOX_DELETED.  */
        suspend_ptr -> mb_return_status =  AK_MAILBOX_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (MB_SUSPEND *) (suspend_ptr -> mb_suspend_link.cs_next);

        /* Resume the specified task.  */
        AK_Resume_Task((T_hTask) suspend_ptr -> mb_suspended_task);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == mailbox -> mb_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  AK_NULL;
        else

            /* Position the suspend pointer to the next suspend block.  */
            suspend_ptr =  next_ptr;

    }

    /* Return a successful completion.  */
    return(AK_SUCCESS);
}


T_S32  MBCE_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                          T_U32 suspend)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
T_S32          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if mailbox pointer is invalid.  */
    if (mailbox == AK_NULL)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else if (mailbox -> mb_id != MB_MAILBOX_ID)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else if (message == AK_NULL)
    
        /* Message pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_POINTER;

    else
    
        /* Parameters are valid, call actual function.  */
        status =  MBC_Send_To_Mailbox(mailbox_ptr, message, suspend);

    /* Return the completion status.  */
    return(status);
}

T_S32  MBC_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend)
{

MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
T_U32    *source_ptr;                 /* Pointer to source         */
T_U32    *destination_ptr;            /* Pointer to destination    */
HANDLE         *task;                       /* Task pointer              */
T_S32          status;                     /* Completion status         */

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Determine if the mailbox is empty or full.  */
    if (mailbox -> mb_message_present)
    {

        /* Mailbox already has a message.  Determine if suspension is
           required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            mailbox -> mb_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> mb_mailbox =                  mailbox;
            suspend_ptr -> mb_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> mb_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> mb_message_area =             (T_U32 *) message;
            /*task = (HANDLE *)  TCT_Current_Thread();*/
            DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), 
                            GetCurrentProcess(), (void**)&task, 0, TRUE, 
                            DUPLICATE_SAME_ACCESS);
            suspend_ptr -> mb_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               mailbox.  */
            if (mailbox -> mb_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this mailbox.  */
                CSC_Place_On_List((CS_NODE **) &(mailbox ->mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> mb_suspend_link.cs_priority =
                                                    GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(mailbox -> mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the mailbox.  */
            AK_Suspend_Task((T_hTask) task);

            /* Pickup the return status.  */
            status =  suspend_ptr -> mb_return_status;
        }
        else
        {

            /* Return a status of AK_MAILBOX_FULL because there is no
               room in the mailbox for the message.  */
            status =  AK_MAILBOX_FULL;
        }
    }
    else
    {

        /* Determine if a task is waiting on the mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

            /* Task is waiting on mailbox for a message.  */

            /* Decrement the number of tasks waiting on mailbox.  */
            mailbox -> mb_tasks_waiting--;

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  mailbox -> mb_suspension_list;
            CSC_Remove_From_List((CS_NODE **)
                &(mailbox -> mb_suspension_list),
                                &(suspend_ptr -> mb_suspend_link));

            /* Setup the source and destination pointers.  */
            source_ptr =       (T_U32 *) message;
            destination_ptr =  suspend_ptr -> mb_message_area;

            /* Copy the message directly into the waiting task's
               destination.  */
            *destination_ptr       =  *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Setup the appropriate return value.  */
            suspend_ptr -> mb_return_status =  AK_SUCCESS;

            /* Wakeup the waiting task and check for preemption.  */
            AK_Resume_Task((T_hTask) suspend_ptr -> mb_suspended_task);
        }
        else
        {

            /* Mailbox is empty and no task is waiting.  */

            /* Setup the source and destination pointers.  */
            source_ptr =       (T_U32 *) message;
            destination_ptr =  &(mailbox -> mb_message_area[0]);

            /* Place the message in the mailbox. */
            *destination_ptr =        *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Indicate that the mailbox has a message.  */
            mailbox -> mb_message_present =  AK_TRUE;
        }
    }
    /* Return the completion status.  */
    return(status);
}

T_S32  MBCE_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                        T_U32 suspend)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
T_S32          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if mailbox pointer is invalid.  */
    if (mailbox == AK_NULL)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else if (mailbox -> mb_id != MB_MAILBOX_ID)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else
    
        /* Parameters are valid, call actual function.  */
        status =  MBC_Receive_From_Mailbox(mailbox_ptr, message, suspend);

    /* Return the completion status.  */
    return(status);
}

T_S32  MBC_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend)
{

MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
T_U32    *source_ptr;                 /* Pointer to source         */
T_U32    *destination_ptr;            /* Pointer to destination    */
HANDLE         *task;                       /* Task pointer              */
T_S32          status;                     /* Completion status         */

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Determine if the mailbox is empty or full.  */
    if (mailbox -> mb_message_present)
    {

        /* Copy message from mailbox into the caller's area.  */

        /* Setup the source and destination pointers.  */
        source_ptr =       &(mailbox -> mb_message_area[0]);
        destination_ptr =  (T_U32 *) message;

        /* Copy the message directly into the waiting task's
           destination.  */
        *destination_ptr =        *source_ptr;
        *(destination_ptr + 1) =  *(source_ptr + 1);
        *(destination_ptr + 2) =  *(source_ptr + 2);
        *(destination_ptr + 3) =  *(source_ptr + 3);

        /* Determine if another task is waiting to place something into the
           mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

            /* Yes, another task is waiting to send something to the
               mailbox.  */

            /* Decrement the number of tasks waiting counter.  */
            mailbox -> mb_tasks_waiting--;

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  mailbox -> mb_suspension_list;
            CSC_Remove_From_List((CS_NODE **)
                &(mailbox -> mb_suspension_list),
                                &(suspend_ptr -> mb_suspend_link));

            /* Setup the source and destination pointers.  */
            source_ptr =       suspend_ptr -> mb_message_area;
            destination_ptr =  &(mailbox -> mb_message_area[0]);

            /* Copy the message directly into the waiting task's
               destination.  */
            *destination_ptr =        *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Setup the appropriate return value.  */
            suspend_ptr -> mb_return_status =  AK_SUCCESS;

            /* Resume the suspended task.  */
            AK_Resume_Task((T_hTask) suspend_ptr -> mb_suspended_task);

        }
        else
        {

            /* Clear the message present flag.  */
            mailbox -> mb_message_present =  AK_FALSE;
        }

    }
    else
    {

        /* Mailbox is empty.  Determine if suspension is required.  */
        if (suspend)
        {

            /* Suspension is required.  */

            /* Increment the number of tasks waiting on the mailbox counter. */
            mailbox -> mb_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> mb_mailbox =                  mailbox;
            suspend_ptr -> mb_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> mb_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> mb_message_area =             (T_U32 *) message;
            /*task = (HANDLE *) TCT_Current_Thread();*/
            DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), 
                            GetCurrentProcess(), (void**)&task, 0, TRUE, 
                            DUPLICATE_SAME_ACCESS);
            suspend_ptr -> mb_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               mailbox.  */
            if (mailbox -> mb_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this mailbox.  */
                CSC_Place_On_List((CS_NODE **) &(mailbox ->mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> mb_suspend_link.cs_priority =
                                                    GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(mailbox -> mb_suspension_list),
                                        &(suspend_block.mb_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the mailbox.  */
            AK_Suspend_Task((T_hTask) task);

            /* Pickup the return status.  */
            status =  suspend_ptr -> mb_return_status;
        }
        else
        {

            /* Return a status of AK_MAILBOX_EMPTY because there is
               nothing in the mailbox.  */
            status =  AK_MAILBOX_EMPTY;
        }

    }

    /* Return the completion status.  */
    return(status);
}

T_S32  MBSE_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                        T_U32 suspend)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
T_S32          status;                     /* Completion status         */



    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if mailbox pointer is invalid.  */
    if (mailbox == AK_NULL)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else if (mailbox -> mb_id != MB_MAILBOX_ID)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_MAILBOX;

    else if (message == AK_NULL)
    
        /* Message pointer is invalid, indicate in completion status.  */
        status =  AK_INVALID_POINTER;

    else
        
        /* Parameters are valid, call actual function.  */
        status =  MBS_Broadcast_To_Mailbox(mailbox_ptr, message, suspend);
    
    /* Return the completion status.  */
    return(status);
}

T_S32  MBS_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend)
{

MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
MB_SUSPEND     *suspend_head;               /* Pointer to suspend head   */
MB_SUSPEND     *next_suspend_ptr;           /* Get before restarting task*/
T_U32    *source_ptr;                 /* Pointer to source         */
T_U32    *destination_ptr;            /* Pointer to destination    */
HANDLE         *task;                       /* Task pointer              */
T_S32          status;                     /* Completion status         */

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Determine if the mailbox is empty or full.  */
    if (mailbox -> mb_message_present)
    {

        /* Mailbox already has a message.  Determine if suspension is
           required.  */
        if (suspend)
        {

            /* Suspension is requested.  */

            /* Increment the number of tasks suspended on the mailbox. */
            mailbox -> mb_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> mb_mailbox =                  mailbox;
            suspend_ptr -> mb_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> mb_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> mb_message_area =             (T_U32 *) message;
            /*task = (HANDLE *)  TCT_Current_Thread();*/
            DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), 
                            GetCurrentProcess(), (void**)&task, 0, TRUE, 
                            DUPLICATE_SAME_ACCESS);
            suspend_ptr -> mb_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               mailbox.  */
            if (mailbox -> mb_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this mailbox.  */
                CSC_Place_On_List((CS_NODE **) &(mailbox ->mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> mb_suspend_link.cs_priority =
                                                    GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(mailbox -> mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the mailbox.  */
            AK_Suspend_Task((T_hTask) task);

            /* Pickup the return status.  */
            status =  suspend_ptr -> mb_return_status;
        }
        else
        {

            /* Return a status of AK_MAILBOX_FULL because there is no
               room in the mailbox for the message.  */
            status =  AK_MAILBOX_FULL;
        }
    }
    else
    {

        /* Determine if a task is waiting on the mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

            /* At least one task is waiting on mailbox for a message.  */

            /* Save off the suspension list and and then clear out the
               mailbox suspension.  */
            suspend_head =  mailbox -> mb_suspension_list;
            mailbox -> mb_suspension_list =  AK_NULL;

            /* Loop to wakeup all of the tasks waiting on the mailbox for
               a message.  */
            suspend_ptr =  suspend_head;
            do
            {

                /* Setup the source and destination pointers.  */
                source_ptr =       (T_U32 *) message;
                destination_ptr =  suspend_ptr -> mb_message_area;

                /* Copy the message directly into the waiting task's
                   destination.  */
                *destination_ptr =        *source_ptr;
                *(destination_ptr + 1) =  *(source_ptr + 1);
                *(destination_ptr + 2) =  *(source_ptr + 2);
                *(destination_ptr + 3) =  *(source_ptr + 3);

                /* Setup the appropriate return value.  */
                suspend_ptr -> mb_return_status =  AK_SUCCESS;

                /* Move the suspend pointer along to the next block. */
                next_suspend_ptr =  (MB_SUSPEND *)
                                suspend_ptr -> mb_suspend_link.cs_next;

                /* Wakeup each task waiting.  */
                AK_Resume_Task((T_hTask) suspend_ptr -> mb_suspended_task);
                suspend_ptr = next_suspend_ptr;

            } while (suspend_ptr != suspend_head);

            /* Clear the number of tasks waiting counter of the mailbox.  */
            mailbox -> mb_tasks_waiting =  0;
        }
        else
        {

            /* Mailbox is empty and no task is waiting.  */

            /* Setup the source and destination pointers.  */
            source_ptr =       (T_U32 *) message;
            destination_ptr =  &(mailbox -> mb_message_area[0]);

            /* Place the message in the mailbox. */
            *destination_ptr =        *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Indicate that the mailbox has a message.  */
            mailbox -> mb_message_present =  AK_TRUE;
        }
    }

    /* Return the completion status.  */
    return(status);
}

#endif