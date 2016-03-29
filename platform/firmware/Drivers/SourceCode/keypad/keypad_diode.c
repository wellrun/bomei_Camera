/**
 * @FILENAME: keypad_AI2L01.c
 * @BRIEF keypad CI2401 driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR huangxueying
 * @DATE 2007-04-18
 * @VERSION 1.0
 * @REF
 */
#include "akdefine.h"

#ifdef MOUNT_DIODE_SCAN_MODE 

#define KEYPAD_TIMER        uiTIMER1

//head define
#define DEFAULT_LONG_KEY_DELAY      1000
#define DEFAULT_INTERVAL_KEY_DELAY  200
#define SIMGAME_LONG_KEY_DELAY      200
#define SIMGAME_INTERVAL_KEY_DELAY  20





//typedef enum {
//    kbNULL = 0xFF,
//    kbUP = 0,       kbDOWN,         kbLEFT,         kbRIGHT,
//    kbVOICE_UP,     kbVOICE_DOWN,   kbMENU,         kbOK,
//    kbSWA,          kbSWB,          kbSW1,          kbSW2,
//    kbSW3,          kbSW4,          kbSW5,          kbCLEAR,
//    MAX_KEY_NUM
//} T_eKEY_ID;

static T_U32 kb_CLEAR_Val = 0;

//#define MAX_KEY_NUM 16


typedef enum {
    PRESS_SHORT = 0,
    PRESS_LONG,
    PRESS_UP,
    PRESS_TYPE_NUM
} T_PRESS_TYPE;

typedef struct {
    //T_eKEY_ID keyID;
    T_U32 keyID;
    T_PRESS_TYPE pressType;
} T_KEYPAD_DIODE;

typedef enum {
    NORMAL_MODE = 0,            // one long, another short, up
    WINDOWS_MODE,               // all short
    SINGLE_MODE,                // one short
    LONG_MODE,                  // all long
    NES_MODE,
    SNES_MODE,
    KEYPAD_MODE_NUM
} T_KEYPAD_DIODE_MODE;









typedef T_VOID (*T_fKEYPAD_CALLBACK_DIODE)(const T_KEYPAD_DIODE *keypad);

//#include "keypad.h"
//#include "vtimer.h"
//#include "gpio.h"
#define ALLOW_SEND_KEY_NUM          2

//#define KEYPAD_MODE 1


static T_fKEYPAD_CALLBACK_DIODE m_fKeypad_callback_func = AK_NULL;


static T_U32 m_keypad_shake_time = 20;

static T_U32 m_keypad_short_time = DEFAULT_INTERVAL_KEY_DELAY;
static T_U32 m_keypad_long_time_num = DEFAULT_LONG_KEY_DELAY/DEFAULT_INTERVAL_KEY_DELAY;

static T_TIMER m_keypad_power_shake_timer = ERROR_TIMER;
static T_TIMER m_keypad_power_timer = ERROR_TIMER;
static T_TIMER m_keypad_intr_shake_timer = ERROR_TIMER;
static T_TIMER m_keypad_intr_timer = ERROR_TIMER;

static T_KEYPAD_DIODE_MODE m_keypad_mode = NORMAL_MODE;

static T_U32 m_keypad_send_num[MAX_KEY_NUM] = {0};
static T_BOOL m_keypad_long_flag = AK_FALSE;



//static T_eKEY_ID m_keypad_prev[ALLOW_SEND_KEY_NUM];
static T_U32 m_keypad_prev[ALLOW_SEND_KEY_NUM];
static T_U32 m_keypad_prev_count = 0;

static T_VOID keypad_power_callback(T_U32 pin, T_U8 polarity);
static T_VOID keypad_power_shake_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_power_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_intr_callback(T_U32 pin, T_U8 polarity);
static T_VOID keypad_intr_shake_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_intr_timer(T_TIMER timer_id, T_U32 delay);
static T_BOOL keypad_intr_scan(T_BOOL isSHAKE);
//static T_VOID keypad_send(T_eKEY_ID keyID, T_BOOL isUP);
static T_VOID keypad_send(T_U32 keyID, T_BOOL isUP);



