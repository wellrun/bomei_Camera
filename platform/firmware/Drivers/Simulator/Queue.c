#ifdef OS_WIN32
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include "akos_api.h"
#include "anyka_types.h"
#include "Queue.h"

#ifdef WIN32

#define CBYTE(size)         ((size)/4)

T_VOID    CSC_Place_On_List(CS_NODE **head, CS_NODE *new_node)
{

    /* Determine if the list in non-empty.  */
    if (*head)
    {

        /* The list is not empty.  Add the new node to the end of
           the list.  */
        new_node -> cs_previous =               (*head) -> cs_previous;
        (new_node -> cs_previous) -> cs_next =  new_node;
        new_node -> cs_next =                   (*head);
        (new_node -> cs_next) -> cs_previous =  new_node;
    }
    else
    {

        /* The list is empty, setup the head and the new node.  */
        (*head) =  new_node;
        new_node -> cs_previous =  new_node;
        new_node -> cs_next =      new_node;
    }

}

T_VOID    CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node)
{

CS_NODE         *search_ptr;                /* List search pointer       */

    /* Determine if the list in non-empty.  */
    if (*head)
    {

        /* Search the list to find the proper place for the new node.  */
        search_ptr =  (*head);

        /* Check for insertion before the first node on the list.  */
        if (search_ptr -> cs_priority > new_node -> cs_priority)
        {

            /* Update the head pointer to point at the new node.  */
            (*head) =  new_node;
        }
        else
        {

            /* We know that the new node is not the highest priority and
               must be placed somewhere after the head pointer.  */

            /* Move search pointer up to the next node since we are trying
               to find the proper node to insert in front of. */
            search_ptr =  search_ptr -> cs_next;
            while ((search_ptr -> cs_priority <= new_node -> cs_priority) &&
                   (search_ptr != (*head)))
            {

                /* Move along to the next node.  */
                search_ptr =  search_ptr -> cs_next;
            }
        }

        /* Insert before search pointer.  */
        new_node -> cs_previous =               search_ptr -> cs_previous;
        (new_node -> cs_previous) -> cs_next =  new_node;
        new_node -> cs_next =                   search_ptr;
        (new_node -> cs_next) -> cs_previous =  new_node;
    }
    else
    {

        /* The list is empty, setup the head and the new node.  */
        (*head) =  new_node;
        new_node -> cs_previous =  new_node;
        new_node -> cs_next =      new_node;
    }

}

T_VOID    CSC_Remove_From_List(CS_NODE **head, CS_NODE *node)
{

    /* Determine if this is the only node in the system.  */
    if (node -> cs_previous == node)
    {

        /* Yes, this is the only node in the system.  Clear the node's
           pointers and the head pointer.  */
        (*head) =              AK_NULL;
    }
    else
    {

        /* Unlink the node from a multiple node list.  */
        (node -> cs_previous) -> cs_next =  node -> cs_next;
        (node -> cs_next) -> cs_previous =  node -> cs_previous;

        /* Check to see if the node to delete is at the head of the
           list. */
        if (node == *head)

            /* Move the head pointer to the node after.  */
            *head =  node -> cs_next;
    }

}
//create queue
T_hQueue AK_Create_Queue(T_VOID *start_address, T_U32 queue_size, 
                         T_OPTION message_type, T_U32 message_size, 
                         T_OPTION suspend_type)
{
    T_S32 status, queue;
	
	queue = (T_hQueue)malloc(sizeof(NU_QUEUE));
	if (AK_NULL == (T_pVOID*)queue)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)queue, 0, sizeof(NU_QUEUE));
	status = QUCE_Create_Queue((NU_QUEUE*)queue, "queue", start_address, 
                              CBYTE(queue_size), message_type, 
                              CBYTE(message_size), suspend_type);
    
    if (AK_SUCCESS == status)
    {
        return queue;
    }
    free((T_VOID*)queue);
    queue = 0;
    return status;
}


T_S32  QUCE_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name, 
                      T_VOID *start_address, T_U32 queue_size, 
                      T_OPTION message_type, T_U32 message_size,
                      T_OPTION suspend_type)
{

QU_QCB         *queue;
T_S32          status;
T_S32           overhead;
    
    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;
    
    /* Determine if queue supports variable length messages.  If so, an
       additional word of overhead is required.  */
    if (message_type == AK_VARIABLE_SIZE)
    
        /* Variable-size queues require an additional word of overhead.  */
        overhead =  1;
    else
    
        /* Fixed-size message queues require no additional overhead.  */
        overhead =  0;

    /* Determine if there is an error with the queue pointer.  */
    if ((queue == AK_NULL) || (queue -> qu_id == QU_QUEUE_ID))
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;
        
    else if (start_address == AK_NULL)
    
        /* Indicate that the starting address of the queue is invalid.  */
        status =  AK_INVALID_MEMORY;
        
    else if ((queue_size == 0) || (message_size == 0) || 
                                ((message_size+overhead) > queue_size))
             
        /* Indicate that one or both of the size parameters are invalid.  */
        status =  AK_INVALID_SIZE;
        
    else if ((message_type != AK_FIXED_SIZE) && 
                        (message_type != AK_VARIABLE_SIZE))
                                
        /* Indicate that the message type is invalid.  */
        status =  AK_INVALID_MESSAGE;
        
    else if ((suspend_type != AK_FIFO) && (suspend_type != AK_PRIORITY))
    
        /* Indicate that the suspend type is invalid.  */
        status =  AK_INVALID_SUSPEND;
        
    else
    
        /* All the parameters are okay, call the actual function to create
           a queue.  */
        status =  QUC_Create_Queue(queue_ptr, name, start_address, queue_size,
                                  message_type, message_size, suspend_type);
                                  
    /* Return completion status.  */
    return(status);
}

