
#ifndef __KEYHANDLER_H__
#define __KEYHANDLER_H__

#include "anyka_types.h"

typedef enum {
    PRESS_SHORT = 0,
    PRESS_LONG,
    PRESS_UP,
    PRESS_TYPE_NUM
} T_PRESS_TYPE;


typedef struct {
    T_U32 keyID;
    T_PRESS_TYPE pressType;
} T_MMI_KEYPAD;

/**
 * @brief stop key, stop triggering short key event when long press key 
 * @author zhengwenbo
 * @date 2008-4-9
 * @return T_BOOL whether success
 * @retval AK_TRUE: success  AK_FALSE:fail
 *
*/
T_BOOL keypad_keystop(T_VOID);

T_VOID Set_key_valid(T_BOOL flag);

T_BOOL Is_key_valid(T_VOID);

void keypad_open(void);

T_VOID Fwl_keypad_change_to_num2x3(T_VOID);

T_VOID Fwl_keypad_change_to_num3x4(T_VOID);


#endif
