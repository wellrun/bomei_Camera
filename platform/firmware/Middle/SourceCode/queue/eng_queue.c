/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºEng_Queue.c
 * Function£ºThe file process a queue operation.
 
 *
 * Author£ºXPY
 * Date£º2001-07-14
 * Version£º1.0		  
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Eng_Queue.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "akos_api.h"
#include "eng_string_uc.h"
#include "fwl_vme.h"
#include "Eng_debug.h"

static T_hSemaphore semQu;
static T_hSemaphore semMailbox;

T_MAILBOX gb_mbQueue[MAX_MAILBOX_NUMBER];

vINT8	mbQueueHead = 0, mbQueueTail = 0;

static T_VOID CleanMailBox(T_VOID);

/**
 * @brief Init a queue	
 * 
 * @author @b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param AK_QUEUE *queue :An AK_QUEUE point points to the queue to be initiated
 * @return T_S32
 * @retval 0 if fail, 1, if init sucses
 */
T_S32 QueueInit(AK_QUEUE *queue)
{
    AK_FUNCTION_ENTER("QueueInit");
	AK_ASSERT_PTR(queue, "QueueInit(): queue", 0);

	queue->last = 0;
	queue->last = (AkQuNodePoint)Fwl_Malloc(sizeof(AkQuNode));
	if (queue->last == AK_NULL)
		return 0;
	queue->first = 	queue->last;
	if (!queue->first)
   {
        AK_FUNCTION_RET_INT("QueueInit",0);
		return 0;
    }
	queue->first->next = AK_NULL;
	queue->queuelength = 0;
    AK_FUNCTION_RET_INT("QueueInit",1);
	semQu = AK_Create_Semaphore(1, AK_PRIORITY);
	return 1;
}

//This function is not safe... Not used now.
T_S32 QueueFree(AK_QUEUE *queue)
{

	T_CHR* string = QueueGetHead(queue);;

	while(string != AK_NULL)   // free all the members
		string = QueueGetHead(queue);

	queue->last = Fwl_Free(queue->last);

	return 1;
}


/**
 * @brief Add an element to the tail of the queue	
 * 
 * @author pyxue
 * @date 2001-07-27
 * @param AK_QUEUE *queue: An AK_QUEUE point points to the queue to be added
 * @param T_pSTR string: a string to be added
 * @return T_S32
 * @retval  0 if fail, 1, if init sucses
 */
T_S32 QueueAddTail(AK_QUEUE *queue,T_pSTR string)
{
	AkQuNodePoint temp;
	T_S32 i;
	T_S32			retVal = 0;

	AK_Obtain_Semaphore(semQu, AK_SUSPEND);

    AK_FUNCTION_ENTER("QueueAddTail");
	AK_ASSERT_PTR(queue, "QueueAddTail(): queue", 0);
	AK_ASSERT_PTR(string, "QueueAddTail(): string", 0);

	temp = 0;
	temp = (AkQuNodePoint)Fwl_Malloc(sizeof(AkQuNode));
	
	if(!temp)
    {
        AK_FUNCTION_RET_INT("QueueAddTail",0);
		goto quit;
    }
	for(i = 0; *(string + i) != 0; i++)
        ;
	temp->string = (T_pSTR)Fwl_Malloc(++i);
	if (temp->string == AK_NULL)
		goto quit;

	Utl_StrCpy(temp ->string,string);
	temp->next = AK_NULL;
	queue->last ->next = temp;
	queue->last = temp;
	queue->queuelength ++;
    AK_FUNCTION_RET_INT("QueueAddTail",1);
	retVal = 1;
quit:
	AK_Release_Semaphore(semQu);

	return retVal;
}


/**
 * @brief Get the header element from the queue	
 * 
 * @author @b  pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param AK_QUEUE *queue: An AK_QUEUE point points to the queue 
 * @return T_S8
 * @retval AK_NULL if fail, else points to the element gotten from the queue
 */
T_pSTR QueueGetHead(AK_QUEUE *queue)
{
	T_CHR			*strResult;
	AkQuNodePoint	temp;

	AK_FUNCTION_ENTER("QueueGetHead");
	AK_ASSERT_PTR(queue, "QueueGetHead(): queue", AK_NULL);

	AK_Obtain_Semaphore(semQu, AK_SUSPEND);
	if(queue->queuelength == 0)
    {
		AK_Release_Semaphore(semQu);
        AK_FUNCTION_RET_STR("QueueGetHead",AK_NULL);
		return AK_NULL;
    }
	temp = queue->first -> next;

	strResult = temp->string;

	if(queue->queuelength == 1)
	{
		queue->last = queue->first;
		queue->first ->next = AK_NULL;
	}
	else
	{
		queue->first -> next = queue->first ->next -> next;
	}
	queue->queuelength --;

	temp -> next = AK_NULL;
	temp = Fwl_Free((T_pVOID )temp);
	AK_Release_Semaphore(semQu);

    AK_FUNCTION_RET_STR("QueueGetHead",strResult);

	return strResult;
}



/**
 * @brief decide if  queue if empty	
 * 
 * @author @b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param AK_QUEUE *queue: An AK_QUEUE point points to the queue to be initiated
 * @return T_S32
 * @retval  0  empty
 *			1  not empty
 */
T_S32 QueueIsEmpty(AK_QUEUE *queue)
{
    AK_FUNCTION_ENTER("QueueIsEmpty");
	AK_ASSERT_PTR(queue, "QueueIsEmpty(): queue", 0);

	if(queue->queuelength == 0)
    {
        AK_FUNCTION_RET_INT("QueueIsEmpty",1);
		return 1;
    }
	else
    {
        AK_FUNCTION_RET_INT("QueueIsEmpty",0);
		return 0;
    }
}


/* mailbox operation*/


/**
 * @brief mailbox init
 * 
 * @author @b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param T_VOID
 * @return T_S32
 * @retval  T_VOID
 */

T_VOID MailBoxInit(T_VOID)
{
	T_U32 i;

	for(i = 0; i < MAX_MAILBOX_NUMBER; i ++)
	{
		gb_mbQueue[i].Messageboxtype = 0;
		gb_mbQueue[i].MessageBoxTitle = AK_NULL;
	}
	mbQueueHead = 0;
    mbQueueTail = 0;
	semMailbox = AK_Create_Semaphore(1, AK_PRIORITY);
}

/**
 * @brief push the event into mailbox
 * 
 * @author @b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param T_MAILBOX* mailbox
 * @return T_S32
 * @retval  T_VOID
 */

T_VOID PushMailbox( T_TCHR* MessageTitle, T_TCHR* MessageContent, T_U8 MessageType, T_S16 delayTime )
{
    T_MAILBOX	mailbox;

	AK_Obtain_Semaphore(semMailbox, AK_SUSPEND);
	if( (mbQueueTail + 1) % MAX_MAILBOX_NUMBER == mbQueueHead )
	{
		AK_Release_Semaphore(semMailbox);
		return;
	}

	if((mbQueueTail - mbQueueHead) >= 5)
	{
		CleanMailBox();
	}

    mailbox.Messageboxtype = MessageType;
	mailbox.delayTime = delayTime;
    mailbox.MessageBoxContent = (T_TCHR*)Fwl_Malloc((Utl_TStrLen((T_U16*)MessageContent)+1)*sizeof(T_TCHR));
    Utl_TStrCpy((T_U16*)mailbox.MessageBoxContent, (T_U16*)MessageContent);
    mailbox.MessageBoxTitle = (T_TCHR*)Fwl_Malloc((Utl_TStrLen((T_U16*)MessageTitle)+1)*sizeof(T_TCHR));
    Utl_TStrCpy((T_U16*)mailbox.MessageBoxTitle, (T_U16*)MessageTitle);
   
	gb_mbQueue[mbQueueTail] = mailbox;
	mbQueueTail++;
	mbQueueTail %= MAX_MAILBOX_NUMBER;
	AK_Release_Semaphore(semMailbox);
}


/**
 * @brief push the event into mailbox
 * 
 * @author @b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param T_MAILBOX* mailbox
 * @return T_S32
 * @retval  T_BOOL
 */

T_BOOL GetMailbox( T_MAILBOX *mailbox )
{
	AK_Obtain_Semaphore(semMailbox, AK_SUSPEND);
	if( mbQueueHead == mbQueueTail )
	{
		AK_Release_Semaphore(semMailbox);
		return AK_FALSE;
	}

	*mailbox = gb_mbQueue[mbQueueHead];
	mbQueueHead++;
	mbQueueHead %= MAX_MAILBOX_NUMBER;

	AK_Release_Semaphore(semMailbox);
	return AK_TRUE;
}


T_VOID MailboxFree(T_MAILBOX *mailbox)
{
    mailbox->MessageBoxContent = Fwl_Free(mailbox->MessageBoxContent);
    mailbox->MessageBoxTitle = Fwl_Free(mailbox->MessageBoxTitle);
    return;
}

static T_VOID CleanMailBox()
{
	T_MAILBOX *mailbox;

    while (mbQueueHead != mbQueueTail)
    {
        //Point to head mailbox.
        mailbox = &gb_mbQueue[mbQueueHead];
    	mbQueueHead++;
    	mbQueueHead %= MAX_MAILBOX_NUMBER;
        //Free the mailbox content and title.
        mailbox->MessageBoxContent = Fwl_Free(mailbox->MessageBoxContent);
        mailbox->MessageBoxTitle   = Fwl_Free(mailbox->MessageBoxTitle);
    }
    gb.CommonMesgNumber = 0;
}

/////////////////////////////////