static T_U32 keypad_gpio_num = 0;
static T_U8 *m_keypad_gpio=AK_NULL;
static T_U8 gpio_power_key=0xff;
static T_U8 gpio_keypad_intr=0xff;
static T_U32 *m_keypad_matrix = AK_NULL;


//下面这4个宏要处理一下,这几个参数应该是由初始化函数传下来的
#define LEVEL_HIGH              1
#define LEVEL_LOW               0
#define GPIO_DIR_INPUT          0
#define GPIO_DIR_OUTPUT         1



//目前用这个回调
static T_f_H_KEYPAD_CALLBACK sys_keypad_callback = AK_NULL;

static T_VOID keypad_callback_changePara(const T_KEYPAD_DIODE *keypadDiode)
{
    T_KEYPAD keypad;
        
    keypad.keyID = keypadDiode->keyID; 

    if(PRESS_SHORT == keypadDiode->pressType)
    {
        if (m_keypad_long_flag)
        {
            keypad.longPress = AK_FALSE;         
            keypad.status = eKEYPRESS;        
            sys_keypad_callback(&keypad);    
        }
        else
        {
            keypad.longPress = AK_FALSE;         
            keypad.status = eKEYDOWN;
            sys_keypad_callback(&keypad);    
    
            keypad.longPress = AK_FALSE;         
            keypad.status = eKEYPRESS;        
            sys_keypad_callback(&keypad);    
    
            keypad.longPress = AK_FALSE;         
            keypad.status = eKEYUP;        
            sys_keypad_callback(&keypad);    
        }
    }
    else if(PRESS_UP == keypadDiode->pressType)
    {
//        if (AK_TRUE == m_keypad_long_flag)
//            keypad.longPress = AK_TRUE;         
//        else
            keypad.longPress = AK_FALSE;         
        keypad.status = eKEYUP;
        
        sys_keypad_callback(&keypad);
    }
    else if(PRESS_LONG == keypadDiode->pressType)
    {
        keypad.longPress = AK_FALSE;         
        keypad.status = eKEYDOWN;
        sys_keypad_callback(&keypad);  
        
        keypad.longPress = AK_TRUE;         
        keypad.status = eKEYPRESS;
        sys_keypad_callback(&keypad);  
    }    
    else
    {
        return;
    }    
}


//T_VOID keypad_init(T_fKEYPAD_CALLBACK_DIODE callback_func)
static T_VOID keypad_init_diode(T_f_H_KEYPAD_CALLBACK callback_func,const T_PLATFORM_KEYPAD_PARM *keypad_parm)
{
    T_U32 i;
    
    akprintf(C3, M_DRVSYS, "keypad_init_diode\n");
    
    keypad_gpio_num = keypad_parm->row_qty;
    m_keypad_gpio = keypad_parm->RowGpio;
    gpio_power_key = keypad_parm->switch_key_id;
    gpio_keypad_intr = (keypad_parm->ColumnGpio)[0];
    m_keypad_matrix = keypad_parm->keypad_matrix;
    kb_CLEAR_Val = keypad_parm->switch_key_value;
    
    
    for (i=0; i<keypad_gpio_num; i++)
    {
        gpio_set_pin_dir(m_keypad_gpio[i], GPIO_DIR_OUTPUT);
        gpio_set_pull_up_r(m_keypad_gpio[i], AK_FALSE);
        gpio_set_pin_level(m_keypad_gpio[i], LEVEL_HIGH);
    }


    m_keypad_power_shake_timer = ERROR_TIMER;
    m_keypad_power_timer = ERROR_TIMER;
    m_keypad_intr_shake_timer = ERROR_TIMER;
    m_keypad_intr_timer = ERROR_TIMER;

    m_keypad_mode = NORMAL_MODE;

    for (i=0; i<MAX_KEY_NUM; i++)
    {
        m_keypad_send_num[i] = 0;
    }
    m_keypad_long_flag = AK_FALSE;

    for (i=0; i<ALLOW_SEND_KEY_NUM; i++)
    {
        m_keypad_prev[i] = kbUnKnown;
    }
    m_keypad_prev_count = 0;
        
    sys_keypad_callback = (T_f_H_KEYPAD_CALLBACK )callback_func;
    m_fKeypad_callback_func = keypad_callback_changePara;
    
    //现在由键盘模块设置自己的方向
    gpio_set_pin_dir(gpio_power_key,0);   
    gpio_set_pin_dir(gpio_keypad_intr,0);   
    //关闭上拉电阻
    gpio_set_pull_up_r(gpio_power_key, AK_FALSE);
    gpio_set_pull_up_r(gpio_keypad_intr, AK_FALSE);
        
    gpio_register_int_callback(gpio_power_key, LEVEL_LOW, GPIO_INTERRUPT_DISABLE, keypad_power_callback);   
    gpio_register_int_callback(gpio_keypad_intr, LEVEL_HIGH, GPIO_INTERRUPT_DISABLE, keypad_intr_callback);   
}

