/**
 * @file Eng_KeyMapping.h
 * @brief This file is for event process function prototype
 * 
 */


#ifndef __ENG_KEY_MAPPING_H__
#define __ENG_KEY_MAPPING_H__

#include "anyka_types.h"
#include "fwl_keyhandler.h"

typedef enum {
    fkNULL = 0,

    /* Public state key define */
    fkPUBLIC_POWER_OFF,
    fkPUBLIC_KEY_LOCK,
    fkPUBLIC_VOICE_UP,
    fkPUBLIC_VOICE_DOWN,
    fkPUBLIC_VOICE_UP_FAST,
    fkPUBLIC_VOICE_DOWN_FAST,
    
#ifdef CHRONTEL
		fkPUBLIC_TV_EN,
		fkPUBLIC_TV_SWITCH,
#endif    
    
#ifdef SUPPORT_TVOUT    
    fkPUBLIC_TV_EN,
    fkPUBLIC_TV_SWITCH,
#endif    

    /* Sleep state key define */
    fkSLEEP_POWER_ON,

    /* Standby key define */
    fkSTDBY_MENU,
    fkSTDBY_HIDE_BAR,
    fkSTDBY_POWEROFF,

    /* Menu control key define */
    fkMENU_UP_ITEM,     fkMENU_DOWN_ITEM,
    fkMENU_UP_LINE,     fkMENU_DOWN_LINE,
    fkMENU_EXIT,        fkMENU_HOME,
    fkMENU_SELECT,

    /* Message box control key define */
    fkMBOX_UP_LINE,     fkMBOX_DOWN_LINE,
    fkMBOX_UP_PAGE,     fkMBOX_DOWN_PAGE,
    fkMBOX_BUTTON_LEFT, fkMBOX_BUTTON_RIGHT,
    fkMBOX_EXIT,        fkMBOX_HOME,
    fkMBOX_COMPLETE,

    /*Camera model key define*/
    fkCAMERA_MENU,
    fkCAMERA_CAPTURE,
    fkCAMERA_ZOOM_IN,
    fkCAMERA_ZOOM_OUT,
    fkCAMERA_BRIGHT_UP,
    fkCAMERA_BRIGHT_DOWN,
    fkCAMERA_SWICH_2_DC,
    fkCAMERA_SWICH_2_DV,
    fkCAMERA_EXIT,

    MAX_FUNCKEY_NUM
} T_eFUNC_KEY;

typedef enum
{
    EB_STAY = 0,                  
    EB_RETURN,                
    EB_RETURN_HOME,
    EB_MENU,               
    EB_UP_LINE = 4,
    EB_DOWN_LINE,
    EB_UP_PAGE,
    EB_DOWN_PAGE,
    EB_SCROLLINES,
    EB_GO_TOP,
    EB_ATTRBT_REFRESH
} T_EB_ACTION;


T_eFUNC_KEY MappingPublicKey(T_MMI_KEYPAD phyKey);
T_eFUNC_KEY MappingSleepKey(T_MMI_KEYPAD phyKey);
T_eFUNC_KEY MappingStdbyKey(T_MMI_KEYPAD phyKey);
T_eFUNC_KEY MappingMenuKey(T_MMI_KEYPAD phyKey);
T_eFUNC_KEY MappingMsgboxKey(T_MMI_KEYPAD phyKey);
T_eFUNC_KEY MappingCameraKey(T_MMI_KEYPAD phyKey);
T_EB_ACTION MappingEbookKey(T_MMI_KEYPAD phyKey);

#endif

