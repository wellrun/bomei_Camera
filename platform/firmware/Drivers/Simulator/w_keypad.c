/**
 * @file keypad.c
 * @brief Define keypad driver.
 * This file provides keypad driver APIs: initialization, enable, disable, keypad
 * interrupt handler, and keypad scaning function.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-21
 * @version 1.0
 * @ref
 */
#ifdef OS_WIN32

#include "Gbl_Global.h"
#include "drv_api.h"

#include "eng_keymapping.h"
#include "eng_debug.h"
#include "akdefine.h"

#define KEYPAD_TIMER_DELAY            20

// *** keypad
typedef struct {
    T_U16        left; 
    T_U16        top;
    T_U16        right;
    T_U16        bottom;
    T_eKEY_ID    key_id;
} T_WIN_KEY;

#define KEYPAD_VALUE_BUF_SIZE    32
typedef struct
{
    T_U32    keypad_head;
    T_U32    keypad_tail;
    T_KEYPAD keypad_buf[KEYPAD_VALUE_BUF_SIZE];
} T_KEYPAD_BUF_CTRL;


#define MAX_PHY_KEY_NUM         25

static T_WIN_KEY s_keypad_rect[MAX_PHY_KEY_NUM] = {
    {  0, 0,  1,  1, kbNULL},   //Key 0
    {  0, 0,  1,  1, kbNULL},   //Key 1
    {  0, 0,  1,  1, kbNULL},   //Key 2
    {  0, 0,  1,  1, kbNULL},   //Key 3
    {  0, 0,  1,  1, kbNULL},   //Key 4
    {  0, 0,  1,  1, kbNULL},   //Key 5
    {  0, 0,  1,  1, kbNULL},   //Key 6
    {  0, 0,  1,  1, kbNULL},   //Key 7
    {  0, 0,  1,  1, kbNULL},   //Key 8
    {  0, 0,  1,  1, kbNULL},   //Key 9
    {  0, 0,  1,  1, kbNULL},   //Key Star *
    {  0, 0,  1,  1, kbNULL},   //Key Pond #
    {  45,  70, 57, 40, kbUP},   //Up Arrow
    {  45,  205,57, 40, kbDOWN}, //Down Arrow
    {  20,  140,25, 35, kbLEFT}, //Left Arrow
    {  100, 140,25, 35, kbRIGHT},    //Right Arrow
    {  50,  133,45, 45, kbOK},   //OK
    {  0,   0,  1,  1,  kbNULL},   //Call
    {  45,  10, 55, 25, kbCLEAR},    //HangUp, Switch on/off
#if (KEYPAD_NUM == 10)
    {  650, 242,  55,  25, kbVOICE_DOWN},    //Soft Key 1 -> left,  Cancel
    {  650, 207,  55,  25, kbVOICE_UP}, //Soft Key 2 -> right, Menu
#elif (KEYPAD_NUM == 7)
    {  0, 0,   1,  1, kbNULL},  //Soft Key 1 -> left,  Cancel
    {  0, 0,   1,  1, kbNULL}, //Soft Key 2 -> right, Menu
#endif
    {  650, 10,  55,  25, kbMENU},  //Menu
    {  655, 70,  45,  45, kbSWA},       //A
    {  655, 120, 45,  45, kbSWB},      //B
    {  0,   0,   1,   1,  kbNULL}               //Side Key 4
};


static T_U16        s_cur_mouse_x = 0;
static T_U16        s_cur_mouse_y = 0;
static T_U8         s_cur_mouse_state;

static T_U32        m_prev_key_id = 0;                /* previous key ID */
static T_TIMER      m_timer_id;

static T_fKEYPAD_CALLBACK m_fKeypad_callback_func = AK_NULL;
static T_U32 m_long_key_delay = DEFAULT_LONG_KEY_DELAY;    /* long key delay timer */
static T_U8  m_sent_key_num = 0;                /* key number which be has been sent */

static T_VOID keypad_timer(T_TIMER timer_id, T_U32 delay);
static T_KEYPAD_BUF_CTRL key_ctl;

