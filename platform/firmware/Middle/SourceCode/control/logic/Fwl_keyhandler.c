/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name£ºfwl_keyhandler.c
 * Function£ºThis file encapsulate some function for handling key pressing 
 *
 * Author£ºMBL
 * Date£º2001-05-15
 * Version£º1.0          
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Fwl_public.h"
#include "fwl_keyhandler.h"
#include "Fwl_EvtMailBox.h"
#include "Fwl_sysevent.h"
#include "Eng_KeyMapping.h"
#include "Eng_MsgQueue.h"
#include "hal_keypad.h"
#include "akos_api.h"
#include "Eng_Debug.h"

#ifdef OS_ANYKA
#include "drv_gpio.h" 
#include "fwl_power.h"
#endif


const T_U8* KeyIdStr[MAX_KEY_NUM] = {
    {"kbUP"},    
    {"kbDOWN"},         
    {"kbLEFT"},         
    {"kbRIGHT"},
    {"kbVOICE_UP"},     
    {"kbVOICE_DOWN"},   
    {"kbMENU"},        
    {"kbOK"},
    {"kbSWA"},          
    {"kbSWB"}, 
    {"kbSWC"},          
    {"kbSWX"},          
    {"kbSWY"},
    {"kbSWZ"},
    {"kbSW3"},          
    {"kbSW4"},          
    {"kbSW5"},          
    {"kbCLEAR"},
    {"kbSTART_MODULE"},    
};

const T_U8* KeyTypeStr[PRESS_TYPE_NUM] = {
    {"    --SHORT"},
    {"    --LONG"},
    {"    --UP"}
};


static T_PRESS_TYPE keytype = PRESS_UP;
static T_BOOL keystop = AK_FALSE;
static T_BOOL longkey = AK_FALSE;
static T_BOOL keyIsvalid = AK_TRUE;

static T_VOID keypad_handle_key(T_KEYPAD *keypad);
static T_VOID keypad_callback_func(T_VOID);


//===============================================================
extern const T_VOID *keypad_get_platform_parm(T_VOID);
extern T_KEYPAD_TYPE keypad_get_platform_type(T_VOID);
#if (KEYPAD_TYPE == 0)
extern T_BOOL keypad_set_number_type_2X3(T_VOID);
extern T_BOOL keypad_set_number_type_3X4(T_VOID);
#endif
///extern T_VOID fc_emu_key_callback(T_MMI_KEYPAD phyKey);
///extern T_VOID snes_emu_key_callback(T_MMI_KEYPAD phyKey);
///extern T_VOID gba_emu_key_callback(T_MMI_KEYPAD phyKey);

#ifdef SUPPORT_AUTOTEST
extern T_BOOL autotest_testflag;
extern T_BOOL autotest_record_statement(T_U8 keyID, T_U8 presstype);
#endif


T_VOID Set_key_valid(T_BOOL flag)
{
    keyIsvalid = flag;
}

T_BOOL Is_key_valid(T_VOID)
{
    return keyIsvalid;
}

/**
 * @brief stop key, stop triggering short key event when long press key 
 * @author zhengwenbo
 * @date 2008-4-9
 * @return T_BOOL whether success
 * @retval AK_TRUE: success  AK_FALSE:fail
 *
*/
T_BOOL keypad_keystop(T_VOID)
{
    if (AK_TRUE == longkey) // if in long key state
    {
        keystop = AK_TRUE;
        return AK_TRUE;
    }
    else
    {    
        return AK_FALSE;
    }
}


//============================================================
static T_VOID keypad_handle_key(T_KEYPAD *keypad)
{
    T_SYS_MAILBOX   mailbox;
    T_MMI_KEYPAD mmikey;

    mailbox.event = 0;
#if 0
    Fwl_Print(C3, M_CONTROL, "key id = %d, longpress = %d, status = %d\n", 
        keypad->keyID, keypad->longPress, keypad->status);
#endif

    switch (keypad->status)
    {
        case eKEYDOWN:
            /*
                        mailbox.event = SYS_EVT_USER_KEY;
                        mmikey.keyID = keypad->keyID;
                        mmikey.pressType = PRESS_SHORT;
                        */
           return;
           break;
           
        case eKEYPRESS:
            mailbox.event = SYS_EVT_USER_KEY;
            mmikey.keyID = keypad->keyID;
            if (AK_TRUE == keypad->longPress)
            {
                mmikey.pressType = PRESS_LONG;
                keytype = PRESS_LONG;
                keystop = AK_FALSE;
                longkey = AK_TRUE;
            }
            else
            {
                if (AK_FALSE == keystop)
                {
                    mmikey.pressType = PRESS_SHORT;
                    keytype = PRESS_SHORT;
                }
                else
                {
                    return;
                }
            }
            break;
            
        case eKEYUP:
            keystop = AK_FALSE;
            
            if (AK_TRUE == longkey)
            {
                mailbox.event = SYS_EVT_USER_KEY;
                mmikey.keyID = keypad->keyID;
                mmikey.pressType = PRESS_UP;
                keytype = PRESS_UP;
                
                longkey = AK_FALSE;
            }
            else
            {
                longkey = AK_FALSE;
                return;
            }
            break;
            
        default:
            break;
    }

    if (kbNULL == mmikey.keyID)
    {
        Fwl_Print(C3, M_KEY_PAD, "kbNULL %s\n", KeyTypeStr[mmikey.pressType]);
    }
    else
    {
        Fwl_Print(C3, M_KEY_PAD, "%s %s\n", KeyIdStr[mmikey.keyID], KeyTypeStr[mmikey.pressType]);

    }

          
    if (gb.PowerOffStatus == AK_TRUE)
    {
        if ((mmikey.keyID == kbCLEAR) && (mmikey.pressType == PRESS_LONG))
        {
        	AkDebugOutput("begin reset ...\n");
            Fwl_DisplayBacklightOff();
            Fwl_DisplayOff();
            
            VME_Reset();
        }

        return;
    }
    
    if (Is_key_valid())
    {
        mailbox.param.c.Param1 = (T_U8)mmikey.keyID;
        mailbox.param.c.Param2 = (T_U8)mmikey.pressType;
    
#ifdef SUPPORT_AUTOTEST
        autotest_record_statement((T_U8)mmikey.keyID, (T_U8)mmikey.pressType);
        //autotest testing not to press any key
        if(autotest_testflag != AK_TRUE)
        {
            AK_PostEventEx(&mailbox, AK_NULL,AK_TRUE, AK_FALSE,AK_TRUE);
        }
#else

        AK_PostEventEx(&mailbox, AK_NULL,AK_TRUE, AK_FALSE,AK_TRUE);
#endif
    }
    else
    {
        Set_key_valid(AK_TRUE);
    }

}

static T_VOID keypad_callback_func(T_VOID)
{
    T_KEYPAD    keypad;

    while (keypad_get_key(&keypad))
    {
// 		AkDebugOutput("=============id=%d=============\n",keypad.keyID);
       keypad_handle_key(&keypad);
    } 
}
//============================================================


void keypad_open(void)
{
    keypad_init(keypad_callback_func, keypad_get_platform_type(), keypad_get_platform_parm());
}

T_VOID keypad_set_loopkey_delay(T_U32 loopkey_delay, T_BOOL all_key_loop)
{
    keypad_set_delay(DEFAULT_KEYDOWN_DELAY, DEFAULT_KEYUP_DELAY, DEFAULT_LONG_KEY_DELAY, DEFAULT_POWERKEY_LONG_DELAY, loopkey_delay);
}


T_VOID Fwl_keypad_change_to_num2x3(T_VOID)
{
#ifdef OS_ANYKA
#if (KEYPAD_TYPE == 0)
    if (keypad_set_number_type_2X3())
    {
        keypad_disable_intr();
        keypad_init(keypad_callback_func, keypad_get_platform_type(), keypad_get_platform_parm());
        keypad_enable_intr();
    }
#endif
#endif 
}

T_VOID Fwl_keypad_change_to_num3x4(T_VOID)
{
#ifdef OS_ANYKA
#if (KEYPAD_TYPE == 0)
    if (keypad_set_number_type_3X4())
    {
        keypad_disable_intr();
        keypad_init(keypad_callback_func, keypad_get_platform_type(), keypad_get_platform_parm());
        keypad_enable_intr();
    }
#endif    
#endif 
}