static T_VOID keypad_enable_intr_diode(T_VOID)
{
    gpio_int_control(gpio_power_key, AK_TRUE);
    gpio_int_control(gpio_keypad_intr, AK_TRUE);
}

static T_VOID keypad_disable_intr_diode(T_VOID)
{
    gpio_int_control(gpio_power_key, AK_FALSE);
    gpio_int_control(gpio_keypad_intr, AK_FALSE);
}


////驱动库的唤醒gpio是统一设置的。这个函数可以不要了。
//T_VOID keypad_enable_wakeup(T_VOID)
//{
//#if (KEYPAD_MODE == 1)
//    gpio_set_pin_dir(gpio_keypad_intr, GPIO_DIR_INPUT);
//    gpio_wgpio_mask(gpio_keypad_intr, AK_TRUE);
//    gpio_wgpio_pol(gpio_keypad_intr, LEVEL_HIGH);
//    
//    gpio_set_pin_dir(gpio_power_key, GPIO_DIR_INPUT);
//    gpio_wgpio_mask(gpio_power_key, AK_TRUE);
//    gpio_wgpio_pol(gpio_power_key, LEVEL_LOW);
//#elif (KEYPAD_MODE == 2)
//    T_U32 i;
//
//    for (i=0; i<KEYPAD_MAX_COLUMN; i++)    //set row gpio output high level
//    {
//        gpio_set_pin_dir(m_ucColumnGpio[i], GPIO_DIR_OUTPUT);
//        gpio_set_pin_level(m_ucColumnGpio[i], LEVEL_HIGH);
//    }
//
//    for (i=0; i<KEYPAD_MAX_ROW; i++) //set column gpio as input
//    {
//        gpio_set_pin_dir(m_ucRowGpio[i], GPIO_DIR_INPUT);
//        gpio_wgpio_mask(m_ucRowGpio[i], AK_TRUE);
//        gpio_wgpio_pol(m_ucRowGpio[i], LEVEL_HIGH);
//    }
//#endif
//}

//T_VOID keypad_disable_wakeup(T_VOID)
//{
//#if (KEYPAD_MODE == 1)
//    gpio_wgpio_mask(gpio_keypad_intr, AK_FALSE);
//    gpio_wgpio_mask(gpio_power_key, AK_FALSE);
//#elif (KEYPAD_MODE == 2)
//    T_U32 i;
//
//    for (i=0; i<KEYPAD_MAX_ROW; i++) //set column gpio as input
//    {
//        gpio_wgpio_mask(m_ucRowGpio[i], AK_FALSE);
//    }
//#endif
//}