static T_VOID keypad_send_key(const T_KEYPAD *p_key)
{    
    if (((key_ctl.keypad_tail + 1) & (KEYPAD_VALUE_BUF_SIZE - 1)) == key_ctl.keypad_head)
    {
        Fwl_Print(C3, M_DRVSYS, "keypad_send_key key is full\r\n");
    }

    key_ctl.keypad_buf[key_ctl.keypad_tail] = *p_key;
    key_ctl.keypad_tail = (key_ctl.keypad_tail + 1) & (KEYPAD_VALUE_BUF_SIZE - 1);

    /* callback */
    m_fKeypad_callback_func();
}


T_BOOL keypad_get_key(T_KEYPAD *key)
{
    T_BOOL ret = AK_FALSE;
    
    if (key == AK_NULL
        || (key_ctl.keypad_head == key_ctl.keypad_tail))
    {
        if (AK_NULL != key)
        {
            key->keyID = -1;
        }
        
        return ret;
    }

    *key = key_ctl.keypad_buf[key_ctl.keypad_head];
    key_ctl.keypad_head = (key_ctl.keypad_head + 1) & (KEYPAD_VALUE_BUF_SIZE - 1);
    
    return AK_TRUE;
}


/**
 * @brief Initialize keypad
 * If pointer callback_func is not equal AK_NULL, the keypad interrupt will be enabled
 * Function keypad_init() must be called before call any other keypad functions
 * @author miaobaoli
 * @date 2004-09-21
 * @param T_fKEYPAD_CALLBACK callback_func: Keypad callback function
 * @return T_VOID
 * @retval
 */
 
T_VOID keypad_init(T_fKEYPAD_CALLBACK callback_func, T_U32 type_index, const T_VOID* para)
{
    int iKeypadIndex;

    m_fKeypad_callback_func = callback_func;

    // correct RECT Structure
    for (iKeypadIndex = 0; iKeypadIndex < MAX_PHY_KEY_NUM; iKeypadIndex++)
    {
        s_keypad_rect[iKeypadIndex].right   +=   s_keypad_rect[iKeypadIndex].left;
        s_keypad_rect[iKeypadIndex].bottom  +=   s_keypad_rect[iKeypadIndex].top;
    }

    key_ctl.keypad_head = 0;
    key_ctl.keypad_tail = 0;

    return;
}

const T_VOID *keypad_get_platform_parm(T_VOID)
{
    return AK_NULL;
}

T_KEYPAD_TYPE keypad_get_platform_type(T_VOID)
{
    return KEYPAD_MATRIX_NORMAL;
}


T_VOID keypad_set_delay(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay)
{

}

/**
 * @brief Set keypad parameter
 * @author miaobaoli
 * @date 2004-09-21
 * @param T_U32 delay: long key delay time (millisecond)
 * @return T_VOID
 * @retval
 */
/*
T_VOID keypad_set_parm(T_U32 delay)
{
    m_long_key_delay = delay;
}
*/

/**
 * @brief Scan keypad
 * Function keypad_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-21
 * @param T_VOID
 * @return T_eKEY_ID: The pressed key's ID
 * @retval
 */
T_S32 keypad_scan(T_VOID)
{
    T_eKEY_ID    keyID = kbNULL;
    T_U16        i;

    if (s_cur_mouse_state == 2)
    {
        return kbNULL;
    }

    for (i = 0; i < MAX_PHY_KEY_NUM; i++)
    {
        if (s_cur_mouse_x >= s_keypad_rect[i].left &&
            s_cur_mouse_x < s_keypad_rect[i].right &&
            s_cur_mouse_y >= s_keypad_rect[i].top &&
            s_cur_mouse_y < s_keypad_rect[i].bottom)
        {
            keyID = s_keypad_rect[i].key_id;
            break;
        }
    }

    return keyID;
}

/**
 * @brief Keypad interrupt handler for WIN32
 * If chip detect that KEYPAD GPIO interrupt, this function will be called.
 * Function keypad_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-21
 * @param T_U8 MouseState: 0: mouse down, 1: mouse move, 2: mouse up
 * @return T_BOOL
 * @retval AK_TRUE: keypad interrupt arrive
 */
