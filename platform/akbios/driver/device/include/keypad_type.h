
/**@file fm_probe.h
 * @brief fm moudle, fm probe
 *
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2008.4.17
 * @version 1.0
 */

#ifndef __KEYPAD_TYPE_H__
#define __KEYPAD_TYPE_H__


typedef enum {
    KEYPAD_MATRIX_NORMAL = 0,   // normal matrix keypad
    KEYPAD_MATRIX_DIODE,        // diode matrix keypad
    KEYPAD_MATRIX_MIXED,        // mixed matrix keypad
    KEYPAD_KEY_PER_GPIO,         //one gpio = one key keypad
    KEYPAD_TYPE_NUM
} T_KEYPAD_TYPE;



/* 此结构体不允许修改, 此结构体内容需由客户实现，并通过接口keypad_get_platform_parm返回给我们 */
/* 我们在调用keypad_init时会调用keypad_get_platform_parm接口，得到用户平台的键盘信息 */
typedef struct _PLATFORM_KEYPAD_PARM_{
    T_U32		row_qty;        			///< row gpio 数量 
    T_U32		column_qty;     			///< column gpio 数量 
    T_U8 		*RowGpio;       			///< 指向row gpio 数组的指针 
    T_U8 		*ColumnGpio;    			///< 指向Column gpio 数组的指针 
    T_U32		*keypad_matrix; 			///< 指向键盘阵列逻辑数组的指针 
    T_S8 		*updown_matrix; 			///< 指向updown逻辑数组的指针 
    T_U32		active_level;   			///< 键盘有效电平值, 1或0 
    T_U32		switch_key_id;  			///< 电源键的gpio id值 
    T_U32       switch_key_value;			///< 电源键的键值 
    T_U32		switch_key_active_level; 	///< 电源键的有效电平值，1或0 
} T_PLATFORM_KEYPAD_PARM;



/**
 * @brief 用户实现的接口，取平台的键盘的设置信息，此接口会被keypad_init调用
 *
 * @author Miaobaoli
 * @date 2004-09-21
 * @return T_PLATFORM_KEYPAD_PARM *：平台的键盘的设置信息的结构体指针
 * @retval the pointer of platform keypad information 
 */
const T_PLATFORM_KEYPAD_PARM *keypad_get_platform_parm(T_VOID);

/**
 * @取平台键盘扫描类型
 * @author Dengjian
 * @date 2008-05-21
 * @param[in] T_VOID
 * @return T_KEYPAD_TYPE
 */
const T_KEYPAD_TYPE keypad_get_platform_type(T_VOID);
//**********************************************************************************************




#endif