static T_VOID keypad_power_callback(T_U32 pin, T_U8 polarity)
{
     akprintf(C3, M_DRVSYS, "keypad_power_callback in diode keypad,pin=%d,polarity=%d\n",pin,polarity);    
    gpio_int_control(gpio_power_key, AK_FALSE);

    if (polarity == LEVEL_LOW)
    {
        if (m_keypad_power_shake_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_power_shake_timer);
            m_keypad_power_shake_timer = ERROR_TIMER;
        }

        m_keypad_power_shake_timer = timer_start(KEYPAD_TIMER,m_keypad_shake_time, AK_FALSE, keypad_power_shake_timer);
        if (m_keypad_power_shake_timer == ERROR_TIMER)
        {
            akprintf(C3, M_DRVSYS, "keypad_power_callback: keypad power shake not enough timer!\n");
        }

        gpio_set_int_p(gpio_power_key, LEVEL_HIGH);
    }
    else
    {
        if (m_keypad_power_shake_timer != ERROR_TIMER)
        {
            akprintf(C3, M_DRVSYS, "keypad_power_callback: keypad power shake!\n");
            timer_stop(m_keypad_power_shake_timer);
            m_keypad_power_shake_timer = ERROR_TIMER;
        }

        if (m_keypad_power_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_power_timer);
            m_keypad_power_timer = ERROR_TIMER;

            keypad_send(kb_CLEAR_Val, AK_TRUE);
        }

        gpio_set_int_p(gpio_power_key, LEVEL_LOW);
    }

    m_keypad_send_num[kb_CLEAR_Val] = 0;
    m_keypad_long_flag = AK_FALSE;
    gpio_int_control(gpio_power_key, AK_TRUE);
}

static T_VOID keypad_power_shake_timer(T_TIMER timer_id, T_U32 delay)
{
    if (m_keypad_power_shake_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_power_shake_timer);
        m_keypad_power_shake_timer = ERROR_TIMER;
    }

    if (gpio_get_pin_level(gpio_power_key) == LEVEL_LOW)
    {
        if (m_keypad_power_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_power_timer);
            m_keypad_power_timer = ERROR_TIMER;
        }

        m_keypad_power_timer = timer_start(KEYPAD_TIMER,DEFAULT_INTERVAL_KEY_DELAY, AK_TRUE, keypad_power_timer);
        if (m_keypad_power_timer == ERROR_TIMER)
        {
            akprintf(C3, M_DRVSYS, "keypad_power_shake_timer: keypad power not enough timer!\n");
        }
    }
}

static T_VOID keypad_power_timer(T_TIMER timer_id, T_U32 delay)
{
    if (gpio_get_pin_level(gpio_power_key) == LEVEL_LOW)
    {
        keypad_send(kb_CLEAR_Val, AK_FALSE);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "keypad_power_timer: keypad power timer error!\n");
        if (m_keypad_power_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_power_timer);
            m_keypad_power_timer = ERROR_TIMER;
        }
    }
}