T_S32  QUC_Create_Queue(NU_QUEUE *queue_ptr, T_U8 *name,
                      T_VOID *start_address, T_U32 queue_size,
                      T_OPTION message_type, T_U32 message_size,
                      T_OPTION suspend_type)
{

    QU_QCB      *queue;                      /* Queue control block ptr   */
    INT          i;                          /* Working index variable    */
    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* First, clear the queue ID just in case it is an old Queue
       Control Block.  */
    queue -> qu_id =             0;

    /* Fill in the queue name.  */
    for (i = 0; i < AK_MAX_NAME; i++)
        queue -> qu_name[i] =  name[i];

    /* Setup the queue suspension type.  */
    if (suspend_type == AK_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        queue -> qu_fifo_suspend =  AK_TRUE;

    else

        /* Priority suspension is selected.  */
        queue -> qu_fifo_suspend =  AK_FALSE;

    /* Setup the queue message type.  */
    if (message_type == AK_FIXED_SIZE)

        /* Fixed-size messages are required.  */
        queue -> qu_fixed_size =  AK_TRUE;
    else

        /* Variable-size messages are required.  */
        queue -> qu_fixed_size =  AK_FALSE;

    /* Setup the message size.  */
    queue -> qu_message_size =  message_size;

    /* Clear the messages counter.   */
    queue -> qu_messages =  0;

    /* Setup the actual queue parameters.  */
    queue -> qu_queue_size =    queue_size;

    /* If the queue supports fixed-size messages, make sure that the queue
       size is an even multiple of the message size.  */
    if (queue -> qu_fixed_size)

        /* Adjust the area of the queue being used.  */
        queue_size =  (queue_size / message_size) * message_size;

    queue -> qu_available =     queue_size;
    queue -> qu_start =         (T_U32 *) start_address;
    queue -> qu_end =           queue -> qu_start + queue_size;
    queue -> qu_read =          (T_U32 *) start_address;
    queue -> qu_write =         (T_U32 *) start_address;

    /* Clear the suspension list pointer.  */
    queue -> qu_suspension_list =  AK_NULL;

    /* Clear the number of tasks waiting on the queue counter.  */
    queue -> qu_tasks_waiting =  0;

    /* Clear the urgent message list pointer.  */
    queue -> qu_urgent_list =  AK_NULL;

    /* Initialize link pointers.  */
    queue -> qu_created.cs_previous =    AK_NULL;
    queue -> qu_created.cs_next =        AK_NULL;

    /* Protect against access to the list of created queues.  */
//    TCT_Protect(&QUD_List_Protect);

    /* At this point the queue is completely built.  The ID can now be
       set and it can be linked into the created queue list.  */
    queue -> qu_id =                     QU_QUEUE_ID;

    /* Link the queue into the list of created queues and increment the
       total number of queues in the system.  */
//    CSC_Place_On_List(&QUD_Created_Queues_List, &(queue -> qu_created));
//    QUD_Total_Queues++;


    /* Return successful completion.  */
    return(AK_SUCCESS);
}
/*end of create Queue*/

/*Delete Queue*/

T_S32 AK_Delete_Queue(T_hQueue queue)
{
	T_S32 status;

	status = QUCE_Delete_Queue((NU_QUEUE*)queue);
    if(AK_SUCCESS == status)
    {
        free((T_VOID*)queue);
		queue = (T_hQueue)AK_NULL;
    }

    return status;
}

T_S32  QUCE_Delete_Queue(NU_QUEUE *queue_ptr)
{

QU_QCB         *queue;                      
T_S32          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;

    else
    
        /* All the parameters are okay, call the actual function to delete
           a queue.  */
        status =  QUC_Delete_Queue(queue_ptr);
                                  
    /* Return completion status.  */
    return(status);
}

T_S32  QUC_Delete_Queue(NU_QUEUE *queue_ptr)
{

QU_QCB      *queue;                      /* Queue control block ptr   */
QU_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
QU_SUSPEND     *next_ptr;                   /* Next suspend block pointer*/


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Clear the queue ID.  */
    queue -> qu_id =  0;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  queue -> qu_suspension_list;

    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
             AK_QUEUE_DELETED.  */
        suspend_ptr -> qu_return_status =  AK_QUEUE_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (QU_SUSPEND *) (suspend_ptr -> qu_suspend_link.cs_next);

        /* Resume the specified task.  */
        /******************************change**************************/
        SetEvent(suspend_ptr->qu_suspended_event);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == queue -> qu_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  AK_NULL;
        else

            /* Position suspend pointer to the next pointer.  */
            suspend_ptr =  next_ptr;

    }

    /* Pickup the urgent message suspension list.  */
    suspend_ptr =  queue -> qu_urgent_list;

    /* Walk the chain task(s) currently suspended on the queue.  */
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           AK_QUEUE_DELETED.  */
        suspend_ptr -> qu_return_status =  AK_QUEUE_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (QU_SUSPEND *) (suspend_ptr -> qu_suspend_link.cs_next);

        /* Resume the specified task.  */
        SetEvent(suspend_ptr->qu_suspended_event);
//    TCC_Resume_Task((T_hTask) suspend_ptr -> qu_suspended_task,AK_QUEUE_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == queue -> qu_urgent_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  AK_NULL;
        else

            /* Position to the next suspend block in the list.  */
            suspend_ptr =  next_ptr;

    }

    /* Determine if preemption needs to occur.  */
//    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
  //      TCT_Control_To_System();

    /* Return a successful completion.  */
    return(AK_SUCCESS);
}
/*END of DELETE QUEUE*/

/*Now begin send to queue*/
T_S32 AK_Send_To_Queue(T_hQueue queue, T_VOID *message, 
                              T_U32 size, T_U32 suspend)
{
    return QUCE_Send_To_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}


