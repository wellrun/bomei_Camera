/** @file 
 * @brief header file for interface  of event related
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __M_EVENT_API_H__
#define __M_EVENT_API_H__

#include "fwl_vme.h"

/** @defgroup SMCORE State machine core engine
 * @ingroup ENG
 */
/*@{*/
/*@}*/



/**  for init, exit, paint*/
typedef void (*_fVoid)(void);

/** @defgroup Event_API Event interface 
 *	@ingroup SMCORE
 */

/*@{*/
/**
 * @brief Add the triggered event to event queue
 * @param[in] event   triggered event
 * @param[in] pEventParm   address pointer of event parameter
 * @return void
 */
void  m_triggerEvent(vT_EvtCode event, vT_EvtParam* pEventParm);

/**
 * @brief Get the max quantity of triggered events which can be 
 *		  accommodated in the event buffer area.
 * @param[in] void
 * @return the max quantity of triggered events which can be accommodated
 */
unsigned int SM_GetEventMaxEntries(void);

/**
 * @brief Get the size of event buffer area according to the quantity of events
 * @param[in] maxEntries the quantity of events in  event queue
 * @return the size of event buffer area, calculated in bytes
 */
unsigned int SM_CalcEventBufferByMaxEntries(unsigned int maxEntries);

/*@}*/

#endif