static T_VOID keypad_intr_callback(T_U32 pin, T_U8 polarity)
{
    T_U32 i;
    
    //akprintf(C3, M_DRVSYS, "keypad_intr_callback in diode keypad\n");    
    gpio_int_control(gpio_keypad_intr, AK_FALSE);

    if (polarity == LEVEL_HIGH)
    {
        if (m_keypad_intr_shake_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_intr_shake_timer);
            m_keypad_intr_shake_timer = ERROR_TIMER;
        }

        m_keypad_intr_shake_timer = timer_start(KEYPAD_TIMER,m_keypad_shake_time, AK_FALSE, keypad_intr_shake_timer);
        if (m_keypad_intr_shake_timer == ERROR_TIMER)
        {
            akprintf(C3, M_DRVSYS, "keypad_intr_callback: keypad intr shake not enough timer!\n");
        }

        for (i=0; i<ALLOW_SEND_KEY_NUM; i++)
        {
            m_keypad_prev[i] = kbUnKnown;
        }
        m_keypad_prev_count = 0;

        gpio_set_int_p(gpio_keypad_intr, LEVEL_LOW);
    }
    else
    {
        if (m_keypad_intr_shake_timer != ERROR_TIMER)
        {
            //akprintf(C3, M_DRVSYS, "keypad_intr_callback: keypad intr shake!\n");
            timer_stop(m_keypad_intr_shake_timer);
            m_keypad_intr_shake_timer = ERROR_TIMER;
        }

        if (m_keypad_intr_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_intr_timer);
            m_keypad_intr_timer = ERROR_TIMER;

            for (i=0; i<ALLOW_SEND_KEY_NUM; i++)
            {
                if (m_keypad_prev[i] != kbUnKnown)
                {
                    keypad_send(m_keypad_prev[i], AK_TRUE);
                }
                else
                {
                    //akprintf(C3, M_DRVSYS, "keypad_intr_callback: keypad prev error!\n");
                }
            }
        }

        for (i=0; i<ALLOW_SEND_KEY_NUM; i++)
        {
            m_keypad_prev[i] = kbUnKnown;
        }
        m_keypad_prev_count = 0;

        gpio_set_int_p(gpio_keypad_intr, LEVEL_HIGH);
    }

    for (i=0; i<MAX_KEY_NUM; i++)
    {
        if (i != kb_CLEAR_Val)
        {
            m_keypad_send_num[i] = 0;
        }
    }
    m_keypad_long_flag = AK_FALSE;
    gpio_int_control(gpio_keypad_intr, AK_TRUE);
}

static T_VOID keypad_intr_shake_timer(T_TIMER timer_id, T_U32 delay)
{
     if (m_keypad_intr_shake_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_intr_shake_timer);
        m_keypad_intr_shake_timer = ERROR_TIMER;
    }

    if (gpio_get_pin_level(gpio_keypad_intr) == LEVEL_HIGH)
    {
        if (m_keypad_intr_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_intr_timer);
            m_keypad_intr_timer = ERROR_TIMER;
        }

        if (keypad_intr_scan(AK_TRUE) == AK_TRUE)
        {
            m_keypad_intr_timer = timer_start(KEYPAD_TIMER,m_keypad_short_time, AK_TRUE, keypad_intr_timer);
            if (m_keypad_intr_timer == ERROR_TIMER)
            {
                akprintf(C3, M_DRVSYS, "keypad_intr_shake_timer: keypad intr not enough timer!\n");
            }
        }
    }
}

static T_VOID keypad_intr_timer(T_TIMER timer_id, T_U32 delay)
{
    if (gpio_get_pin_level(gpio_keypad_intr) == LEVEL_HIGH)
    {
        keypad_intr_scan(AK_FALSE);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "keypad_intr_timer: keypad intr timer error!\n");
        if (m_keypad_power_timer != ERROR_TIMER)
        {
            timer_stop(m_keypad_power_timer);
            m_keypad_power_timer = ERROR_TIMER;
        }
    }
}





static T_S32 keypad_scan_diode(T_VOID)
{
    T_U32 i, j, k;

    for (i=0; i<keypad_gpio_num; i++)
    {
        gpio_set_pin_dir(m_keypad_gpio[i], GPIO_DIR_OUTPUT);
        gpio_set_pull_up_r(m_keypad_gpio[i], AK_FALSE);
        gpio_set_pin_level(m_keypad_gpio[i], LEVEL_HIGH);

        for (j=0; j<keypad_gpio_num; j++)
        {
            if (j != i)
            {
                gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_INPUT);
                gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
            }
        }

        for (j=0; j<10000; j++);

        for (k=0; k<keypad_gpio_num; k++)
        {
            if (k == i)
            {
                continue;
            }

            if (gpio_get_pin_level(m_keypad_gpio[k]) == LEVEL_HIGH)
            {
                for (j=0; j<keypad_gpio_num; j++)
                {
                    gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_OUTPUT);
                    gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
                    gpio_set_pin_level(m_keypad_gpio[j], LEVEL_HIGH);
                }
                
                if(k > i)                
                    return (m_keypad_matrix[i*(keypad_gpio_num-1) + k - 1]);
                else
                    return (m_keypad_matrix[i*(keypad_gpio_num-1) + k ]);     
            }
        }
    }

    for (j=0; j<keypad_gpio_num; j++)
    {
        gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_OUTPUT);
        gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
        gpio_set_pin_level(m_keypad_gpio[j], LEVEL_HIGH);
    }

    return kbUnKnown;
}






