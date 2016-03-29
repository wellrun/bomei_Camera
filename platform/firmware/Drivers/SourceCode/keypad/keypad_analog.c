/**
 * @FILENAME: keypad_analog.c
 * @BRIEF ananlog key function file, provide keypad APIs: init, enable...
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR luheshan
 * @DATE 2012-02-28
 * @VERSION 1.0
 */
#include <string.h>
#include "anyka_types.h"
#include "platform_hd_config.h"
#include "hal_keypad.h"
#include "keypad_define.h"
#include "drv_keypad.h"
#include "drv_in_callback.h"
#include "drv_timer.h"

#if (KEYPAD_TYPE == 1)

//长按键按下的时间长
#define AD_LONG_TIME_PRESS             1000          //1s

#define AD_SHORT_TIME_PRESS         40
//主动检查AD值TIMER 的时间间隔
#define KEY_CHECK_TIME    (60) //100 ms

#define KEY_LOOP_TIME    (8) //20 ms

#define KEY_LONG_TIME    (100) //100 ms

#define PREVENT_DITHER_TIME    (2*KEY_LOOP_TIME)

//两次取出的AD值的最大差值
#define    KEY_OUT_REF_OFFSET    15

#define KEYPAD_DEBUG gb_drv_cb_fun.akprintf
//#define KEYPAD_DEBUG 

#define KEY_ID_INVALID  (0XFF) // invalid key ID
//To check two AD value is out of offset 
#define IS_OUT_REF_VAL(val,ref,offset)  (((val)< ((ref)-(offset))) || ((val)> ((ref)+(offset))))

typedef enum
{
    TRIGGER_PROC_NULL,
    TRIGGER_PROC_FAIL,//had sampled ,But fail
    TRIGGER_PROC_OK, // had sampled and OK
    TRIGGER_PROC_SAMPLE,//in process of sampling
} TRIGGER_PROC_STATUS;

typedef struct _KEYPAD_CONTROL_ {
    T_AD_VAL            AdKeyHwValBuffer[PREVENT_DITHER_TIME/KEY_LOOP_TIME];// prevent dither signal buffer
    KEY_DETECT_STR      *KeyAvlArray;
    T_TIMER             mCheckTimerId;
    T_TIMER             mLongKeyTimerId;
    T_TIMER             mLoopKeyTimerId;
    T_eKEY_PRESSMODE    PressMode;
    TRIGGER_PROC_STATUS GetAdState;
    T_U32               KeypadPressTime;
    T_U32               gAdHwkeyValCount; //how many times to sampl of AD
    T_U32               keyMaxNum;
    T_U32               PreventTimes;
    T_U32               AdValMin;
    T_U32               AdValMax;
    T_AD_VAL            gAdOldVal;
    T_AD_VAL            AdValPrevent;
    T_ID_VAL            gCurKeyId;
    T_BOOL              key_init;
} T_KEYPAD_CONTROL;


typedef struct _HARDWARE_KEYPAD_ {
    T_KEYPAD_CONTROL         control;
    T_f_H_KEYPAD_CALLBACK    callback_func;  // global pointer to the timer callback function 
} T_HARDWARE_KEYPAD;

static T_HARDWARE_KEYPAD keypad = {0};

static T_VOID keypad_init_analog(T_f_H_KEYPAD_CALLBACK callback_func, const T_PLATFORM_KEYPAD_PARM *keypad_parm);
static T_S32 keypad_scan_analog(T_VOID);
static T_VOID keypad_enable_scan_analog(T_VOID);
static T_VOID keypad_disable_scan_analog(T_VOID);
static T_eKEY_PRESSMODE keypad_get_pressmode_analog(T_VOID);
static T_BOOL keypad_set_pressmode_analog(T_eKEY_PRESSMODE PressMode);
static T_VOID keypad_set_delay_analog(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay);

static T_BOOL AD_Hw_Read_AD(T_AD_VAL *AdVal);
static T_BOOL AD_Get_Cur_Key_ID(T_ID_VAL *KeyId, T_AD_VAL AdVal);
static TRIGGER_PROC_STATUS AD_Get_AdVal_Prevent(T_AD_VAL * AdVal);
static T_VOID AD_Send_Key_ID(T_ID_VAL KeyId, T_BOOL longPress, T_KEYPADSTATUS state);
static T_VOID AD_LongKey_Timer(T_TIMER timer_id, T_U32 delay);
static T_VOID AD_LoopKey_Timer(T_TIMER timer_id, T_U32 delay);
static T_VOID AD_Check_Key_Timer(T_TIMER timer_id, T_U32 delay);

