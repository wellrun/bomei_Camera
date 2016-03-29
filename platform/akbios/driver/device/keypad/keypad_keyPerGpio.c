#include "akdefine.h"
#include "drv_api.h"
#include "keypad_type.h"
#include "drv_keypad.h"
#include "drv_timer.h"
#include "drv_gpio.h"

#ifdef MOUNT_KEYPERGPIO_SCAN_MODE 

#define KEYPAD_TIMER        uiTIMER1


static T_eKEY_PRESSMODE CurrentPressMode = eSINGLE_PRESS;

/*
扫描一个键盘
*/
const T_PLATFORM_KEYPAD_PARM *pKeypadPara=AK_NULL;
static T_f_H_KEYPAD_CALLBACK sys_keypad_callback = AK_NULL;

static T_U32 ShakeTime=100;
static T_U32 LongPressTime=500;

#define LONGPRESSCNTFLAG (LongPressTime/ShakeTime + 5)
/*
当按下时间计数为0xff时，判断为该gpio被Mask
*/
#define GPIO_MASK_FLAG 0xff

static T_TIMER KeypadTimer = ERROR_TIMER;
static T_TIMER KeypadLockTimer = ERROR_TIMER;

static T_S32 keypad_scan_KeyPerGpio(T_VOID) 
{
    T_U32 i;
    
    //akprintf(C3, M_DRVSYS, "start scan keypad\n");
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if (gpio_get_pin_level(pKeypadPara->RowGpio[i]) == pKeypadPara->active_level)        
            return pKeypadPara->keypad_matrix[i];
    }
    
    //return kbUnKnown;
    return 0xffffffff;
}



static T_VOID keypad_enable_intr_KeyPerGpio(T_VOID)
{
    T_U32 i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;    
            
        gpio_int_control(pKeypadPara->RowGpio[i], AK_TRUE);
    }    
}


static T_VOID keypad_disable_intr_KeyPerGpio(T_VOID)
{
    T_U32 i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;    
    
        gpio_int_control(pKeypadPara->RowGpio[i], AK_FALSE);
    }
}



static T_BOOL keypad_SetMode_KeyPerGpio(T_eKEY_PRESSMODE press_mode)
{
    CurrentPressMode = press_mode;
    return AK_TRUE;
}

static T_eKEY_PRESSMODE keypad_GetMode_KeyPerGpio(T_VOID)
{
     return CurrentPressMode;
}


static T_VOID keypad_timer_scan(T_TIMER timer_id, T_U32 delay)
{
    T_U32 i,j;
    T_BOOL isSomeKeyPress;
    T_KEYPAD keypad;
    
    isSomeKeyPress = AK_FALSE;
    //save press cnt
    for(i=0;i<pKeypadPara->row_qty;i++)
    {   
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;    
        
        if (gpio_get_pin_level(pKeypadPara->RowGpio[i]) == pKeypadPara->active_level)//down       
        {
            if(0 == pKeypadPara->updown_matrix[i])//this key is first down
            {
                //send down   
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_FALSE;         
                keypad.status = eKEYDOWN;
                sys_keypad_callback(&keypad);
            }
            else if(pKeypadPara->updown_matrix[i] < (LongPressTime/ShakeTime))//send long
            {
                //no action,wait long comming,or short up
            }
            else if(pKeypadPara->updown_matrix[i] == (LongPressTime/ShakeTime))//send long
            {
                //send long+press
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_TRUE;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
            }
            else
            {
                //send press after long flag
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_FALSE;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
                
                pKeypadPara->updown_matrix[i] = LONGPRESSCNTFLAG;//Prevent overflow    
            }
            
            (pKeypadPara->updown_matrix[i])++;
            isSomeKeyPress = AK_TRUE;
            
//            if (eSINGLE_PRESS == CurrentPressMode)//single mode,send one key
//                break;
        }
        else//up
        {
            if (pKeypadPara->updown_matrix[i] == 0)//up of long press
            {
                //no press,no action
            }
            else if (pKeypadPara->updown_matrix[i] < (LongPressTime/ShakeTime))
            {
                //short press up,send press and up
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_FALSE;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
                
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_FALSE;         
                keypad.status = eKEYUP;
                sys_keypad_callback(&keypad);
            }
            else
            {
                //long press up, send up 
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = AK_FALSE;         
                keypad.status = eKEYUP;
                sys_keypad_callback(&keypad);
            }
            
            pKeypadPara->updown_matrix[i] = 0;   
        }
    }
    
    //No key press
    if (!isSomeKeyPress)
    {
        //akprintf(C3, M_DRVSYS, "no key press,out timer\n");
        timer_stop(KeypadTimer);
        KeypadTimer = ERROR_TIMER;
        keypad_enable_intr_KeyPerGpio();
    }
}

static T_VOID keypad_gpio_callback(T_U32 pin, T_U8 polarity)
{
    T_U32 j;
    
    keypad_disable_intr_KeyPerGpio();//close all gpio intr,use timer scan
    
    //akprintf(C3, M_DRVSYS, "keypad gpio shake,PIN=%d",pin);
    
    if(KeypadTimer != ERROR_TIMER)
    {
        akprintf(C3, M_DRVSYS, "keypad scan is running,PIN=%d\n",pin);
        keypad_enable_intr_KeyPerGpio();
        return;
    }
    
    KeypadTimer = timer_start(KEYPAD_TIMER,ShakeTime, AK_TRUE, keypad_timer_scan);
    if (ERROR_TIMER == KeypadTimer)
    {
        akprintf(C3, M_DRVSYS, "can't malloc timer in keypad_gpio_callback(keypergpio keypad)\n");
        keypad_enable_intr_KeyPerGpio();
        return;
    }
}




//**************************lock handle*************************************//

static T_VOID keypad_locktimer_scan(T_TIMER timer_id, T_U32 delay)
{
    T_U32 i;
    T_KEYPAD keypad;
    
    if(KeypadLockTimer != ERROR_TIMER)
    {
        timer_stop(KeypadLockTimer);
        KeypadLockTimer = ERROR_TIMER;
    }
    
    if (gpio_get_pin_level(pKeypadPara->switch_key_id) == pKeypadPara->switch_key_active_level)       
    {
        akprintf(C3, M_DRVSYS, "Keypad is locked by pin %d",pKeypadPara->switch_key_id);
        
        for(i=0;i<pKeypadPara->row_qty;i++)
        {
            if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            {
                akprintf(C3, M_DRVSYS, "pin %d is been masked",pKeypadPara->updown_matrix[i]);
                continue;
            }
            
            gpio_int_control(pKeypadPara->RowGpio[i], AK_FALSE);
        }    
        
        
        //send hardware lock 
        keypad.keyID = pKeypadPara->switch_key_value; 
        keypad.longPress = AK_TRUE;         
        keypad.status = eKEYPRESS;
        sys_keypad_callback(&keypad);    
        
        //change int polarity
        gpio_set_int_p(pKeypadPara->switch_key_id, 1 - pKeypadPara->switch_key_active_level);     
        
    }
    else
    {
        akprintf(C3, M_DRVSYS, "Keypad locked shake by pin %d",pKeypadPara->switch_key_id);
    }
    
    gpio_int_control(pKeypadPara->switch_key_id, AK_TRUE);
}

static T_VOID keypad_unlocktimer_scan(T_TIMER timer_id, T_U32 delay)
{
    T_U32 i;
    T_KEYPAD keypad;
    
    if(KeypadLockTimer != ERROR_TIMER)
    {
        timer_stop(KeypadLockTimer);
        KeypadLockTimer = ERROR_TIMER;
    }
    
    if (gpio_get_pin_level(pKeypadPara->switch_key_id) == (1-pKeypadPara->switch_key_active_level))       
    {
        akprintf(C3, M_DRVSYS, "Keypad is unlocked by pin %d",pKeypadPara->switch_key_id);
        
        for(i=0;i<pKeypadPara->row_qty;i++)
        {
            if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            {
                akprintf(C3, M_DRVSYS, "pin %d is been masked",pKeypadPara->updown_matrix[i]);
                continue;
            }
            
            pKeypadPara->updown_matrix[i] = 0;
            gpio_int_control(pKeypadPara->RowGpio[i], AK_TRUE);
        }    
        
        
        //send hardware UNlock 
        keypad.keyID = pKeypadPara->switch_key_value; 
        keypad.longPress = AK_FALSE;         
        keypad.status = eKEYPRESS;
        sys_keypad_callback(&keypad);       
        
        //change int polarity
        gpio_set_int_p(pKeypadPara->switch_key_id, pKeypadPara->switch_key_active_level); 
        
    }
    else
    {
        akprintf(C3, M_DRVSYS, "Keypad unlocked shake by pin %d",pKeypadPara->switch_key_id);
    }
    
    gpio_int_control(pKeypadPara->switch_key_id, AK_TRUE);
}