static T_BOOL keypad_intr_scan(T_BOOL isSHAKE)
{
    T_U32 i, j, k, savecount;
    T_U32 keyid, savekeyid[ALLOW_SEND_KEY_NUM+1];

    savecount = 0;
    for(i=0; i<ALLOW_SEND_KEY_NUM+1; i++)
    {
        savekeyid[i] = kbUnKnown;
    }

    for (i=0; i<keypad_gpio_num; i++)
    {
        gpio_set_pin_dir(m_keypad_gpio[i], GPIO_DIR_OUTPUT);
        gpio_set_pull_up_r(m_keypad_gpio[i], AK_FALSE);
        gpio_set_pin_level(m_keypad_gpio[i], LEVEL_HIGH);

        for (j=0; j<keypad_gpio_num; j++)
        {
            if (j != i)
            {
                gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_INPUT);
                gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
            }
        }

        for (j=0; j<100; j++);

        for (k=0; k<keypad_gpio_num; k++)
        {
            if (k == i)
            {
                continue;
            }

            if (gpio_get_pin_level(m_keypad_gpio[k]) == LEVEL_HIGH)
            {
                //keyid = m_keypad_matrix[i][(k+1)%keypad_gpio_num];
                //keyid = m_keypad_matrix[i*keypad_gpio_num + ((k+1)%keypad_gpio_num)];
                if(k > i)                
                    keyid = m_keypad_matrix[i*(keypad_gpio_num-1) + k - 1];
                else
                    keyid = m_keypad_matrix[i*(keypad_gpio_num-1) + k ];     
                
                if (((m_keypad_mode == NES_MODE) || (m_keypad_mode == SNES_MODE)) && (isSHAKE == AK_FALSE))
                {
                    for (j=0; j<ALLOW_SEND_KEY_NUM; j++)
                    {
                        if ((m_keypad_prev[j] != kbUnKnown) && (m_keypad_prev[j] != keyid))
                        {
                            keypad_send(m_keypad_prev[j], AK_TRUE);
                        }
                    }
                }

                savekeyid[savecount++] = keyid;
                if (savecount > ALLOW_SEND_KEY_NUM)
                {
                    akprintf(C3, M_DRVSYS, "keypad_intr_scan: scan >%d key pressed!\n", ALLOW_SEND_KEY_NUM);

                    for (j=0; j<keypad_gpio_num; j++)
                    {
                        gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_OUTPUT);
                        gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
                        gpio_set_pin_level(m_keypad_gpio[j], LEVEL_HIGH);
                    }

                    return AK_FALSE;
                }
            }
        }
    }
    
    if ((savecount == 0) && (gpio_get_pin_level(gpio_keypad_intr) == LEVEL_HIGH))
    {
        savekeyid[savecount++] = kb_CLEAR_Val;
    }
    
    if ((isSHAKE == AK_FALSE) && (savecount > 0))
    {
        for (i=0; i<savecount; i++)
        {
            keypad_send(savekeyid[i], AK_FALSE);
        }
    }

    if ((savecount < m_keypad_prev_count) && (m_keypad_prev_count > 1))
    {
        for (j=0; j<m_keypad_prev_count; j++)
        {
            for (i=0; i<savecount; i++)
            {
                if (m_keypad_prev[j] == savekeyid[i])
                {
                    break;
                }
            }

            if (i >= savecount)
            {
                keypad_send(m_keypad_prev[j], AK_TRUE);
                m_keypad_send_num[m_keypad_prev[j]] = 0;
                m_keypad_prev[j] = kbUnKnown;
            }
        }
    }

    for (i=0; i<savecount; i++)
    {
        m_keypad_prev[i] = savekeyid[i];
    }
    m_keypad_prev_count = savecount;

    for (j=0; j<keypad_gpio_num; j++)
    {
        gpio_set_pin_dir(m_keypad_gpio[j], GPIO_DIR_OUTPUT);
        gpio_set_pull_up_r(m_keypad_gpio[j], AK_FALSE);
        gpio_set_pin_level(m_keypad_gpio[j], LEVEL_HIGH);
    }

    return AK_TRUE;
}

