/**
 * @file hal_probe.c
 * @brief device probe framework
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author guoshaofeng 
 * @date 2010-12-07
 * @version 1.0
 * @ref
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_probe.h"

#define FM_MAX_SUPPORT      20  //max support fm module number
#define MAX_KEYPAD_SUPPORT  20  //max support keypad module number

typedef struct
{
    T_U32 DeviceID;
    T_TS_CAP_FUNCTION_HANDLER *handler;
}T_TS_CAP_INFO;


static T_LCD_INFO LCD_INFO_TABLE[LCD_MAX_SUPPORT] = {0};
static T_CAMERA_INFO CAMERA_INFO_TABLE[CAMERA_MAX_SUPPORT] = {0};
static T_FM_INFO FM_INFO_TABLE[FM_MAX_SUPPORT] = {0};
static T_TS_CAP_INFO TS_CAP_INFO_TABLE[TS_CAP_MAX_SUPPORT] = {0};
static T_KEYPAD_TYPE_INFO REG_TABLE[MAX_KEYPAD_SUPPORT] = {0};
static T_TS_INFO TS_INFO_TABLE[TS_MAX_SUPPORT] = {0};

/**
 * @BRIEF lcd probe pointer
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-24
 * @PARAM 
 * @RETURN T_LCD_FUNCTION_HANDLER: lcd device pointer
 * @RETVAL
 */
T_LCD_FUNCTION_HANDLER *lcd_probe(T_eLCD lcd)
{
    T_U32 i, id;
    
    for (i = 0; i < LCD_MAX_SUPPORT; i++)
    {
        if (LCD_INFO_TABLE[i].handler != AK_NULL)
        {
            id = LCD_INFO_TABLE[i].handler->lcd_read_id_func(lcd);
            akprintf(C2, M_DRVSYS, "lcd probe id = 0x%x, and device id = 0x%x\n", id, LCD_INFO_TABLE[i].DeviceID);
            if (id == LCD_INFO_TABLE[i].DeviceID)
            {
                akprintf(C2, M_DRVSYS, "match lcd, id = 0x%x\n", id);
                return LCD_INFO_TABLE[i].handler;
            }
        }
    }
    
    return AK_NULL;
}

T_BOOL lcd_reg_dev(T_U32 id, T_LCD_FUNCTION_HANDLER *handler, T_BOOL idx_sort_foward)
{
    T_S32 i;
    T_BOOL ret = AK_FALSE;
    if (idx_sort_foward )
    {
        for (i = 0; i < LCD_MAX_SUPPORT; i++)
        {
            // check device register or not 
            if (LCD_INFO_TABLE[i].DeviceID == id)
                break;
            // got an empty place for it 
            if (LCD_INFO_TABLE[i].DeviceID == 0 &&
                LCD_INFO_TABLE[i].handler == AK_NULL)
            {
                akprintf(C2, M_DRVSYS, "lcd register id = 0x%x, cnt = %d From the start\n", id, i);
                LCD_INFO_TABLE[i].DeviceID = id;
                LCD_INFO_TABLE[i].handler = handler;
                ret = AK_TRUE;
                break;            
            }
        }
    }
    else
    {
        for (i = LCD_MAX_SUPPORT - 1; i >= 0; i--)
        {
            // check device register or not 
            if (LCD_INFO_TABLE[i].DeviceID == id)
                break;
            // got an empty place for it 
            if (LCD_INFO_TABLE[i].DeviceID == 0 &&
                LCD_INFO_TABLE[i].handler == AK_NULL)
            {
                akprintf(C2, M_DRVSYS, "lcd register id = 0x%x, cnt = %d From the end \n", id, i);
                LCD_INFO_TABLE[i].DeviceID = id;
                LCD_INFO_TABLE[i].handler = handler;
                ret = AK_TRUE;
                break;            
            }
        }

    }

    return ret;
}

/**
 * @brief camera probe pointer
 * @author xia_wenting
 * @date 2010-12-07
 * @param
 * @return T_CAMERA_FUNCTION_HANDLER camera device pointer
 * @retval
 */
T_CAMERA_FUNCTION_HANDLER *cam_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        if (CAMERA_INFO_TABLE[i].handler != AK_NULL)
        {
            CAMERA_INFO_TABLE[i].handler->cam_open_func();
            id = CAMERA_INFO_TABLE[i].handler->cam_read_id_func();
            akprintf(C3, M_DRVSYS, "camera probe id = 0x%x, and device id = 0x%x\n", id, CAMERA_INFO_TABLE[i].DeviceID);
            if (id == CAMERA_INFO_TABLE[i].DeviceID)
            {
                akprintf(C3, M_DRVSYS, "match camera, id = 0x%x\n", id);
                return CAMERA_INFO_TABLE[i].handler;
            }
            else
            {
                CAMERA_INFO_TABLE[i].handler->cam_close_func();
            }
        }
    }
    
    return AK_NULL;
}
        