/**
 * @brief Initialize keypad
 * If pointer callback_func is not equal AK_NULL, the keypad interrupt will be enabled
 * Function keypad_init() must be called before call any other keypad functions
 * @author LHS
 * @date 2012-02-28
 * @param T_fKEYPAD_CALLBACK callback_func: Keypad callback function
 * @return T_VOID
 * @retval
 */
static T_VOID keypad_init_analog(T_f_H_KEYPAD_CALLBACK callback_func, const T_PLATFORM_KEYPAD_PARM *keypad_parm)
{
    memset(&keypad, 0, sizeof(keypad));

    if ((AK_NULL == callback_func)
        || (AK_NULL == keypad_parm)
        || (AK_NULL == keypad_parm->PARM_ANALOG.key_avl_array)
        )
    {
        if (AK_NULL != gb_drv_cb_fun.akprintf)
        {
            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad init error\r\n");
        }
        
        keypad.control.key_init = AK_FALSE;
        return;
    }
    
    keypad.callback_func = callback_func;    
    keypad.control.KeyAvlArray = keypad_parm->PARM_ANALOG.key_avl_array;
    keypad.control.keyMaxNum = keypad_parm->PARM_ANALOG.key_max_num;
    keypad.control.AdValMin = keypad_parm->PARM_ANALOG.AdValMin;
    keypad.control.AdValMax = keypad_parm->PARM_ANALOG.AdValMax;
    keypad.control.mCheckTimerId = ERROR_TIMER;
    keypad.control.mLongKeyTimerId = ERROR_TIMER;
    keypad.control.mLoopKeyTimerId = ERROR_TIMER;
    keypad.control.PressMode = eSINGLE_PRESS;
    keypad.control.KeypadPressTime = KEY_CHECK_TIME;
    keypad.control.gAdHwkeyValCount = 0;
    keypad.control.PreventTimes = PREVENT_DITHER_TIME/KEY_LOOP_TIME;
    keypad.control.gCurKeyId = KEY_ID_INVALID;
    keypad.control.GetAdState = TRIGGER_PROC_NULL;
    keypad.control.key_init = AK_TRUE;
    
    AD_Hw_Read_AD(&keypad.control.gAdOldVal);

    keypad_enable_scan_analog(); //open analog keypad check timer;
    
}

/**
 * @BRIEF Scan keypad
 * Function keypad_init() must be called before call this function
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_VOID
 * @RETURN T_S32: The pressed key's scan code
 * @RETVAL
 */
static T_S32 keypad_scan_analog(T_VOID)
{
    T_S32 KeyId = KEY_ID_INVALID;
    T_AD_VAL AdVal;
    
    if (AD_Hw_Read_AD(&AdVal))
    {
        if (!AD_Get_Cur_Key_ID(&KeyId,AdVal))
        {
            if (AK_NULL != gb_drv_cb_fun.akprintf)
            {
                gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad enable timer fail\r\n");
            }
        }
    }

    return KeyId;
}

/**
 * @BRIEF enable keypad scan
 * Function keypad_init() must be called before call this function
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID keypad_enable_scan_analog(T_VOID)
{
    if(ERROR_TIMER != keypad.control.mCheckTimerId)
    {
        vtimer_stop(keypad.control.mCheckTimerId);
        keypad.control.mCheckTimerId = ERROR_TIMER;
    }
        
    keypad.control.mCheckTimerId = vtimer_start(KEY_CHECK_TIME, AK_TRUE, AD_Check_Key_Timer);
    
    if(ERROR_TIMER == keypad.control.mCheckTimerId)
    {
        if (AK_NULL != gb_drv_cb_fun.akprintf)
        {
            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad enable timer fail\r\n");
        }
    }
}

/**
 * @BRIEF Disable keypad scan
 * Function keypad_init() must be called before call this function
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID keypad_disable_scan_analog(T_VOID)
{
    if(ERROR_TIMER != keypad.control.mCheckTimerId)
    {
        vtimer_stop(keypad.control.mCheckTimerId);
        keypad.control.mCheckTimerId = ERROR_TIMER;
    }
}

/**
 * @BRIEF get  the mode of keypad
 * Function keypad_init() must be called before call this function
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_VOID
 * @RETURN T_eKEY_PRESSMODE single key or multiple key,the analog key must be single key mode
 * @RETVAL
 */
