/**
 * @file hal_ts.c
 * @brief touch screen driver source file
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2010-08-31
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_ts_cap.h"
#include "ts_adc.h"
#include "ts_cap.h"
#include "drv_module.h"

//buffer size
#define TS_VALUE_BUF_SIZE       (64)

//ts buffer control struct
typedef struct
{
    T_U32       ts_head;
    T_U32       ts_tail;
    T_TSPOINT   ts_buf[TS_VALUE_BUF_SIZE];
} T_TS_BUF_CTRL;

//ts struct
typedef struct
{
    T_TS_BUF_CTRL       buf_ctrl;
    T_fTS_ADC_CALLBACK  callback_func;
    E_TS_TYPE           ts_type;
} T_HAL_TS;

static T_VOID       ts_callback(T_U32 *param, T_U32 len);
static T_VOID       ts_send_value(const T_TSPOINT *pt);
static T_HAL_TS     m_hal_ts;

/**
 * @brief Initialize touch panel
 *
 * this function only need to be called one time when the system powers on !
 * But it must be called after gpio and timer init
 * @author LuoXiaoQing
 * @date 2006-04-26
 * @param[in] callback touchscreen callback function pointor
 * @param[in] gpio  gpio used to detect ts's interrupt
 * @param[in] active_level gpio level that will activate ts
 * @return T_VOID
 * @remark This callback function is very important in this driver! It will be called when the driver
 * get a sample point. These are two special point  (0x7fff,0x00) means pen up (0x7fff, 0x7fff) means fond end.
 */
T_VOID ts_init(E_TS_TYPE type, T_fTS_ADC_CALLBACK callback, T_U32 gpio, T_U8 active_level)
{
    //init global variables
    memset(&m_hal_ts, 0, sizeof(m_hal_ts));
    m_hal_ts.callback_func = callback;

    //get ts type and do hardware init
    if (E_TS_TYPE_RES == type)
    {
        //ts_adc_init(ts_send_value, gpio, active_level);
    }
    else if (E_TS_TYPE_CAP == type)
    {
        ts_cap_init(ts_send_value, gpio, active_level);
    }
    else
    {
        akprintf(C2, M_DRVSYS, "select touch screen type error %d\r\n", type);
        return;
    }
    m_hal_ts.ts_type = type;
}

/**
 * @brief Get the point from ts inner buffer
 *
 * @author LuoXiaoQing
 * @date 2006-04-26
 * @param pt [out]: pointer to T_TSPOINT struct, (x,y) value will be saved here
 * @return T_BOOL
 * @retval AK_TRUE means get point successfully
 * @retval AK_FALSE means inner point buffer empty
 */
T_BOOL ts_get_value(T_TSPOINT *pt)
{
    T_BOOL ret = AK_FALSE;
    T_TS_BUF_CTRL *buf_ctrl = &m_hal_ts.buf_ctrl;

    //check param and check if ts buffer is empty or not
    if (pt == AK_NULL || (buf_ctrl->ts_head == buf_ctrl->ts_tail))
    {
        if (AK_NULL != pt)
        {
            pt->x = 0;
            pt->y = 0;
            pt->x2 = 0;
            pt->y2 = 0;
            pt->finger_num = 0;
        }

        return ret;
    }

    //get value from buffer
    *pt = buf_ctrl->ts_buf[buf_ctrl->ts_head];
    buf_ctrl->ts_head = (buf_ctrl->ts_head + 1) % TS_VALUE_BUF_SIZE;

    return AK_TRUE;
}

/**
 * @brief Get the current point's coordinate data
 *
 * @author LuoXiaoQing
 * @date 2006-04-26
 * @param[out] pt pointer of the current point's coordinate data
 * @return T_BOOL
 * @retval AK_FALSE means failed
 * @retval AK_TRUE means successful
 * @remark This function is use for calibrating touch screen.
 */
T_BOOL ts_get_cur_point(T_pTSPOINT pt)
{
    T_BOOL ret;
    U_TS_CAP_PARAM param;

    if (E_TS_TYPE_RES == m_hal_ts.ts_type)
    {
        ret = AK_FALSE;//ts_adc_get_cur_point(pt);
    }
    else if (E_TS_TYPE_CAP == m_hal_ts.ts_type)
    {
        ret = ts_cap_get_info(TS_CAP_GET_CUR_POINT, &param);

        pt->x = param.point.x;
        pt->y = param.point.y;
        pt->x2 = param.point.x2;
        pt->y2 = param.point.y2;
        pt->finger_num = param.point.finger_num;
    }
    else
    {
        ret = AK_FALSE;
        akprintf(C2, M_DRVSYS, "select touch screen type error\r\n");
    }

    return ret;
}


/**
 * @brief save point value to ts buffer, and notify outside observer
 *
 * @author LuoXiaoQing
 * @date 2006-04-26
 * @param[out] pt pointer of the current point's coordinate data
 * @return T_BOOL
 * @retval AK_FALSE means failed
 * @retval AK_TRUE means successful
 * @remark This function is use for calibrating touch screen.
 */
static T_VOID ts_send_value(const T_TSPOINT *pt)
{
    T_BOOL ret = AK_FALSE;
    T_TS_BUF_CTRL *buf_ctrl = &m_hal_ts.buf_ctrl;
    extern T_U32 pc_lr;

    //print pc value if ts buffer is full
    if (((buf_ctrl->ts_tail + 1) % TS_VALUE_BUF_SIZE) == buf_ctrl->ts_head)
    {
        akprintf(C2, M_DRVSYS, "ts_send_value buf is full\r\n");
        akprintf(C2, M_DRVSYS, "pc_lr=0x%x\n", pc_lr);
    }

    //save pt to global ts buffer
    buf_ctrl->ts_buf[buf_ctrl->ts_tail] = *pt;
    buf_ctrl->ts_tail = (buf_ctrl->ts_tail + 1) % TS_VALUE_BUF_SIZE;

    //send ts message
    if (AK_NULL != m_hal_ts.callback_func)
    {
        m_hal_ts.callback_func();
    }
}

