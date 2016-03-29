/** @file 
 * @brief header file for interface  of smcore_lib porting
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */
#ifndef _SM_PORT_H_
#define _SM_PORT_H_

#include "Lib_state_api.h"
/**
 * @brief return triggered event ID which is used for state maching popping stack
 * @param[in] void
 * @return event ID
 */
vT_EvtCode SM_GetEvtReturn(void);

/**
 * @brief return event ID of querying state machine processing function
 * @param[in] void
 * @return event ID
 */
vT_EvtCode SM_GetEvtNoNext(void);

/**
 * @brief return ID of pre-process state machine
 * @param[in] void
 * @return state machine ID
 */
M_STATES SM_GetPreProcID(void);

/**
 * @brief return ID of post-process state machine
 * @param[in] void
 * @return state machine ID
 */
M_STATES SM_GetPostProcID(void);

/**
 * @brief return address of funcall handler 
 * @param[in] void
 * @return address of funcall handler
 */
const _fHandle* SM_GetfHande(void);

/**
 * @brief return address of efuncall handler 
 * @param[in] void
 * @return address of efuncall handler
 */
const _feHandle* SM_GeteHandle(void);

/**
 * @brief return head address of array for state 
 *        machine event processing function's sets
 * @param[in] void
 * @return header address
 */
const M_STATESTRUCT** SM_GetStateArray(void);

/**
 * @brief get buffer size of state machine stack
 * @param[out] bufSize buffer size of state machine stack
 * @return null
 */
void *SM_GetStackBuffer(unsigned int *bufSize);

/**
 * @brief get buffer size of state machine event queue
 * @param[out] bufSize buffer size of state machine event queue
 * @return null
 */
void *SM_GetEventQueueBuffer(unsigned int *bufSize);

/**
 * @brief reset stack of state machine
 * @param[in] void
 * @return void
 */
void SM_StackReset(void);

#endif