static T_VOID keypad_send(T_U32 keyID, T_BOOL isUP)
{
    T_KEYPAD_DIODE keypad;

    keypad.keyID = keyID;
    keypad.pressType = PRESS_SHORT;
    m_keypad_send_num[keypad.keyID]++;

    switch (m_keypad_mode)
    {
        case NORMAL_MODE:
            if (isUP == AK_TRUE)
            {
                if (m_keypad_long_flag == AK_FALSE)
                {
                    m_fKeypad_callback_func(&keypad);
                }
                else
                {
                    keypad.pressType = PRESS_UP;
                    m_fKeypad_callback_func(&keypad);
                }
            }
            else
            {
                if (m_keypad_send_num[keypad.keyID] == m_keypad_long_time_num)
                {
                    m_keypad_long_flag = AK_TRUE;
                    keypad.pressType = PRESS_LONG;
                    m_fKeypad_callback_func(&keypad);
                }
                else if (m_keypad_send_num[keypad.keyID] > m_keypad_long_time_num)
                {
                    m_fKeypad_callback_func(&keypad);
                }
            }
            break;
        case WINDOWS_MODE:
            m_fKeypad_callback_func(&keypad);
            break;
        case SINGLE_MODE:
            if ((isUP == AK_TRUE) 
                && (m_keypad_send_num[keypad.keyID] < m_keypad_long_time_num))
            {
                m_fKeypad_callback_func(&keypad);
            }
            else if (m_keypad_send_num[keypad.keyID] == m_keypad_long_time_num)
            {
                m_keypad_long_flag = AK_TRUE;
                keypad.pressType = PRESS_LONG;
                m_fKeypad_callback_func(&keypad);
            }
            break;
        case LONG_MODE:
            if (isUP == AK_TRUE)
            {
                if (m_keypad_send_num[keypad.keyID] > m_keypad_long_time_num)
                {
                    keypad.pressType = PRESS_UP;
                    m_fKeypad_callback_func(&keypad);
                }
            }
            else
            {
                if ((m_keypad_send_num[keypad.keyID] % m_keypad_long_time_num) == 0)
                {
                    m_keypad_long_flag = AK_TRUE;
                    keypad.pressType = PRESS_LONG;
                    m_fKeypad_callback_func(&keypad);
                }
            }
            break;
        case NES_MODE:
        case SNES_MODE:
            if (keyID == kb_CLEAR_Val)
            {
                if (isUP == AK_TRUE)
                {
                    if (m_keypad_long_flag == AK_FALSE)
                    {
                        m_fKeypad_callback_func(&keypad);
                    }
                    else
                    {
                        keypad.pressType = PRESS_UP;
                        m_fKeypad_callback_func(&keypad);
                    }
                }
                else
                {
                    if (m_keypad_send_num[keypad.keyID] == m_keypad_long_time_num*10)
                    {
                        m_keypad_long_flag = AK_TRUE;
                        keypad.pressType = PRESS_LONG;
                        m_fKeypad_callback_func(&keypad);
                    }
                    else if (m_keypad_send_num[keypad.keyID] > m_keypad_long_time_num*10)
                    {
                        m_fKeypad_callback_func(&keypad);
                    }
                }
            }
            else
            {
                if (isUP == AK_TRUE)
                {
                    keypad.pressType = PRESS_UP;
                }

                if (m_keypad_mode == NES_MODE)
                {
                    //fc_emu_key_callback(keypad);
                }
                else if (m_keypad_mode == SNES_MODE)
                {
                    //snes_emu_key_callback(keypad);
                }
            }
            break;
        default:
            akprintf(C3, M_DRVSYS, "keypad_send: keypad mode error!\n");
            break;
    }
}


