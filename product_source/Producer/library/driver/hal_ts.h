/**@file hal_ts.h
 * @brief list touch screen operation interfaces.
 
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @date  2006-04-19
 * @version 1.0
 */
#ifndef _HAL_TS_H_
#define _HAL_TS_H_

/** @defgroup TouchScreen TouchScreen group
 *    @ingroup Drv_Lib
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#define TS_MAX_SUPPORT 5

/**
 * @brief touch panel  callback define
 *    define  touch panel callback type
 */
typedef T_VOID (*T_fTS_ADC_CALLBACK)(T_VOID);    

/**
 * @brief touch panel  callback define
 *  is use for gpio interrupt call back
 */
typedef T_BOOL (*T_fTS_PW_CALLBACK)(T_VOID);


/* type need get from other file in system*/ 
typedef struct {
    T_S32    x;
    T_S32    y;
    T_S32    x2;
    T_S32    y2;
    T_S32    finger_num;
}T_TSPOINT, *T_pTSPOINT;

typedef enum
{
    E_TS_TYPE_RES = 0,
    E_TS_TYPE_CAP,
    E_TS_TYPE_NUM
} E_TS_TYPE;


typedef enum
{
    PACKET_TYPE_ABSOLUTE = 0,      //can get one finger absolute coordinate
    PACKET_TYPE_INDEX              //can get two finger index coordinate
}E_PACKET_TYPE;

typedef struct 
{
    T_S32 max_x;
    T_S32 max_y;
}T_TS_COORDINATE;

typedef enum
{
    TS_CAP_SET_PACKET_TYPE = 0,
    TS_CAP_GET_PACKET_TYPE,
    TS_CAP_GET_MAX_COORDINATE,
    TS_CAP_GET_CUR_POINT,
    TS_CAP_GET_LASTDOWN_POINT,
    TS_CAP_EVENT_NUM
}E_TS_CAP_EVENT;

typedef union
{
    T_TS_COORDINATE max_coordinate;
    E_PACKET_TYPE packet_type;
    T_TSPOINT point;
}U_TS_CAP_PARAM;

typedef enum
{
    CTL_TYPE_INSIDE = 0,          //use chip inside control
    CTL_TYPE_EXTERNAL             //usb external ic control
}E_TS_CONTROL_TYPE;

//触摸屏接口参数
typedef struct{
    T_U32       PenDownCnt;     //落笔中断延时计数，unit:asic
    T_U16       TsAdcSampeRate; //ADC采样率
    T_U16       FontEndCount;   //字结束变量 
    T_U16       WordEndCount;   //笔结束变量
    T_U16       PenEndCount;    //落笔稳定计数
    T_U32       TsTypeId;       //自定义ID
    T_U8        Interval;       //定时器间隔时间
    T_U8        ts_threshold;   //最大值与最小值差值
    T_U8        ts_diff_min;    //相邻两点差值。
    T_U8        ts_control_type;//触摸屏控制器类型
    
    T_U32  (*ts_read_id_func)(T_VOID); 
    T_VOID (*ts_init_func)(T_U32 gpio, T_U8 gpioActiveLevel, T_fTS_PW_CALLBACK cb);
    T_VOID (*ts_read_value)(T_U32 *x1, T_U32 *y1, T_U32 *x2, T_U32 *y2);
    T_BOOL (*ts_check_pendown)(T_VOID);
    T_VOID (*ts_interrupt_enable)(T_VOID);
}T_TS_PARAM;

typedef struct
{
    T_U32       DeviceID;
    T_TS_PARAM  *handler;
}T_TS_INFO;


#define TS_HAND_WR_INVALID_COORD    0x7fff

T_BOOL ts_reg_dev(T_U32 id, T_TS_PARAM *handler);

T_TS_PARAM *ts_probe(T_VOID);

/**
 * @brief Initialize touch panel
 *
 * this function only need to be called one time when the system powers on !
 * But it must be called after gpio and timer init
 * @author liangenhui
 * @date 2006-04-26
 * @param[in] type touch screen type
 * @param[in] callback touchscreen callback function pointor
 * @param[in] gpio  gpio used to detect ts's interrupt
 * @param[in] active_level gpio level that will activate ts
 * @return T_VOID
 * @remark This callback function is very important in this driver! It will be called when the driver
 * get a sample point. These are two special point  (0x7fff,0x00) means pen up (0x7fff, 0x7fff) means fond end.
 */
T_VOID ts_init(E_TS_TYPE type, T_fTS_ADC_CALLBACK callback, T_U32 gpio, T_U8 active_level);

/**
 * @brief Get the current point's coordinate data
 *
 * @author liangenhui
 * @date 2006-04-26
 * @param[out] pt pointer of the current point's coordinate data
 * @return T_BOOL
 * @retval AK_FALSE means failed
 * @retval AK_TRUE means successful
 * @remark This function is use for calibrating touch screen .
 */
T_BOOL ts_get_cur_point(T_pTSPOINT pt);


/**
 * @brief Get the point from ts inner buffer
 *
 * @author liangenhui
 * @date 2006-04-26
 * @param[out] pt pointer to T_TSPOINT struct, (x,y) value will be saved here
 * @return T_BOOL
 * @retval AK_TRUE means get point successfully
 * @retval AK_FALSE means inner point buffer empty
 */
T_BOOL ts_get_value(T_TSPOINT *pt);


/**
 * @brief set ts cap param
 * @author guoshaofeng
 * @date 2008-04-29
 * @param[in] event refer to E_TS_CAP_EVENT definition
 * @param[in] param refer to U_TS_CAP_PARAM definition
 * @return T_BOOL
 * @retval AK_TURE set ts cap param successful
 * @retval AK_FALSE set ts cap param failed
 */
T_BOOL ts_cap_set_param(E_TS_CAP_EVENT event, U_TS_CAP_PARAM param);

/**
 * @brief get ts cap info
 * @author guoshaofeng
 * @date 2008-04-29
 * @param[in] event refer to E_TS_CAP_EVENT definition
 * @param[in] param pointer, refer to U_TS_CAP_PARAM definition
 * @return T_BOOL
 * @retval AK_TURE get ts cap info successful
 * @retval AK_FALSE get ts cap info failed
 */
T_BOOL ts_cap_get_info(E_TS_CAP_EVENT event, U_TS_CAP_PARAM *param);

/**
 * @brief close touch screen
 * @author LHS
 * @date 2012-01-07
 * @param[in]T_VOID
 * @return T_VOID
 */
T_VOID ts_cap_power_off(T_VOID);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef _HAL_TS_H_
