#ifdef  OS_WIN32
#ifndef _AK_MAILBOX_H_
#define _AK_MAILBOX_H_

#include "windows.h"
#include "anyka_types.h"
#include "queue.h"

#define         MB_MAILBOX_ID           0x4d424f58UL
#define         MB_MESSAGE_SIZE         4

typedef struct MB_MCB_STRUCT 
{
    CS_NODE             mb_created;            /* Node for linking to    */
                                               /*   created mailbox list */
    T_U32               mb_id;                 /* Internal MCB ID        */
    T_U8                mb_name[AK_MAX_NAME];  /* Mailbox name           */
    T_S32              mb_message_present;    /* Message present flag   */
    T_S32              mb_fifo_suspend;       /* Suspension type flag   */
    T_U32               mb_tasks_waiting;      /* Number of waiting tasks*/
    T_U32                                      /* Message area           */
                        mb_message_area[MB_MESSAGE_SIZE];
    struct MB_SUSPEND_STRUCT
                       *mb_suspension_list;    /* Suspension list        */
} MB_MCB;    

typedef struct MB_SUSPEND_STRUCT
{
    CS_NODE             mb_suspend_link;       /* Link to suspend blocks */
    MB_MCB             *mb_mailbox;            /* Pointer to the mailbox */
    HANDLE              mb_suspended_task;     /* Task suspended         */
    T_U32              *mb_message_area;       /* Pointer to message area*/
    T_S32              mb_return_status;      /* Return status          */
} MB_SUSPEND;

typedef MB_MCB          NU_MAILBOX;


T_hMailbox AK_Create_Mailbox(T_OPTION suspend_type);
T_S32 AK_Delete_Mailbox(T_hMailbox mailbox);
T_S32 AK_Broadcast_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type);
T_S32 AK_Receive_From_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type);
T_S32 AK_Send_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type);
T_S32  MBCE_Create_Mailbox(NU_MAILBOX *mailbox_ptr, T_U8 *name, 
												T_OPTION suspend_type);
T_S32  MBC_Create_Mailbox(NU_MAILBOX *mailbox_ptr, T_U8 *name,
                                                T_OPTION suspend_type);
T_S32  MBCE_Delete_Mailbox(NU_MAILBOX *mailbox_ptr);
T_S32  MBC_Delete_Mailbox(NU_MAILBOX *mailbox_ptr);
T_S32  MBCE_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                          T_U32 suspend);
T_S32  MBC_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend);
T_S32  MBCE_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                        T_U32 suspend);
T_S32  MBCE_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                        T_U32 suspend);
T_S32  MBC_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend);
T_S32  MBSE_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message, 
                                                        T_U32 suspend);
T_S32  MBS_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, T_VOID *message,
                                                        T_U32 suspend);
#endif
#endif
