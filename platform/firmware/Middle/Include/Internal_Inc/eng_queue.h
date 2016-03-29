/**
 * @file eng_queue.h
 * @brief This file is for mailbox queue function prototype
 * @author ANYKA
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __ENG_QUEUE_H__
#define __ENG_QUEUE_H__

#include "anyka_types.h"

#define MAX_MAILBOX_NUMBER 10	/**< maximum number of mailbox*/

/** structure definition for mailbox*/
typedef struct
{
	T_U16	Messageboxtype;		/**< type of message box*/
	T_S16	delayTime;			/**< delay time*/
	T_TCHR	*MessageBoxTitle;	/**< title of message box*/
    T_TCHR	*MessageBoxContent; /**< content of message box*/
} T_MAILBOX;

/** global variables for mailbox queue*/
extern T_MAILBOX gb_mbQueue[MAX_MAILBOX_NUMBER];



typedef struct Ak_QuNode {
	T_pSTR 				string;
	struct Ak_QuNode*	next;
} AkQuNode, *AkQuNodePoint;		/* node */

typedef struct {
	AkQuNodePoint	first;
	AkQuNodePoint	last;
	T_S32			queuelength;
	T_S32			queuefull;
} AK_QUEUE;


/** @defgroup MQUE Mailbox queue interface
 * @ingroup ENG
 */
/*@{*/
/**
 * @brief Init a queue	
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] queue		An AK_QUEUE point points to the queue to be initiated
 * @return T_S32
 * @retval 0	if fail
 * @retval 1	if init success
 */
T_S32	QueueInit(AK_QUEUE *queue);

/**
 * @brief Get the header element from the queue	
 * 
 * @author \b  pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] queue		An AK_QUEUE point points to the queue 
 * @return T_S8
 * @retval AK_NULL		if fail 
 * @retval else			points to the element gotten from the queue
 */
T_pSTR QueueGetHead(AK_QUEUE *queue);

/**
 * @brief Add an element to the tail of the queue	
 * 
 * @author pyxue
 * @date 2001-07-27
 * @param[in] queue		An AK_QUEUE point points to the queue to be added
 * @param[in] string	a string to be added
 * @return T_S32
 * @retval 0	if fail
 * @retval 1	if init success
 */
T_S32	QueueAddTail(AK_QUEUE *queue, T_pSTR string);

/**
 * @brief decide if  queue if empty	
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] queue		An AK_QUEUE point points to the queue to be initiated
 * @return T_S32
 * @retval  0  empty
 * @retval	1  not empty
 */
T_S32	QueueIsEmpty(AK_QUEUE *queue);

/**
 * @brief free queue
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] queue		An AK_QUEUE point points to the queue to be initiated
 * @return 1
 */
T_S32 QueueFree(AK_QUEUE *queue);

/**
 * @brief push the event into mailbox
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] MessageTitle		message title
 * @param[in] MessageContent	message content
 * @param[in] MessageType		message type
 * @param[in] delayTime			delay time
 * @return void
 */
T_VOID PushMailbox( T_TCHR* MessageTitle, T_TCHR* MessageContent, T_U8 MessageType, T_S16 delayTime );

/**
 * @brief Get the head mailbox from the mailbox queue
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[out] mailbox		head mailbox
 * @return success or not
 */
T_BOOL GetMailbox( T_MAILBOX *mailbox );

/**
 * @brief free the mailbox
 * 
 * @author \b pyxue
 * 
 * @author 
 * @date 2001-07-27
 * @param[in] mailbox		mailbox to be free
 * @return void
 */
T_VOID MailboxFree(T_MAILBOX *mailbox);

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
T_VOID MailBoxInit(T_VOID);
/*@}*/
#endif