T_BOOL camera_reg_dev(T_U32 id, T_CAMERA_FUNCTION_HANDLER *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if (CAMERA_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it 
        if (CAMERA_INFO_TABLE[i].DeviceID == 0 &&
            CAMERA_INFO_TABLE[i].handler == AK_NULL)
        {
            akprintf(C3, M_DRVSYS, "camera register id = 0x%x, cnt = %d\n", id, i);
            CAMERA_INFO_TABLE[i].DeviceID = id;
            CAMERA_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}

/**
 * @brief probe fm
 * @author zhengwenbo
 * @date 2008-04-17
 * @param T_VOID
 * @return T_FM_FUNCTION_HANDLER: fm function handler
 */
T_FM_FUNCTION_HANDLER *fm_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i = 0; i < FM_MAX_SUPPORT; i++)
    {
        if (FM_INFO_TABLE[i].handler != AK_NULL)
        {
            id = FM_INFO_TABLE[i].handler->fm_read_id();
            akprintf(C3, M_DRVSYS, "fm probe id = 0x%x, and device id = 0x%x\n", id, FM_INFO_TABLE[i].DeviceID);
            if (id == FM_INFO_TABLE[i].DeviceID)
            {
                akprintf(C3, M_DRVSYS, "match fm, id = 0x%x\n", id);
                return FM_INFO_TABLE[i].handler;
            }
        }
    }
    
    return AK_NULL;
}

/**
 * @brief registe fm
 * @author zhengwenbo
 * @date 2008-04-17
 * @param[in] T_U32 id: device id
 * @param[in] T_FM_FUNCTION_HANDLER *handler: function handler
 * @return T_BOOL:  success or fail
 */
T_BOOL fm_reg_dev(T_U32 id, T_FM_FUNCTION_HANDLER *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < FM_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if ((FM_INFO_TABLE[i].DeviceID == id) && (AK_NULL != FM_INFO_TABLE[i].handler))
            break;
        // got an empty place for it 
        if (FM_INFO_TABLE[i].DeviceID == 0 &&
            FM_INFO_TABLE[i].handler == AK_NULL)
        {
            akprintf(C3, M_DRVSYS, "fm register id = 0x%x, cnt = %d\n", id, i);
            FM_INFO_TABLE[i].DeviceID = id;
            FM_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}

/**
 * @BRIEF keypad probe pointer
 * @AUTHOR dengjian
 * @DATE 2008-6-2
 * @PARAM 
 * @RETURN T_LCD_FUNCTION_HANDLER: lcd device pointer
 * @RETVAL
 */
T_KEYPAD_HANDLE *keypad_type_probe(T_U32 index)
{
    T_U32 i;
    
    for (i = 0; i < MAX_KEYPAD_SUPPORT; i++)
    {
        if (REG_TABLE[i].index == index)
        {
            return REG_TABLE[i].handler;
        }
    }
    
    return AK_NULL;
}

T_BOOL keypad_reg_scanmode(T_U32 index, T_KEYPAD_HANDLE *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < MAX_KEYPAD_SUPPORT; i++)
    {
        if ((REG_TABLE[i].index == 0) && (REG_TABLE[i].handler == AK_NULL))
        {
            REG_TABLE[i].index = index;
            REG_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}

/**
 * @BRIEF ts cap probe pointer
 * @AUTHOR guoshaofeng
 * @DATE 2008-04-29
 * @PARAM 
 * @RETURN T_TS_CAP_FUNCTION_HANDLER: ts cap device pointer
 * @RETVAL
 */
T_TS_CAP_FUNCTION_HANDLER *ts_cap_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i=0; i<TS_CAP_MAX_SUPPORT; i++)
    {
        if (TS_CAP_INFO_TABLE[i].handler != AK_NULL)
        {
            TS_CAP_INFO_TABLE[i].handler->ts_cap_open_func();
            id = TS_CAP_INFO_TABLE[i].handler->ts_cap_read_id_func();
            akprintf(C2, M_DRVSYS, "ts cap probe id = 0x%x, and device id = 0x%x\n", id, TS_CAP_INFO_TABLE[i].DeviceID);
            if (id == TS_CAP_INFO_TABLE[i].DeviceID)
            {
                akprintf(C3, M_DRVSYS, "match ts cap, id = 0x%x\n", id);
                return TS_CAP_INFO_TABLE[i].handler;
            }
            else
            {
                TS_CAP_INFO_TABLE[i].handler->ts_cap_close_func();
            }
        }
    }
    
    return AK_NULL;
}

T_BOOL ts_cap_reg_dev(T_U32 id, T_TS_CAP_FUNCTION_HANDLER *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < TS_CAP_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if (TS_CAP_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it 
        if (TS_CAP_INFO_TABLE[i].DeviceID == 0 &&
            TS_CAP_INFO_TABLE[i].handler == AK_NULL)
        {
            akprintf(C2, M_DRVSYS, "ts cap register id = 0x%x, cnt = %d\n", id, i);
            TS_CAP_INFO_TABLE[i].DeviceID = id;
            TS_CAP_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}

T_TS_PARAM *ts_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i = 0; i < TS_MAX_SUPPORT; i++)
    {
        if (TS_INFO_TABLE[i].handler != AK_NULL)
        {
            id = TS_INFO_TABLE[i].handler->TsTypeId;
            akprintf(C2, M_DRVSYS, "ts probe id = 0x%x\n",id);
            if (id == TS_INFO_TABLE[i].DeviceID)
            {
                akprintf(C2, M_DRVSYS, "match ts, id = 0x%x\n", id);
                return TS_INFO_TABLE[i].handler;
            }
        }
    }
    
    return AK_NULL;
}


T_BOOL ts_reg_dev(T_U32 id, T_TS_PARAM *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < TS_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if (TS_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it 
        if (TS_INFO_TABLE[i].DeviceID == 0 &&
            TS_INFO_TABLE[i].handler == AK_NULL)
        {
            akprintf(C2, M_DRVSYS, "TS register id = 0x%x\n", id);
            TS_INFO_TABLE[i].DeviceID = id;
            TS_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;            
        }
    }
    return ret;
}