static T_VOID keypad_lockgpio_callback(T_U32 pin, T_U8 polarity)
{
    akprintf(C3, M_DRVSYS, "keypad_lockgpio_callback,pin = %d",pin);
    
    gpio_int_control(pin, AK_FALSE);
    if(KeypadLockTimer != ERROR_TIMER)
    {
        akprintf(C3, M_DRVSYS, "KeypadLockTimer is running,PIN=%d\n",pin);
        gpio_int_control(pin, AK_TRUE);
        return;
    }
    
    if (polarity == pKeypadPara->switch_key_active_level)
        KeypadLockTimer = timer_start(KEYPAD_TIMER,LongPressTime, AK_FALSE, keypad_locktimer_scan);
    else
        KeypadLockTimer = timer_start(KEYPAD_TIMER,LongPressTime, AK_FALSE, keypad_unlocktimer_scan);
    
    
    if (ERROR_TIMER == KeypadLockTimer)
    {
        akprintf(C3, M_DRVSYS, "can't malloc timer in keypad_lockgpio_callback(keypergpio keypad)\n");
        gpio_int_control(pin, AK_TRUE);
        return;
    }
}


static T_VOID keypad_init_KeyPerGpio(T_f_H_KEYPAD_CALLBACK callback_func,const T_PLATFORM_KEYPAD_PARM *keypad_parm)
{
    T_U32 i;
    
/*
    check parameter,in here,we only chage updown_matrix
*/    
    if (AK_NULL==keypad_parm->updown_matrix)
    {
        akprintf(C3, M_DRVSYS, "keypad_parm->updown_matrix is null\n");
    }    
    
    pKeypadPara = keypad_parm;
    KeypadTimer = ERROR_TIMER;
    sys_keypad_callback = callback_func;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        pKeypadPara->updown_matrix[i] = 0;
        
        gpio_set_pin_dir(pKeypadPara->RowGpio[i], 0);//input
        gpio_set_pull_up_r(pKeypadPara->RowGpio[i], AK_FALSE);//closepullup
        gpio_set_pull_down_r(pKeypadPara->RowGpio[i], AK_TRUE);//openpulldown
    }
    //register callback
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        gpio_register_int_callback(pKeypadPara->RowGpio[i], pKeypadPara->active_level, 0, keypad_gpio_callback);//para=(gpio,high,disable,callback)   
    }
    
    /*next is keypad lock seting*/
    if (pKeypadPara->switch_key_id == INVALID_GPIO)
    {
        akprintf(C3, M_DRVSYS, "lock key gpio is invalid\n");
        return;
    }
    
    gpio_set_pin_dir(pKeypadPara->switch_key_id, 0);//input
    gpio_set_pull_up_r(pKeypadPara->switch_key_id, AK_FALSE);//closepullup
    gpio_set_pull_down_r(pKeypadPara->switch_key_id, AK_TRUE);//openpulldown
    gpio_register_int_callback(pKeypadPara->switch_key_id, pKeypadPara->switch_key_active_level, 1, keypad_lockgpio_callback);//para=(gpio,high,disable,callback)   
}


static T_VOID keypad_KeyPerGpio_AddMaskGpio(T_U32 pin)
{
    T_U32 i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pin == pKeypadPara->RowGpio[i])
        {
            pKeypadPara->updown_matrix[i] = GPIO_MASK_FLAG;
            gpio_int_control(pin, AK_FALSE);
            //akprintf(C3, M_DRVSYS, "KeyPerGpio_AddMaskGpio success,pin=%d\n",pin);
            return;
        }   
    }
    
    akprintf(C3, M_DRVSYS, "KeyPerGpio_AddMaskGpiofail,pin=%d\n",pin);
}

static T_VOID keypad_KeyPerGpio_RemoveMaskGpio(T_U32 pin)
{
    T_U32 i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pin == pKeypadPara->RowGpio[i])
        {
            pKeypadPara->updown_matrix[i] = 0;
            //akprintf(C3, M_DRVSYS, "open intr,pin=%d\n",pin);
            gpio_int_control(pin, AK_TRUE);
            return;
        }   
    }
    
    akprintf(C3, M_DRVSYS, "pin=%d not add by KeyPerGpio_AddMaskGpio\n",pin);
}

//******************************reg my handle info***************************//

static T_KEYPAD_HANDLE KeyPerGpio_handler = 
{
	keypad_init_KeyPerGpio,
	keypad_scan_KeyPerGpio,
	keypad_enable_intr_KeyPerGpio, 
	keypad_disable_intr_KeyPerGpio, 
	keypad_GetMode_KeyPerGpio,
	keypad_SetMode_KeyPerGpio,
	AK_NULL
};

static int keyPerGpio_reg(void)
{
	keypad_reg_scanmode(KEYPAD_KEY_PER_GPIO,&KeyPerGpio_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(keyPerGpio_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif


