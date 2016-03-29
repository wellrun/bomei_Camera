/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef OS_WIN32

#include <stdlib.h>
#include <assert.h>

#include "vme_interface.h"

#include "vme_engine.h"
#include "vme_event.h"



/*****************************************************************************
 * vme interface
 *****************************************************************************
*/

vVOID VME_SetCallback (vT_MainCallback maincallback)
{
    assert (maincallback != NULL);

    // save functionpointer for callback
    vme_engine_SetMainCallBack (maincallback);
    
    return;
}

vVOID VME_Terminate (vVOID)
{
    vme_engine_Term ();
}

vVOID VME_ReTriggerEvent(vT_EvtSubCode event, vUINT32 param)
{
    vme_event_Send_VME_EVT_RETRIGGER (event, param);

    return;
}

vVOID VME_ReTriggerUniqueEvent(vT_EvtSubCode event, vUINT32 param)
{
    vme_event_Send_VME_EVT_RETRIGGER (event, param);

    return;
}
#endif