static T_eKEY_PRESSMODE keypad_get_pressmode_analog(T_VOID)
{
    if (AK_FALSE == keypad.control.key_init)
    {
        if (AK_NULL != gb_drv_cb_fun.akprintf)
        {
            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad is not init \r\n");
        }
       
        return eKEYPRESSMODE_NUM;
    }

    return keypad.control.PressMode;
}

/**
 * @BRIEF set  the mode of keypad
 * Function keypad_init() must be called before call this function
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_eKEY_PRESSMODE single key or multiple key,the analog key must be single key mode
 * @RETURN T_BOOL key no init will return false
 * @RETVAL
 */
static T_BOOL keypad_set_pressmode_analog(T_eKEY_PRESSMODE PressMode)
{
    if (AK_FALSE == keypad.control.key_init)
    {
        if (AK_NULL != gb_drv_cb_fun.akprintf)
        {
            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad is not init \r\n");
        }
       
        return AK_FALSE;
    }

    keypad.control.PressMode = PressMode;

    return AK_TRUE;
}

/**
 * @BRIEF Set keypad delay parameter
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_S32 keylong_delay: long key delay time (millisecond),must >0
 * @PARAM T_S32 keydown_delay: no need
 * @PARAM T_S32 keyup_delay: no need
 * @PARAM T_S32 loopkey_delay: no need
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID keypad_set_delay_analog(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay)
{
    if (AK_FALSE == keypad.control.key_init)
    {
        if (AK_NULL != gb_drv_cb_fun.akprintf)
        {
            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad is not init \r\n");
        }
      
        return;
    }
}

/**
 * @BRIEF to get the AD5 value
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_VOID
 * @RETURN T_BOOL true is valid
 * @RETVAL
 */
static T_BOOL AD_Hw_Read_AD(T_AD_VAL *AdVal)
{
    T_AD_VAL hwval = 0;
    T_BOOL   ret = AK_FALSE;

    hwval = (T_AD_VAL)analog_getvalue_ad5();

    if ((hwval >= keypad.control.AdValMin) \
         && hwval <=keypad.control.AdValMax)
    {
        *AdVal = hwval;
        ret = AK_TRUE;
    }

    return ret;
}

/**
 * @BRIEF to get the key ID By AD5 value
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_ID_VAL KeyId,will return the keyId number
 * @PARAM T_AD_VAL AdVal,ad value
 * @RETURN T_BOOL true is OK.
 * @RETVAL
 */
static T_BOOL AD_Get_Cur_Key_ID(T_ID_VAL *KeyId, T_AD_VAL AdVal)
{   
    T_U32 idx;
    T_BOOL ret = AK_FALSE;

    if (AK_NULL == KeyId)
    {
        return ret;
    }
    
    for(idx=0; idx<keypad.control.keyMaxNum; ++idx)
    {
        if((AdVal >= keypad.control.KeyAvlArray[idx].Min) \
            && (AdVal <= keypad.control.KeyAvlArray[idx].Max))  
        {
            (*KeyId) = keypad.control.KeyAvlArray[idx].KeyID;
            ret = AK_TRUE;
            break;
        }
    }

    return ret;
}


/**
 * @BRIEF to get the key ID By AD5 value,this function  can prevent dither signal
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_AD_VAL AdVal,will return the ad value
 * @RETURN TRIGGER_PROC_STATUS will return sampling state
 * @RETVAL
 */
static TRIGGER_PROC_STATUS AD_Get_AdVal_Prevent(T_AD_VAL * AdVal)
{
    T_KEYPAD_CONTROL *control = &keypad.control;
    T_U32   idx;
    T_U32   AdValAverage = 0;
    T_AD_VAL oldAdval = keypad.control.gAdOldVal;
    T_AD_VAL curAdval;
    T_BOOL ret = AK_FALSE;

    ret = AD_Hw_Read_AD(&curAdval);
    control->AdKeyHwValBuffer[control->gAdHwkeyValCount] = curAdval;
    control->gAdHwkeyValCount++;
    
    //如果采集到的AD值相互误差过大，则重新采样
    if(IS_OUT_REF_VAL(curAdval,oldAdval,KEY_OUT_REF_OFFSET) && !ret)
    {
        gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "key is invalidation 3\r\n");
        control->gAdHwkeyValCount = 0;
        return TRIGGER_PROC_FAIL;
    }
    
    if (control->gAdHwkeyValCount < control->PreventTimes)
    {
        return TRIGGER_PROC_SAMPLE;
    }
    else //prevent sampling is OK
    {       
        for (idx=0; idx<control->PreventTimes; idx++)
        {
            if(IS_OUT_REF_VAL(control->AdKeyHwValBuffer[idx],
                                control->AdKeyHwValBuffer[0],KEY_OUT_REF_OFFSET))
            {
                gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "key is invalidation 4\r\n");
                control->gAdHwkeyValCount = 0;
                return TRIGGER_PROC_FAIL;
            }
            
            AdValAverage += control->AdKeyHwValBuffer[idx];
        }
        
        *AdVal = (AdValAverage/(control->PreventTimes)) \
                    +(AdValAverage%(control->PreventTimes));
        
        control->gAdHwkeyValCount = 0;
        memset(control->AdKeyHwValBuffer,0,sizeof(T_AD_VAL)*control->PreventTimes);

        return TRIGGER_PROC_OK;
    }
}