T_S32  QUCE_Send_To_Queue(NU_QUEUE *queue_ptr, T_VOID *message, T_U32 size, 
                                                        T_U32 suspend)
{

QU_QCB         *queue;
T_S32          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;

    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  AK_INVALID_POINTER;

    else if (size == 0)
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;
        
    else
    
        /* All the parameters are okay, call the actual function to send
           a message to a queue.  */
        status =  QUC_Send_To_Queue(queue_ptr, message, size, suspend);
                              
    /* Return completion status.  */
    return(status);
}

T_S32  QUC_Send_To_Queue(NU_QUEUE *queue_ptr, T_VOID *message, T_U32 size,
                                                        T_U32 suspend)
{

QU_QCB         *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
T_U32          *source;                     /* Pointer to source         */
T_U32          *destination;                /* Pointer to destination    */
T_U32           copy_size;                  /* Partial copy size         */
T_S32             i;                        /* Working counter           */
HANDLE          task;                       /* Task pointer              */
HANDLE          event;                      /* Event pointer for suspend task */
T_S32          status;                     /* Completion status         */
T_U32           suspend_time;               /* suspend time, dull       */

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Initialize the suspend time, I don't know if AK_SUSPEND != INFINITE */
    if (AK_SUSPEND == suspend)
        suspend_time = INFINITE;
    else
        suspend_time = suspend;

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (queue -> qu_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {
        /* Variable messages have one additional word of overhead.  */
        i =  1;

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages))
        {

            /* Pickup task control block pointer.  */
            task = GetCurrentThread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((queue -> qu_fifo_suspend) ||
                (suspend == AK_NO_SUSPEND) ||
                ((queue -> qu_suspension_list) -> qu_suspend_link.cs_priority <=
                                                    GetThreadPriority(task)))

                /* Bump the computed size to aT_VOID placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (T_S32) queue -> qu_available;
        }
    }

    /* Determine if there is enough room in the queue for the message.  The
       extra logic is to prevent a variable-length message from sn*/
    if (queue -> qu_available < (size + i))
    {

        /* Queue does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            queue -> qu_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> qu_message_area =             (T_U32*) message;
            suspend_ptr -> qu_message_size =             size;
            
            /* task = GetCurrentThread();*/
            /* Create the event for suspend task */
            event = CreateEvent(AK_NULL, AK_FALSE, AK_FALSE, AK_NULL);
			suspend_ptr->qu_suspended_event =           event;
            /* Determine if priority or FIFO suspension is associated with the
               queue.  */
            if (queue -> qu_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this queue.  */
                CSC_Place_On_List((CS_NODE **) &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> qu_suspend_link.cs_priority =
                                                 GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            WaitForSingleObject(event, suspend_time);

            //Fwl_printf("Task send quit suspend\r\n");
            
            CloseHandle(event);
            /* Pickup the return status.  */
            status =  suspend_ptr -> qu_return_status;
        }
        else
        {

            /* Return a status of NU_QUEUE_FULL because there is no
               room in the queue for the message.  */
            status =  AK_QUEUE_FULL;

        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

            /* Task is waiting on an empty queue for a message.  */

            /* Decrement the number of tasks waiting on queue.  */
            queue -> qu_tasks_waiting--;

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  queue -> qu_suspension_list;
            CSC_Remove_From_List((CS_NODE **) &(queue -> qu_suspension_list),
                                          &(suspend_ptr -> qu_suspend_link));

            /* Setup the source and destination pointers.  */
            source =       (T_U32*) message;
            destination =  suspend_ptr -> qu_message_area;

            /* Initialize the return status.  */
            suspend_ptr -> qu_return_status =  AK_SUCCESS;

            /* Loop to actually copy the message.  */
            i = (T_S32) size;
            do
            {
                *(destination++) =  *(source);
                if ((--i) == 0)
                    break;
                source++;
            } while (1);

            /* Return the size of the message copied.  */
            suspend_ptr -> qu_actual_size =  size;

            /* Wakeup the waiting task and check for preemption.  */
            if (SetEvent(suspend_ptr->qu_suspended_event))
            {
                //Fwl_print(C3, M_AKFRAME,"send resume receive success!\r\n");
            }
            else
            {
                //Fwl_print(C2, M_AKFRAME,"resume task error!\r\n");
            }
			//Sleep(1000);

        }
        else
        {

            /* There is enough room in the queue and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (T_U32*) message;
            destination =  queue -> qu_write;

            /* Process according to the type of message supported.  */
            if (queue -> qu_fixed_size)
            {

                /* Fixed-size messages are supported by this queue.  */

                /* Loop to copy the message into the queue area.  */
                i =  (T_S32) size;
                do
                {
                    *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);
            }
            else
            {

                /* Variable-size messages are supported.  Processing must
                   check for queue wrap-around conditions.  */

                /* Place message size in first location.  */
                *(destination++) =  size;

                /* Check for a wrap-around condition on the queue.  */
                if (destination >= queue -> qu_end)

                    /* Wrap the write pointer back to the top of the queue
                       area.  */
                    destination =  queue -> qu_start;

                /* Decrement the number of words remaining by 1 for this
                   extra word of overhead.  */
                queue -> qu_available--;

                /* Calculate the number of words remaining from the write
                   pointer to the bottom of the queue.  */
                copy_size =  queue -> qu_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the queue area.  */
                if (copy_size >= size)
                {

                    /* Copy the whole message at once.  */
                    i =  (T_S32) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
                else
                {

                    /* Copy the first half of the message.  */
                    i =  (T_S32) copy_size;
                    do
                    {
                        *(destination) =  *(source++);
                        if ((--i) == 0)
                            break;
                        destination++;
                    } while (1);

                    /* Copy the second half of the message.  */
                    destination =  queue -> qu_start;
                    i =  (T_S32) (size - copy_size);
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
            }

            /* Check again for wrap-around condition on the write pointer. */
            if (destination >= queue -> qu_end)

                /* Move the write pointer to the top of the queue area.  */
                queue -> qu_write =  queue -> qu_start;
            else

                /* Simply copy the last position of the destination pointer
                   into the write pointer.  */
                queue -> qu_write =  destination;

            /* Decrement the number of available words.  */
            queue -> qu_available =  queue -> qu_available - size;

            /* Increment the number of messages in the queue.  */
            queue -> qu_messages++;

        }
    }

    /* Return the completion status.  */
    return(status);
}

/**end of send to queue**/

T_S32 AK_Send_To_Front_of_Queue(T_hQueue queue, T_VOID *message, T_U32 size, T_U32 suspend)
{
    return QUSE_Send_To_Front_Of_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}

T_S32  QUSE_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        T_U32 size, T_U32 suspend)
{

QU_QCB         *queue;
T_S32          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;
    
    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;

    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  AK_INVALID_POINTER;

    else if (size == 0)

        /* Indicate that the message size is invalid.   */
        status = AK_INVALID_SIZE;

    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;
        
    else
    
        /* All the parameters are okay, call the actual function to send
           a message to a queue.  */
        status =  QUS_Send_To_Front_Of_Queue(queue_ptr, message, size, 
                                                                    suspend);
                                  
    /* Return completion status.  */
    return(status);
}

T_S32  QUS_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        T_U32 size, T_U32 suspend)
{

QU_QCB		   *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
T_U32*			source;                     /* Pointer to source         */
T_U32*			destination;                /* Pointer to destination    */
T_U32			copy_size;                  /* Partial copy size         */
T_S32			i;                          /* Working counter           */
HANDLE          task;                       /* Task pointer              */
HANDLE          event;                      /* Event pointer for suspend task */
T_S32          status;                     /* Completion status         */
T_U32           suspend_time;               /* suspend time, dull       */


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Initialize the suspend time, I don't know if AK_SUSPEND != INFINITE */
    if (AK_SUSPEND == suspend)
        suspend_time = INFINITE;
    else
        suspend_time = suspend;

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (queue -> qu_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {
        /* Variable messages have one additional word of overhead.  */
        i =  1;

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages))
        {

            /* Pickup task control block pointer.  */
            task = GetCurrentThread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((queue -> qu_fifo_suspend) ||
                (suspend == AK_NO_SUSPEND) ||
                ((queue -> qu_suspension_list) -> qu_suspend_link.cs_priority <=
                                                    GetThreadPriority(task)))

                /* Bump the computed size to avoid placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (T_S32) queue -> qu_available;
        }
    }

    /* Determine if there is enough room in the queue for the message.  */
    if (queue -> qu_available < (size + i))
    {

        /* Queue does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            queue -> qu_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> qu_message_area =             (T_U32*) message;
            suspend_ptr -> qu_message_size =             size;
            
            /*task = (HANDLE) GetCurrentThread();*/
            event = CreateEvent(AK_NULL, AK_FALSE, AK_FALSE, AK_NULL);
			suspend_ptr -> qu_suspended_event =          event;

            /* Place the task on the urgent message suspension list.  */
            CSC_Place_On_List((CS_NODE **) &(queue -> qu_urgent_list),
                                        &(suspend_ptr -> qu_suspend_link));

            /* Move the head pointer of the list to make this suspension the
               first in the list.  */
            queue -> qu_urgent_list =  (QU_SUSPEND *)
                (queue -> qu_urgent_list) -> qu_suspend_link.cs_previous;

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            WaitForSingleObject(event, suspend_time);

            CloseHandle(event);
            /* Pickup the return status.  */
            status =  suspend_ptr -> qu_return_status;
        }
        else
        {

            /* Return a status of NU_QUEUE_FULL because there is no
               room in the queue for the message.  */
            status =  AK_QUEUE_FULL;

        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

            /* Task is waiting on queue for a message.  */

            /* Decrement the number of tasks waiting on queue.  */
            queue -> qu_tasks_waiting--;

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  queue -> qu_suspension_list;
            CSC_Remove_From_List((CS_NODE **) &(queue -> qu_suspension_list),
                                          &(suspend_ptr -> qu_suspend_link));

            /* Setup the source and destination pointers.  */
            source =       (T_U32*) message;
            destination =  suspend_ptr -> qu_message_area;

            /* Initialize the return status.  */
            suspend_ptr -> qu_return_status =  AK_SUCCESS;

            /* Loop to actually copy the message.  */
            i =  (T_S32) size;
            do
            {
                *(destination++) =  *(source);
                if ((--i) == 0)
                    break;
                source++;
            } while (1);

            /* Return the size of the message copied.  */
            suspend_ptr -> qu_actual_size =  size;

            /* Wakeup the waiting task and check for preemption.  */
            if (!SetEvent(suspend_ptr->qu_suspended_event))
            {
                //Fwl_print(C2, M_AKFRAME,"resume task error!\r\n");
            }
        }
        else
        {

            /* There is enough room in the queue and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (T_U32*) message;
            destination =  queue -> qu_read;

            /* Process according to the type of message supported.  */
            if (queue -> qu_fixed_size)
            {

                /* Fixed-size message queue.  */

                /* Determine if the read pointer is at the top of the queue
                   area.  */
                if (destination == queue -> qu_start)

                    /* Prepare to place the message in the lower part
                       of the queue area.  */
                    destination =  queue -> qu_end - size;
                else

                    /* Backup the length of a message from the current
                       read pointer.  */
                    destination =  destination - size;

                /* Adjust the actual read pointer before the copy is done.  */
                queue -> qu_read =  destination;

                /* Copy the message into the queue area.  */
                i =  (T_S32) size;
                do
                {
                   *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);
            }
            else
            {

                /* Variable-size message queue.  */

                /* Calculate the number of words remaining at the top of the
                   queue.  */
                copy_size =  destination - queue -> qu_start;

                /* Determine if part of the message needs to be placed at the
                   bottom of the queue area.  */
                if (copy_size < (size + i))

                    /* Compute the starting location for the message.  */
                    destination =  queue -> qu_end - ((size +i) - copy_size);
                else

                    /* Compute the starting location for the message.  */
                    destination =  destination - (size + i);

                /* Adjust the actual queue read pointer also.  */
                queue -> qu_read =  destination;

                /* Place message size in first location.  */
                *(destination++) =  size;

                /* Check for a wrap-around condition on the queue.  */
                if (destination >= queue -> qu_end)

                    /* Wrap the write pointer back to the top of the queue
                       area.  */
                    destination =  queue -> qu_start;

                /* Decrement the number of words remaining by 1 for this
                   extra word of overhead.  */
                queue -> qu_available--;

                /* Calculate the number of words remaining from the
                   destination pointer to the bottom of the queue.  */
                copy_size =  queue -> qu_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the queue area.  */
                if (copy_size >= size)
                {

                    /* Copy the whole message at once.  */
                    i =  (T_S32) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
                else
                {

                    /* Copy the first half of the message.  */
                    i =  (T_S32) copy_size;
                    do
                    {
                        *(destination) =  *(source++);
                        if ((--i) == 0)
                            break;
                        destination++;
                    } while (1);

                    /* Copy the second half of the message.  */
                    destination =  queue -> qu_start;
                    i =  (T_S32) (size - copy_size);
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
            }

            /* Decrement the number of available words.  */
            queue -> qu_available =  queue -> qu_available - size;

            /* Increment the number of messages in the queue.  */
            queue -> qu_messages++;

        }
    }

    /* Return the completion status.  */
    return(status);
}


T_S32 AK_Broadcast_To_Queue(T_hQueue queue, T_VOID *message, T_U32 size, T_U32 suspend)
{
    return QUSE_Broadcast_To_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}

T_S32  QUSE_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        T_U32 size, T_U32 suspend)
{

QU_QCB         *queue;
T_S32          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;

    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  AK_INVALID_POINTER;

    else if (size == 0)

        /* Indicate that the message size is invalid.   */
        status = AK_INVALID_SIZE;

    
    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;
        
    else
    
        /* Broadcast a message to a queue.  */
        status =  QUS_Broadcast_To_Queue(queue_ptr, message, size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}
T_S32  QUS_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        T_U32 size, T_U32 suspend)
{

QU_QCB      *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
T_U32* source;                     /* Pointer to source         */
T_U32* destination;                /* Pointer to destination    */
T_U32        copy_size;                  /* Partial copy size         */
T_S32          i;                          /* Working counter           */
HANDLE          task;                       /* Task pointer              */
HANDLE          event;                      /* Event pointer for suspend task */
T_S32          status;                     /* Completion status         */
T_U32           suspend_time;               /* suspend time, dull       */

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Initialize the suspend time, I don't know if AK_SUSPEND != INFINITE */
    if (AK_SUSPEND == suspend)
        suspend_time = INFINITE;
    else
        suspend_time = suspend;

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (queue -> qu_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {
        /* Variable messages have one additional word of overhead.  */
        i =  1;

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages))
        {

            /* Pickup task control block pointer.  */
            task = GetCurrentThread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((queue -> qu_fifo_suspend) ||
                (suspend == AK_NO_SUSPEND) ||
                ((queue -> qu_suspension_list) -> qu_suspend_link.cs_priority <=
                                                    GetThreadPriority(task)))

                /* Bump the computed size to avoid placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (T_S32) queue -> qu_available;
        }
    }

    /* Determine if there is enough room in the queue for the message.  */
    if (queue -> qu_available < (size + i))
    {

        /* Queue does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            queue -> qu_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr = &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> qu_message_area =            (T_U32*) message;
            suspend_ptr -> qu_message_size =             size;

            /*task = (HANDLE) GetCurrentThread();*/
            event = CreateEvent(AK_NULL, AK_FALSE, AK_FALSE, AK_NULL);
			suspend_ptr -> qu_suspended_event =          event;

            /* Determine if priority or FIFO suspension is associated with the
               queue.  */
            if (queue -> qu_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this queue.  */
                CSC_Place_On_List((CS_NODE **) &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> qu_suspend_link.cs_priority =
                                                    GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            WaitForSingleObject(suspend_ptr->qu_suspended_event, suspend_time);

            CloseHandle(suspend_ptr->qu_suspended_event);
            /* Pickup the return status.  */
            status =  suspend_ptr -> qu_return_status;
        }
        else
        {

            /* Return a status of NU_QUEUE_FULL because there is no
               room in the queue for the message.  */
            status =  AK_QUEUE_FULL;
        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

            /* Yes, one or more tasks are waiting for a message from this
               queue.  */
            do
            {

                /* Decrement the number of tasks waiting on queue.  */
                queue -> qu_tasks_waiting--;

                /* Remove the first suspended block from the list.  */
                suspend_ptr =  queue -> qu_suspension_list;
                CSC_Remove_From_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                          &(suspend_ptr -> qu_suspend_link));

                /* Setup the source and destination pointers.  */
                source =       (T_U32*) message;
                destination =  suspend_ptr -> qu_message_area;

                /* Initialize the return status.  */
                suspend_ptr -> qu_return_status =  AK_SUCCESS;

                /* Loop to actually copy the message.  */
                i =  (T_S32) size;
                do
                {
                    *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);

                /* Return the size of the message copied.  */
                suspend_ptr -> qu_actual_size =  size;

                /* Wakeup the waiting task and check for preemption.  */
                if (!SetEvent(suspend_ptr->qu_suspended_event))
                {
                    //Fwl_print(C2, M_AKFRAME,"resume task error!\r\n");
                }

                /* Move the suspend pointer to the next node, which is now
                   at the head of the list.  */
                suspend_ptr =  queue -> qu_suspension_list;
            } while (suspend_ptr);

        }
        else
        {

            /* There is enough room in the queue and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (T_U32*) message;
            destination =  queue -> qu_write;

            /* Process according to the type of message supported.  */
            if (queue -> qu_fixed_size)
            {

                /* Fixed-size messages are supported by this queue.  */

                /* Loop to copy the message into the queue area.  */
                i =  (T_S32) size;
                do
                {
                   *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);
            }
            else
            {

                /* Variable-size messages are supported.  Processing must
                   check for queue wrap-around conditions.  */

                /* Place message size in first location.  */
                *(destination++) =  size;

                /* Check for a wrap-around condition on the queue.  */
                if (destination >= queue -> qu_end)

                    /* Wrap the write pointer back to the top of the queue
                       area.  */
                    destination =  queue -> qu_start;

                /* Decrement the number of words remaining by 1 for this
                   extra word of overhead.  */
                queue -> qu_available--;

                /* Calculate the number of words remaining from the write
                   pointer to the bottom of the queue.  */
                copy_size =  queue -> qu_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the queue area.  */
                if (copy_size >= size)
                {

                    /* Copy the whole message at once.  */
                    i =  (T_S32) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while(1);
                }
                else
                {

                    /* Copy the first half of the message.  */
                    i =  (T_S32) copy_size;
                    do
                    {
                        *(destination) =  *(source++);
                        if ((--i) == 0)
                            break;
                        destination++;
                    } while (1);

                    /* Copy the second half of the message.  */
                    destination =  queue -> qu_start;
                    i =  (T_S32) (size - copy_size);
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
            }

            /* Check again for wrap-around condition on the write pointer. */
            if (destination >= queue -> qu_end)

                /* Move the write pointer to the top of the queue area.  */
                queue -> qu_write =  queue -> qu_start;
            else

                /* Simply copy the last position of the destination pointer
                   into the write pointer.  */
                queue -> qu_write =  destination;

            /* Decrement the number of available words.  */
            queue -> qu_available =  queue -> qu_available - size;

            /* Increment the number of messages in the queue.  */
            queue -> qu_messages++;

        }
    }

    /* Return the completion status.  */
    return(status);
}

T_S32 AK_Receive_From_Queue(T_hQueue queue, T_VOID *message, T_U32 size, 
                            T_U32 *actual_size, T_U32 suspend)
{
    T_S32 status;
    status = QUCE_Receive_From_Queue((NU_QUEUE*)queue, message, CBYTE(size), actual_size, suspend);
    *actual_size = *actual_size*4;
    return status;
}

T_S32  QUCE_Receive_From_Queue(NU_QUEUE *queue_ptr, T_VOID *message,
                T_U32 size, T_U32 *actual_size, T_U32 suspend)
{

QU_QCB         *queue;
T_S32          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;
    
    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status = AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  AK_INVALID_QUEUE;

    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  AK_INVALID_POINTER;

    else if (size == 0)
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  AK_INVALID_SIZE;

    else
    
        /* All the parameters are okay, call the actual function to receive
           a message from a queue.  */
        status =  QUC_Receive_From_Queue(queue_ptr, message, size, 
                                                actual_size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}

T_S32  QUC_Receive_From_Queue(NU_QUEUE *queue_ptr, T_VOID *message,
                T_U32 size, T_U32 *actual_size, T_U32 suspend)
{

QU_QCB         *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
T_U32          *source;                     /* Pointer to source         */
T_U32          *destination;                /* Pointer to destination    */
T_U32           copy_size;                  /* Number of words to copy   */
T_S32           i;                          /* Working counter           */
HANDLE          task;                       /* Task pointer              */
HANDLE          event;                      /* Event pointer for suspend task */
T_S32          status;                     /* Completion status         */
T_U32           suspend_time;               /* suspend time, dull       */



    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Initialize the status as successful.  */
    status =  AK_SUCCESS;

    /* Initialize the suspend time, I don't know if AK_SUSPEND != INFINITE */
    if (AK_SUSPEND == suspend)
        suspend_time = INFINITE;
    else
        suspend_time = suspend;

    /* Determine if an urgent message request is currently suspended.  */
    if (queue -> qu_urgent_list)
    {

        /* If so, copy the message from the suspended request block and
           resume the associated task.  */

        /* Decrement the number of tasks waiting on queue.  */
        queue -> qu_tasks_waiting--;

        /* Remove the first suspended block from the list.  */
        suspend_ptr =  queue -> qu_urgent_list;
        CSC_Remove_From_List((CS_NODE **) &(queue -> qu_urgent_list),
                                          &(suspend_ptr -> qu_suspend_link));

        /* Setup the source and destination pointers.  */
        destination =   (T_U32*) message;
        source =        suspend_ptr -> qu_message_area;

        /* Initialize the return status.  */
        suspend_ptr -> qu_return_status =  AK_SUCCESS;

        /* Loop to actually copy the message.  */
        i =  (T_S32) suspend_ptr -> qu_message_size;
        do
        {
            *(destination++) =  *(source);
            if ((--i) == 0)
                break;
            source++;
        } while (1);

        /* Return the size of the message copied.  */
        *actual_size =  suspend_ptr -> qu_message_size;

        /* Wakeup the waiting task and check for preemption.  */
        if (!SetEvent(suspend_ptr->qu_suspended_event))
        {
            //Fwl_print(C2, M_AKFRAME,"resume task error!\r\n");
        }

    }

    /* Determine if there are messages in the queue.  */
    else if (queue -> qu_messages)
    {

        /* Copy message from queue into the caller's area.  */

        /* Setup the source and destination pointers.  */
        source =       queue -> qu_read;
        destination =  (T_U32*) message;

        /* Process according to the type of message supported by the queue. */
        if (queue -> qu_fixed_size)
        {

            /* Queue supports fixed-size messages.  */

            /* Copy the message from the queue area into the destination.  */
            i =  (T_S32) size;
            do
            {
                *(destination) =  *(source++);
                if ((--i) == 0)
                    break;
                destination++;
            } while (1);
        }
        else
        {

            /* Queue supports variable-size messages.  */

            /* Variable length message size is actually in the queue area. */
            size =  *(source++);

            /* Check for a wrap-around condition on the queue.  */
            if (source >= queue -> qu_end)

                /* Wrap the read pointer back to the top of the queue
                   area.  */
                source =  queue -> qu_start;

            /* Increment the number of available words in the queue.  */
            queue -> qu_available++;

            /* Calculate the number of words remaining from the read pointer
               to the bottom of the queue.  */
            copy_size =  queue -> qu_end - source;

            /* Determine if the message needs to be wrapped around the
               edge of the queue area.  */
            if (copy_size >= size)
            {

                /* Copy the whole message at once.  */
                i =  (T_S32) size;
                do
                {
                    *(destination) =  *(source++);
                    if ((--i) == 0)
                        break;
                    destination++;
                } while (1);
            }
            else
            {

                /* Copy the first half of the message.  */
                i =  (T_S32) copy_size;
                do
                {
                    *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);

                /* Copy the second half of the message.  */
                source =  queue -> qu_start;
                i =  (T_S32) (size - copy_size);
                do
                {
                    *(destination) =  *(source++);
                    if ((--i) == 0)
                        break;
                    destination++;
                } while (1);
            }
        }

        /* Check again for wrap-around condition on the read pointer. */
        if (source >= queue -> qu_end)

            /* Move the read pointer to the top of the queue area.  */
            queue -> qu_read =  queue -> qu_start;
        else

            /* Move the read pointer to where the copy left off.  */
            queue -> qu_read =  source;

        /* Increment the number of available words.  */
        queue -> qu_available =  queue -> qu_available + size;

        /* Decrement the number of messages in the queue.  */
        queue -> qu_messages--;

        /* Return the number of words received.  */
        *actual_size =  size;

        /* Determine if any tasks suspended on a full queue can be woken
           up.  */
        if (queue -> qu_suspension_list)
        {

            /* Overhead of each queue message.  */
            if (!queue -> qu_fixed_size)

                i =  1;
            else

                i =  0;

            /* Pickup the suspension list and examine suspension blocks
               to see if the message could now fit in the queue.  */
            suspend_ptr =  queue -> qu_suspension_list;
            while ((suspend_ptr) &&
              ((suspend_ptr -> qu_message_size + i) <= queue -> qu_available))
            {

                /* Place the suspended task's message into the queue.  */

                /* Setup the source and destination pointers.  */
                source =        suspend_ptr -> qu_message_area;
                destination =   queue -> qu_write;
                size =          suspend_ptr -> qu_message_size;

                /* Process according to the type of message supported.  */
                if (queue -> qu_fixed_size)
                {

                    /* Fixed-size messages are supported by this queue.  */

                    /* Loop to copy the message into the queue area.  */
                    i =  (T_S32) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
                else
                {

                    /* Variable-size messages are supported.  Processing must
                       check for queue wrap-around conditions.  */

                    /* Place message size in first location.  */
                    *(destination++) =  size;

                    /* Check for a wrap-around condition on the queue.  */
                    if (destination >= queue -> qu_end)

                        /* Wrap the write pointer back to the top of the queue
                           area.  */
                        destination =  queue -> qu_start;

                    /* Decrement the number of words remaining by 1 for this
                       extra word of overhead.  */
                    queue -> qu_available--;

                    /* Calculate the number of words remaining from the write
                       pointer to the bottom of the queue.  */
                    copy_size =  queue -> qu_end - destination;

                    /* Determine if the message needs to be wrapped around the
                       edge of the queue area.  */
                    if (copy_size >= size)
                    {

                        /* Copy the whole message at once.  */
                        i =  (T_S32) size;
                        do
                        {
                            *(destination++) =  *(source);
                            if ((--i) == 0)
                                break;
                            source++;
                        } while(1);
                    }
                    else
                    {

                        /* Copy the first half of the message.  */
                        i =  (T_S32) copy_size;
                        do
                        {
                            *(destination) =  *(source++);
                            if ((--i) == 0)
                                break;
                            destination++;
                        } while (1);

                        /* Copy the second half of the message.  */
                        destination =  queue -> qu_start;
                        i =  (T_S32) (size - copy_size);
                        do
                        {
                            *(destination++) =  *(source);
                            if ((--i) == 0)
                                break;
                            source++;
                        } while (1);
                    }
                }

                /* Check again for wrap-around condition on the write
                   pointer. */
                if (destination >= queue -> qu_end)

                    /* Move the write pointer to the top of the queue area.  */
                    queue -> qu_write =  queue -> qu_start;
                else

                    /* Simply copy the last position of the destination pointer
                       into the write pointer.  */
                    queue -> qu_write =  destination;

                /* Decrement the number of available words.  */
                queue -> qu_available =  queue -> qu_available - size;

                /* Increment the number of messages in the queue.  */
                queue -> qu_messages++;

                /* Decrement the number of tasks waiting counter.  */
                queue -> qu_tasks_waiting--;

                /* Remove the first suspended block from the list.  */
                CSC_Remove_From_List((CS_NODE **)
                    &(queue -> qu_suspension_list),
                                &(suspend_ptr -> qu_suspend_link));

                /* Return a successful status.  */
                suspend_ptr -> qu_return_status =  AK_SUCCESS;

                /* Resume the suspended task.  */
                if (SetEvent(suspend_ptr->qu_suspended_event))
                {
                    //Fwl_printf("receive resume send success!\r\n");
                }
                else
                    //Fwl_print(C2, M_AKFRAME,"Task resume error!\r\n");

                /* Setup suspend pointer to the head of the list.  */
                suspend_ptr =  queue -> qu_suspension_list;

                /* Overhead of each queue message.  */
                if (!queue -> qu_fixed_size)

                    i =  1;
                else

                    i =  0;
            }
        }
    }
    else
    {

        /* Queue is empty.  Determine if the task wants to suspend.  */
        if (suspend)
        {

            /* Increment the number of tasks waiting on the queue counter. */
            queue -> qu_tasks_waiting++;

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr = &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     AK_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = AK_NULL;
            suspend_ptr -> qu_message_area =           (T_U32*) message;
            suspend_ptr -> qu_message_size =             size;
            
            /*task = GetCurrentThread();*/
            event = CreateEvent(AK_NULL, AK_FALSE, AK_FALSE, AK_NULL);
			suspend_ptr -> qu_suspended_event =          event;
            
            /* Determine if priority or FIFO suspension is associated with the
               queue.  */
            if (queue -> qu_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this queue.  */
                CSC_Place_On_List((CS_NODE **) &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
				
				/* Initialize task */
				task = GetCurrentThread();
                suspend_ptr -> qu_suspend_link.cs_priority =
                                                    GetThreadPriority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            WaitForSingleObject(suspend_ptr->qu_suspended_event, suspend_time);

            //Fwl_print(C3, M_AKFRAME,"Task receive quit suspend!\r\n");
            CloseHandle(suspend_ptr->qu_suspended_event);
            
            /* Pickup the status of the request.  */
            status =  suspend_ptr -> qu_return_status;
            *actual_size =  suspend_ptr -> qu_actual_size;
        }
        else
        {

            /* Return a status of NU_QUEUE_EMPTY because there are no
               messages in the queue.  */
            status =  AK_QUEUE_EMPTY;
        }
    }

    /* Return the completion status.  */
    return(status);
}


T_S32 AK_Send_Unique_To_Queue(T_hQueue queue_ptr, T_VOID *message, T_U32 size, 
                                    T_U32 suspend, CallbakCompare Function)
{
    QU_QCB *queue;
    T_U32 *source, *end;
    T_BOOL wrap;

    wrap = AK_FALSE;
    queue = (QU_QCB*) queue_ptr;
    source = queue->qu_read;
    end = queue->qu_write;

    if(source > end)
        wrap = AK_TRUE;

    // no message    
    if( 0 == queue->qu_messages )
        return AK_Send_To_Queue(queue_ptr, message, size, suspend);

    while(1)
    {        
        if(!Function((T_VOID*)source, (T_VOID*)message))
        {
            source += size;

            // check for the end of the queue
            if((!wrap) && (source >= end))
            {
                break;
            }

            // check again for wrap-around condition on the read pointer.
            if((wrap) && (source >= queue->qu_end))
            {
                source =  queue->qu_start;
                wrap = AK_FALSE;

                // to avoid two pointers are all at qu_start and has got the end
                if (source == end)
                    break;
            }
        }
        else
            return AK_EXIST_MESSAGE;
    }

    return AK_Send_To_Queue(queue_ptr, message, size, suspend);
}

T_S32 AK_Send_Unique_To_Front_of_Queue(T_hQueue queue_ptr, T_VOID *message, 
                                    T_U32 size, T_U32 suspend, CallbakCompare Function)
{
    QU_QCB *queue;
    T_U32 *source, *end;
    T_BOOL wrap = AK_FALSE;
    
    queue = (QU_QCB*) queue_ptr;
    source = queue->qu_read;
    end = queue->qu_write;

    if(source > end)
        wrap = AK_TRUE;
    
    // no message    
    if( 0 == queue->qu_messages )
        return AK_Send_To_Front_of_Queue(queue_ptr, message, size, suspend);;

    while(1)
    {        
        if(!Function((T_VOID*)source, (T_VOID*)message))
        {
            source += size;

            // check for the end of the queue
            if((!wrap) && (source >= end))
            {
                break;
            }

            // check again for wrap-around condition on the read pointer.
            if((wrap) && (source >= queue->qu_end))
            {
                source =  queue->qu_start;
                wrap = AK_FALSE;

                // to avoid two pointer are all at qu_start and has got the end
                if (source == end)
                    break;
            }
        }
        else
            return AK_EXIST_MESSAGE;
    }

    return AK_Send_To_Front_of_Queue(queue_ptr, message, size, suspend);
}

T_S32 AK_Queue_Information
	( T_hQueue queue_ptr, T_VOID **start_address, T_U32 *queue_size, 
    T_U32 *available, T_U32 *messages, T_OPTION *message_type, T_U32 *message_size, 
    T_OPTION *suspend_type, T_U32 *tasks_waiting, T_hTask *first_task)
{

    QU_QCB *queue;
	T_S32 status = AK_SUCCESS;

	queue = (QU_QCB*) queue_ptr;

    if ((queue != AK_NULL) && (queue->qu_id == QU_QUEUE_ID))
    {

        /* Determine the suspension type.  */
        if (queue->qu_fifo_suspend)
            *suspend_type =  AK_FIFO;
        else
            *suspend_type =  AK_PRIORITY;

        /* Determine the message type.  */
        if (queue->qu_fixed_size)
            *message_type =  AK_FIXED_SIZE;
        else
            *message_type =  AK_VARIABLE_SIZE;

        /* Get various information about the queue.  */
        *start_address 	=  queue->qu_start;
        *queue_size 	=  queue->qu_queue_size;
        *available 		=  queue->qu_available;
        *messages 		=  queue->qu_messages;
        *message_size 	=  queue->qu_message_size;

        /* Retrieve the number of tasks waiting and the pointer to the first task waiting.  */
        *tasks_waiting =  queue->qu_tasks_waiting;

        if (queue->qu_suspension_list)

	        /* There is a task waiting.  */
	        *first_task =  (T_hTask)
	            (queue->qu_suspension_list)->qu_suspended_event;

		else
			*first_task = 0x0;
    }
    else

        /* Indicate that the queue pointer is invalid.   */
        status =  AK_INVALID_QUEUE;

    /* Return the appropriate completion status.  */
	return status;
}

T_U32 AK_Established_Queues(T_VOID)
{
	return 0;
}
#endif
#endif
// END of File