/*游戏过程中当闹钟来时,如果仍有键被按下未松开,那么进入闹钟keypad_mode
    被切换成Normal_Mode时没有给游戏发送press_up类型的按键值,则游戏在闹钟
    消失后有异常。如游戏中长按kbRight,闹钟响起后松开键,则闹钟消失后,
    游戏继续,虽然没有按键,但游戏却仍然为按kbRight键的响应方式,直到有重新按下该键。

    此函数功能和物理上松开按键执行的操作一样。在响应闹钟时改变按键模式前调用,
    则能避免上边的问题。

static T_VOID Keypad_Set_GameKey_Up(T_VOID)
{
    T_U32 i;
        
    if (m_keypad_intr_shake_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_intr_shake_timer);
        m_keypad_intr_shake_timer = ERROR_TIMER;
    }
    
    if (m_keypad_intr_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_intr_timer);
        m_keypad_intr_timer = ERROR_TIMER;

        for (i=0; i < ALLOW_SEND_KEY_NUM; i++)
        {
            if (m_keypad_prev[i] != kbUnKnown)
            {
                //AK_DEBUG_OUTPUT("$$debug token: Keypad_Nes_Set_KeyUp()\n"); 
                akprintf(C3, M_DRVSYS, "$$debug token: Keypad_Nes_Set_KeyUp()\n");
                keypad_send(m_keypad_prev[i], AK_TRUE);
            }
        }
    }
}

static T_U16 Keypad_GetKeyState(T_VOID)
{
    return 0;
}

static T_VOID keypad_stop_intervaltimer(T_VOID)
{
    if (m_keypad_power_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_power_timer);
        m_keypad_power_timer = ERROR_TIMER;
    }

    if (m_keypad_intr_timer != ERROR_TIMER)
    {
        timer_stop(m_keypad_intr_timer);
        m_keypad_intr_timer = ERROR_TIMER;
    }
}

static T_VOID keypad_set_mode(T_KEYPAD_DIODE_MODE keypad_mode)
{
    m_keypad_mode = keypad_mode;
}

//T_VOID keypad_set_delay(T_U32 long_delay, T_U32 short_delay)
static T_VOID keypad_set_delay_diode(T_U32 long_delay, T_U32 short_delay)
{
    if (short_delay == 0)
        short_delay = DEFAULT_INTERVAL_KEY_DELAY;
    if (long_delay == 0)
        long_delay = DEFAULT_LONG_KEY_DELAY;

    m_keypad_short_time = short_delay;
    if ((long_delay%short_delay) == 0)
    {
        m_keypad_long_time_num = long_delay/short_delay;
    }
    else
    {
        m_keypad_long_time_num = long_delay/short_delay + 1;
    }
}

*/




//******************************reg my handle info***************************//
static T_KEYPAD_HANDLE diode_handler = 
{
	keypad_init_diode,
	keypad_scan_diode,
	keypad_enable_intr_diode, 
	keypad_disable_intr_diode, 
	AK_NULL,
	AK_NULL,
	AK_NULL
};

static int diode_reg(void)
{
	keypad_reg_scanmode(KEYPAD_MATRIX_DIODE,&diode_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(diode_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif
