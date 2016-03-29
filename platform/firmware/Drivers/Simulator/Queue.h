#ifdef  OS_WIN32
#ifndef _AK_QUEUE_H_
#define _AK_QUEUE_H_

#include "windows.h"
#include "anyka_types.h"

typedef struct  CS_NODE_STRUCT
{
    struct CS_NODE_STRUCT  *cs_previous;
    struct CS_NODE_STRUCT  *cs_next;
    T_U8            cs_priority;
}  CS_NODE;

typedef struct QU_QCB_STRUCT 
{
    CS_NODE             qu_created;            /* Node for linking to    */
                                               /*   created queue list   */
    T_U32            qu_id;                 /* Internal QCB ID        */
    T_U8             qu_name[AK_MAX_NAME];  /* Queue name             */
    T_U8             qu_fixed_size;         /* Fixed-size messages?   */
    T_U8             qu_fifo_suspend;       /* Suspension type flag   */
    T_U32            qu_queue_size;         /* Total size of queue    */
    T_U32            qu_messages;           /* Messages in queue      */
    T_U32            qu_message_size;       /* Size of each message   */
    T_U32            qu_available;          /* Available words        */
    T_U32           *qu_start;              /* Start of queue area    */
    T_U32           *qu_end;                /* End of queue area + 1  */
    T_U32           *qu_read;               /* Read pointer           */
    T_U32           *qu_write;              /* Write pointer          */
    T_U32            qu_tasks_waiting;      /* Number of waiting tasks*/
    struct QU_SUSPEND_STRUCT
                       *qu_urgent_list;        /* Urgent message suspend */
    struct QU_SUSPEND_STRUCT
                       *qu_suspension_list;    /* Suspension list        */
} QU_QCB;    

typedef struct QU_SUSPEND_STRUCT
{
    CS_NODE             qu_suspend_link;       /* Link to suspend blocks */
    QU_QCB             *qu_queue;              /* Pointer to the queue   */
    HANDLE              qu_suspended_event;    /* Task suspended         */
    T_U32*              qu_message_area;       /* Pointer to message area*/
    T_U32               qu_message_size;       /* Message size requested */
    T_U32               qu_actual_size;        /* Actual size of message */
    T_S32               qu_return_status;      /* Return status          */
} QU_SUSPEND;
/*
typedef struct TC_PROTECT_STRUCT
{
    TC_TCB             *tc_tcb_pointer;        /* Owner of the protection */
/*    T_U32               tc_thread_waiting;     /* Waiting thread flag     */
/*} TC_PROTECT;
*/



typedef QU_QCB      NU_QUEUE;

#define         QU_QUEUE_ID             0x51554555UL

T_hQueue AK_Create_Queue(T_VOID *start_address, T_U32 queue_size, 
                      T_OPTION message_type, T_U32 message_size, T_OPTION suspend_type);
T_S32 AK_Delete_Queue(T_hQueue task);
T_S32 AK_Send_To_Queue(T_hQueue task, T_VOID *message, T_U32 size, T_U32 suspend);
T_S32 AK_Send_To_Front_of_Queue(T_hQueue queue, T_VOID *message, T_U32 size, T_U32 suspend);
T_S32 AK_Broadcast_To_Queue(T_hQueue queue, T_VOID *message, T_U32 size, T_U32 suspend);
T_S32 AK_Receive_From_Queue(T_hQueue task, T_VOID *message, T_U32 size,
                                T_U32 *actual_size, T_U32 suspend);
T_S32 AK_Send_Unique_To_Queue(T_hQueue queue_ptr, T_VOID *message, T_U32 size, 
                                    T_U32 suspend, CallbakCompare Function);

T_S32 AK_Send_Unique_To_Front_of_Queue(T_hQueue queue_ptr, T_VOID *message, 
                            T_U32 size, T_U32 suspend, CallbakCompare Function);

T_VOID    CSC_Place_On_List(CS_NODE **head, CS_NODE *new_node);
T_VOID    CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node);
T_VOID    CSC_Remove_From_List(CS_NODE **head, CS_NODE *node);

T_S32  QUCE_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name, 
                      T_VOID *start_address, T_U32 queue_size, 
                      T_OPTION message_type, T_U32 message_size,
                      T_OPTION suspend_type);
T_S32  QUC_Create_Queue(NU_QUEUE *queue_ptr, T_U8 *name,
                      T_VOID *start_address, T_U32 queue_size,
                      T_OPTION message_type, T_U32 message_size,
                      T_OPTION suspend_type);
T_S32  QUCE_Delete_Queue(NU_QUEUE *queue_ptr);
T_S32  QUC_Delete_Queue(NU_QUEUE *queue_ptr);
T_S32  QUCE_Send_To_Queue(NU_QUEUE *queue_ptr, T_VOID *message, T_U32 size, 
                                                        T_U32 suspend);
T_S32  QUC_Send_To_Queue(NU_QUEUE *queue_ptr, T_VOID *message, T_U32 size,
                                                        T_U32 suspend);
T_S32  QUSE_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        T_U32 size, T_U32 suspend);
T_S32  QUS_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        T_U32 size, T_U32 suspend);
T_S32  QUSE_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        T_U32 size, T_U32 suspend);
T_S32  QUS_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        T_U32 size, T_U32 suspend);
T_S32  QUCE_Receive_From_Queue(NU_QUEUE *queue_ptr, T_VOID *message,
                T_U32 size, T_U32 *actual_size, T_U32 suspend);
T_S32  QUC_Receive_From_Queue(NU_QUEUE *queue_ptr, T_VOID *message,
                T_U32 size, T_U32 *actual_size, T_U32 suspend);

T_S32 AK_Queue_Information
	( T_hQueue queue_ptr, T_VOID **start_address, T_U32 *queue_size, 
    T_U32 *available, T_U32 *messages, T_OPTION *message_type, T_U32 *message_size, 
    T_OPTION *suspend_type, T_U32 *tasks_waiting, T_hTask *first_task);

#endif
#endif
