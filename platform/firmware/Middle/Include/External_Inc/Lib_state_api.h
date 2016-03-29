#ifndef __M_STATE_API_H__
#define __M_STATE_API_H__

#include "Lib_event_api.h"
#include "Lib_state.h"

/** definition of transmit type of state machine*/
typedef enum
{
	/** @{@name all function call types
     */    
	/** funcall*/
    eM_TYPE_SCALL,
	/** efuncall*/
    eM_TYPE_ECALL,
	/** none*/
    eM_TYPE_NONE,
	/** @} */

	/** @{@name all transistion types
     */  
	/** popped transition*/
    eM_TYPE_EXIT,
	/** pushed transition*/
    eM_TYPE_IRPT,
	/** @} */

	/** @{@name all return types
     */  
	/** return top state transition*/
    eM_TYPE_RETURN_ROOT,
	/** return n level transition*/
    eM_TYPE_RETURN
	/** @} */
} M_TRANSTYPE;

//vT_EvtCode must defined as unsigned int outside lib
//sizeof(vT_EvtParam) must <= sizeof(unsigned long[4]) outside lib

/**
 * @brief process function of state machine events
 * @param[in] event   incoming event value 
 * @param[in] pEventParm   address pointer of incoming event parameter
 * @return 1: transmit this event to next state;
 *         0: this event has been processed
 */
typedef unsigned char (*_fHandle)(vT_EvtCode event, vT_EvtParam* pEventParm);
/**
 * @brief process efunction of state machine events
 * @param[in] event   incoming event value 
 * @param[in] pEventParm   address pointer of incoming event parameter
 * @return 1: transmit this event to next state;
 *         0: this event has been processed
 */
typedef unsigned char (*_feHandle)(vT_EvtCode* event, vT_EvtParam** pEventParm);
/**
 * @brief query function for state machine transfer
 * @param[in] event    event value 
 * @param[in] pTrans   transfer type
 * @return ID for event processing or destination state machine 
 */
typedef int (*_fGetNextState)(vT_EvtCode event, M_TRANSTYPE *pTrans);

/** structure type definition for state*/
typedef struct 
{
	/** address pointer for init function*/
    _fVoid pInit;
	/** address pointer for exit function*/
    _fVoid pExit;
	/** address pointer for paint function*/
    _fVoid pPaint;
	/** address pointer for next state machine*/
    _fGetNextState pNext;
} M_STATESTRUCT;


/** @defgroup State_API State interface 
 *	@ingroup SMCORE
 */
/*@{*/
/**
 * @brief init SM Core
 * @param[in] void
 * @return void
 */
void m_initStateHandler(void);

/**
 * @brief main loop for management of stae machine
 * @param[in] Event   triggered event 
 * @param[in] pParam   address pointer of triggered event parameter
 * @return void
 */
void m_mainloop(vT_EvtCode Event, vT_EvtParam *pParam);

/**
 * @brief register suspend function
 * @param[in] pfSuspend   address pointer for suspend function
 * @return void
 */
void  m_regSuspendFunc(_fVoid pfSuspend);

/**
 * @brief register resume function
 * @param[in] pfResume   address pointer for resume function
 * @return void
 */
void  m_regResumeFunc(_fVoid pfResume);

/**
 * @brief register push function
 * @param[in] pfPush   address pointer for state machine push function
 * @param[in] flag	   0:pfPush call before push action func; 1:after push action func
 * @return 0 for success, else -1 for failure
 */
int  m_addPushFunc(_fVoid pfPush, int flag);

/**
 * @brief unregister push function
 * @param[in] pfPush   address pointer for state machine push function
 * @return 0 for success, else -1 for failure
 */
int  m_delPushFunc(_fVoid pfPush);

/**
 * @brief register pop function
 * @param[in] pfPush   address pointer for state machine pop function
 * @param[in] flag	   0:pfPush call before pop action func; 1:after pop action func
 * @return 0 for success, else -1 for failure
 */
int  m_addPopFunc(_fVoid pfPop, int flag);

/**
 * @brief unregister pop function
 * @param[in] pfPush   address pointer for state machine pop function
 * @return 0 for success, else -1 for failure
 */
int  m_delPopFunc(_fVoid pfPop);

/**
 * @brief get current ID of state machine on toe of stack
 * @param[in] void
 * @return ID of state machine on toe of stack
 */
M_STATES SM_GetCurrentSM(void);

/**
 * @brief get current max stack depth of state machine which can be used
 * @param[in] void
 * @return  stack depth 
 */
unsigned int SM_GetStackMaxDepth(void);
/**
 * @brief get current size of stack buffer area according to the max stack 
		  depth of state machine which can be used
 * @param[in] maxDepth max stack depth 
 * @return  size of stack buffer area
 */
unsigned int SM_CalcStackBufByMaxDepth(unsigned int maxDepth);

unsigned int SM_GetCurrentStackDepth(void);

/*@}*/
#endif