T_BOOL keypad_interrupt_handler_WIN32(T_U8 MouseState, T_U16 x, T_U16 y)
{
    T_KEYPAD key;

    s_cur_mouse_x = x;
    s_cur_mouse_y = y;
    s_cur_mouse_state = MouseState;

    if( s_cur_mouse_state == 0 )
    {
        m_prev_key_id = keypad_scan();
        if( m_prev_key_id == kbNULL )
        {
            return AK_FALSE;
        }
        m_timer_id = vtimer_start( m_long_key_delay, 0, keypad_timer );
        m_sent_key_num = 0;
        key.keyID = m_prev_key_id;
        key.longPress = 0;
        key.status = eKEYPRESS;
        keypad_send_key(&key);
        
        return AK_TRUE;
    }
    else if( s_cur_mouse_state == 2 )
    {
        //key was released

        if( !m_sent_key_num && m_prev_key_id != kbNULL )
        {
            vtimer_stop( m_timer_id );
            
            key.keyID = m_prev_key_id;
            key.longPress = 0;
            key.status = eKEYUP;

            keypad_send_key( &key );
        }
        key.keyID = m_prev_key_id;
        key.longPress = 0;
        key.status = eKEYUP;
        keypad_send_key(&key);
        return AK_TRUE;
    }
    return AK_FALSE;

    /*
    /// Mouse pressed, but the previous mouse event has not been handled.
    if (MouseState == 0 && s_key_timer != ERROR_TIMER)
    {
        vtimer_stop(s_key_timer);
        s_key_timer = ERROR_TIMER;
        s_prev_key_id = kbNULL;
    }

    if (s_prev_key_id != kbNULL)
        return AK_FALSE;

    keypad.keyID = keypad_scan();

    if (keypad.keyID != kbNULL)
    {
        /// detect new key 
        s_prev_key_id = keypad.keyID;
        s_cur_key_delay = 0;
        s_sent_key_num = 0;
        s_key_timer = vtimer_start(KEYPAD_TIMER_DELAY, AK_FALSE, keypad_timer);
        keypad_disable_intr();        /// disable keypad interrupt 
        if (s_keypad_callback_func != AK_NULL && s_down_key_mode)
        {
            keypad.longPress = AK_FALSE;
            s_keypad_callback_func(&keypad);
            s_sent_key_num++;
        }
        return AK_TRUE;
    }

    return AK_FALSE;
    */
}

/**
 * @brief Keypad timer
 * Function keypad_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-21
 * @param T_TIMER timer_id: timer ID
 * @param T_U32 delay: timer delay
 * @return T_VOID
 * @retval
 */
static T_VOID keypad_timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD key;
    
    vtimer_stop( m_timer_id );
    
    key.keyID = m_prev_key_id;
    key.longPress = 1;
    key.status = eKEYPRESS;
    
    m_sent_key_num = 1;

    //call keypad callback function
    keypad_send_key( &key );
}


/**
 * @brief 
 * @author wanliming
 * @date 2006/12/30
 * @retval
 */
T_U16  GetW32KeyValue(T_U32  key )
{
     if(key< MAX_PHY_KEY_NUM)
     {
         return s_keypad_rect[key].key_id;
     }    
     else
     {
         return -1;
     }
}
/**
 * @brief 设置按键的工作模式，单按键工作模式或多按键工作模式
 * 单按键工作模式
 * @author lizhuobin
 * @date 2007-12-17
 * @param[in] press_mode single press or multiple press
 * @return T_BOOL
 * @retval AK_TRUE: set success
 * @retval AK_FALSE: set failure
 */
T_BOOL keypad_set_pressmode(T_eKEY_PRESSMODE press_mode)
{
    return AK_TRUE;
}


/**
 * @brief get keypad press mode
 * single press or multiple press, now multiple press just use in game
 * @author lizhuobin
 * @date 2007-12-17
 * @return T_eKEY_PRESSMODE single press or multiple press
 * @retval eSINGLE_PRESS singel press
 * @retval eMULTIPLE_PRESS multiple press
 */
T_eKEY_PRESSMODE keypad_get_pressmode(T_VOID)
{
    return eSINGLE_PRESS;
}
#endif
/* end of file */