/**
 * @BRIEF to send key ID by callback func
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_ID_VAL KeyId,  Key number
 * @PARAM T_BOOL longPress,  true is long key
 * @PARAM T_KEYPADSTATUS state ,key press is state
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID AD_Send_Key_ID(T_ID_VAL KeyId, T_BOOL longPress, T_KEYPADSTATUS state)
{
    T_KEYPAD key;
    
    key.keyID = KeyId;
    key.longPress = longPress;
    key.status = state;
    keypad.callback_func(&key); 
}

/**
 * @BRIEF to check long key is up delay timer 
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_TIMER timer_id,
 * @PARAM T_U32 delay
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID AD_LongKey_Timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &keypad.control;
    T_ID_VAL KeyId = KEY_ID_INVALID;
    T_AD_VAL AdVal;
    T_BOOL   ret;

    AD_Hw_Read_AD(&AdVal);
    ret = AD_Get_Cur_Key_ID(&KeyId,AdVal);

    if (control->gCurKeyId != KeyId) //key is up
    {
        if (ERROR_TIMER != control->mLongKeyTimerId)
        {
            vtimer_stop(control->mLongKeyTimerId);
            control->mLongKeyTimerId = ERROR_TIMER;
        }
        
        AD_Send_Key_ID(control->gCurKeyId,0,eKEYUP);//send  key UP
        control->gCurKeyId = KEY_ID_INVALID;
        control->KeypadPressTime = KEY_CHECK_TIME;
        keypad_enable_scan_analog();
    }
    else //key is press,and send short key
    {
        control->KeypadPressTime += delay;
        if (control->KeypadPressTime >= AD_LONG_TIME_PRESS)
        {
            control->KeypadPressTime = KEY_CHECK_TIME;
            AD_Send_Key_ID(control->gCurKeyId,0,eKEYPRESS);//send short key
        }
    }
}

/**
 * @BRIEF to check key press state timer,this function will send key message 
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_TIMER timer_id,
 * @PARAM T_U32 delay
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID AD_LoopKey_Timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &keypad.control;
    T_AD_VAL AdOldAvl = keypad.control.gAdOldVal;
    T_ID_VAL KeyId = KEY_ID_INVALID;
    T_AD_VAL AdCurAvl;
    T_BOOL   ret;

    if (TRIGGER_PROC_OK != control->GetAdState) // go to sampling
    {
        control->GetAdState = AD_Get_AdVal_Prevent(&control->AdValPrevent);
    }

    if (TRIGGER_PROC_OK == control->GetAdState) //sampling OK
    {
        AD_Hw_Read_AD(&AdCurAvl);
        
        if (IS_OUT_REF_VAL(AdCurAvl,AdOldAvl,KEY_OUT_REF_OFFSET))//key is up
        {
            ret = AD_Get_Cur_Key_ID(&KeyId,control->AdValPrevent);
            
            if (ERROR_TIMER != control->mLoopKeyTimerId)
            {
                vtimer_stop(control->mLoopKeyTimerId);
                control->mLoopKeyTimerId = ERROR_TIMER;
            }

            if (ret && (control->KeypadPressTime >= AD_SHORT_TIME_PRESS) \
                    && (control->KeypadPressTime < AD_LONG_TIME_PRESS))
            {
                AD_Send_Key_ID(KeyId,0,eKEYPRESS);//send short key
            }
            else //key is invalidation
            {
                if (AK_NULL != gb_drv_cb_fun.akprintf)
                {
                    gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "key is invalidation 1\r\n");
                }
            }
            
            control->KeypadPressTime = KEY_CHECK_TIME;
            control->GetAdState = TRIGGER_PROC_NULL;
            keypad_enable_scan_analog();
        }
        else //key is press
        {
            control->KeypadPressTime += delay;

            if (control->KeypadPressTime >= AD_LONG_TIME_PRESS)
            {
                ret = AD_Get_Cur_Key_ID(&KeyId,control->AdValPrevent);

                if (ERROR_TIMER != control->mLoopKeyTimerId)
                {
                    vtimer_stop(control->mLoopKeyTimerId);
                    control->mLoopKeyTimerId = ERROR_TIMER;
                }
                
                if (ERROR_TIMER != control->mLongKeyTimerId)
                {
                    vtimer_stop(control->mLongKeyTimerId);
                    control->mLongKeyTimerId = ERROR_TIMER;
                }
                
                control->KeypadPressTime = KEY_CHECK_TIME;
                control->GetAdState = TRIGGER_PROC_NULL;
                
                if (ret)
                {
                    AD_Send_Key_ID(KeyId,1,eKEYPRESS);//send long key
                    control->gCurKeyId = KeyId;
                    
                    control->mLongKeyTimerId = vtimer_start(KEY_LONG_TIME, AK_TRUE, AD_LongKey_Timer);
                    
                    if(ERROR_TIMER == control->mLongKeyTimerId)
                    {
                        if (AK_NULL != gb_drv_cb_fun.akprintf)
                        {
                            gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad enable longkey timer fail\r\n");
                        }
                    }
                }
                else //key is invalidation
                {
                    keypad_enable_scan_analog();
                    if (AK_NULL != gb_drv_cb_fun.akprintf)
                    {
                        gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "key is invalidation 2\r\n");
                    }
                }
            }
        }
    }
    else if (TRIGGER_PROC_FAIL == control->GetAdState)
    {
        if (ERROR_TIMER != control->mLoopKeyTimerId)
        {
            vtimer_stop(control->mLoopKeyTimerId);
            control->mLoopKeyTimerId = ERROR_TIMER;
        }
        control->KeypadPressTime = KEY_CHECK_TIME;
        control->GetAdState = TRIGGER_PROC_NULL;
        keypad_enable_scan_analog();
    }
}

/**
 * @BRIEF to check key state timer,Open key will enable this timer
 * @AUTHOR LHS
 * @DATE 2012-02-28
 * @PARAM T_TIMER timer_id,
 * @PARAM T_U32 delay
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID AD_Check_Key_Timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &keypad.control;
    T_AD_VAL AdOldAvl = keypad.control.gAdOldVal;
    T_AD_VAL AdCurAvl;
    T_ID_VAL KeyId = KEY_ID_INVALID;
    T_BOOL ret;

    ret = AD_Hw_Read_AD(&AdCurAvl);

    //if gAdHwkeyValCount > 0,key is sampling no need to save the key frist virtual value.
    if (0 == control->gAdHwkeyValCount)
    {
        control->gAdOldVal = AdCurAvl;
    }

    //AD value is change,the key is down
    if (IS_OUT_REF_VAL(AdCurAvl,AdOldAvl,KEY_OUT_REF_OFFSET) \
            && ret && AD_Get_Cur_Key_ID(&KeyId,AdCurAvl))
    {
        if(ERROR_TIMER != control->mLoopKeyTimerId)
        {
            vtimer_stop(control->mLoopKeyTimerId);
            control->mLoopKeyTimerId = ERROR_TIMER;
        }
        
        keypad_disable_scan_analog();
            
        control->mLoopKeyTimerId = vtimer_start(KEY_LOOP_TIME, AK_TRUE, AD_LoopKey_Timer);
        
        if(ERROR_TIMER == control->mLoopKeyTimerId)
        {
            keypad_enable_scan_analog();
            
            if (AK_NULL != gb_drv_cb_fun.akprintf)
            {
                gb_drv_cb_fun.akprintf(C2, M_DRVSYS, "keypad enable loopkey timer fail\r\n");
            }
        }
    }
}

//******************************reg my handle info***************************//
static T_KEYPAD_HANDLE analog_handler = 
{
    keypad_init_analog,
    keypad_scan_analog,
    keypad_enable_scan_analog, 
    keypad_disable_scan_analog, 
    keypad_get_pressmode_analog,
    keypad_set_pressmode_analog,
    keypad_set_delay_analog
};

static int analog_reg(void)
{
    keypad_reg_scanmode(KEYPAD_ANALOG,&analog_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(analog_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif
