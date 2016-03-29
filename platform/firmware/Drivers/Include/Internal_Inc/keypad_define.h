#ifndef __KEYPAD_DEFINE_H__
#define __KEYPAD_DEFINE_H__

#include "anyka_types.h"
#include "platform_hd_config.h"

//GPIO KETPAD PARM
typedef struct{
    T_U32    row_qty;                ///< row gpio 数量 
    T_U32    column_qty;             ///< column gpio 数量 
    T_U8     *RowGpio;               ///< 指向row gpio 数组的指针 
    T_U8     *ColumnGpio;            ///< 指向Column gpio 数组的指针 
    T_U32    *keypad_matrix;         ///< 指向键盘阵列逻辑数组的指针 
    T_S8     *updown_matrix;         ///< 指向updown逻辑数组的指针 
    T_U32    active_level;           ///< 键盘有效电平值, 1或0 
    T_U32    switch_key_id;          ///< 电源键的gpio id值 
    T_U32    switch_key_value;        ///< 电源键的键值 
    T_U32    switch_key_active_level; ///< 电源键的有效电平值，1或0 
} T_PLATFORM_KEYPAD_GPIO;

typedef  T_U16 T_AD_VAL;
typedef  T_U16 T_ID_VAL;

typedef struct 
{
    T_U16  Min;    //按键变化的最小值
    T_U16  Max;    //按键变化的最大值
    T_U16  KeyID;  //按键ID
}KEY_DETECT_STR;

//ANALOG KETPAD PARM
typedef struct{
    KEY_DETECT_STR *key_avl_array;   //指向AD按键的映身表。
    T_U32       key_max_num;      //按键的数量
    T_U32       ad_avl_offset;    //一个按键的上下偏移量
    T_U32       AdValMin;         //有效按键的最小值
    T_U32       AdValMax;         //有效按键的最大值
} T_PLATFORM_KEYPAD_ANALOG;

typedef union{
    T_PLATFORM_KEYPAD_GPIO      PARM_GPIO;
    T_PLATFORM_KEYPAD_ANALOG    PARM_ANALOG;
}T_PLATFORM_KEYPAD_PARM;

#endif //end of __KEYPAD_DEFINE_H__
