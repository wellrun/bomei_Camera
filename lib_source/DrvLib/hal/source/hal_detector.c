/**
 * @file    hal_detector.c
 * @brief   detector module, for detecting device connected or disconnected
 *          by check gpio or voltage of ADC.
 *          The detected event of gpio can be indicated by interrupt of the 
 *          gpio,or by a timer.
 *          The detected event of ADC is indicated by a timer.
 * 支持防抖处理: 
 *     断开会马上响应;
 *     接合会有防抖处理，防抖时间为1秒钟: 当接合时，若1秒内没有出现断开，即为
 *     有效接合。
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.03.09
 * @version 1.0
 */



#include <string.h>
#include "arch_init.h"
#include "anyka_types.h"
#include "hal_print.h"
#include "drv_gpio.h"
#include "drv_module.h"
#include "hal_timer.h"
#include "arch_analog.h"
#include "hal_detector.h"


#define GPIO_PROTECT(gpionum)                  //add the implement
#define GPIO_UNPROTECT(gpionum)                //add the implement 


#define DETECT_MESSAGE        3


#define  DETECT_DEV_MAX       32
#define  DEV_TIMER_MAX        DETECT_DEV_MAX
#define  DEV_INT_QUEUE_LEN    DETECT_DEV_MAX


#define  MAX_INTERVAL_CHECK   50          //ms
#define  SHAKE_CHECK_TIME     1000        //ms
#define  VOLTAGE_CHECK_TIME   100         //ms

#if (SHAKE_CHECK_TIME < VOLTAGE_CHECK_TIME)
#error "must SHAKE_CHECK_TIME >= VOLTAGE_CHECK_TIME"
#endif


/**
 * @brief type of detect object.
 */
typedef enum
{
    eDETECT_OT_GPIO_INT = 0,
    eDETECT_OT_GPIO_CHK,  
    eDETECT_OT_ADC_CHK
}T_eDETECT_OBJ_TYPE;


/**
 * @brief structure for device node.
 */
typedef struct 
{
    T_pCSTR               pdevname;     ///< device name
    T_eDETECT_OBJ_TYPE    type;         ///< detecting type
    T_VOID                *pInfo;       ///< inof about the device
    T_fDETECTOR_CALLBACK  cb;           ///< call back function of the device
}T_DETECT_DEV_LIST;

/**
 * @brief structure for gpio info.
 */
typedef struct
{
    T_U32    gpionum;           ///< number of the gpio
    T_BOOL   active_level;      ///< active level
    T_BOOL   enable;            ///< enable flag
    T_BOOL   connect_state;     ///< connected state
    T_U32    interval_ms;       ///< interval, unit: ms
    T_U32    shake_count;       ///< avoid shaking time counter
}T_DETECT_GPIO_INFO;                


/**
 * @brief structure for adc info.
 */
typedef struct
{
    const T_VOLTAGE_TABLE  *pvoltage_table;     ///< volatage tabel
    T_U32                   voltageitem_num;    ///< number of voltage item
    T_U32                   voltage_departure;  ///< min of voltage departure
    T_U32                   interval_ms;        ///< interval, unit: ms
    T_U32                   dev_list_start;     ///< the index of first device
    T_U32                   enable_bit;         ///< bitmap of enabling
    T_U32                   connect_state;      ///< bitmap of connected flag
    T_U32                   ad_value;           ///< the value of ad value read last time
    T_U32                   ad_chk_count;       ///< steady adc value checking time
    T_U32                  *pshake_count;       ///< avoid shaking time counters
}T_DETECT_ADC_INFO;                 

/**
 * @brief structure for timer maping to device.
 */
typedef struct
{
    T_TIMER            hTimer;                  ///< timer id
    T_DETECT_DEV_LIST *pdev_list_node;          ///< pointer to device node
}T_DEV_TIMER;                       


/**
 * @brief a queue for each device detected by a gpio interrupted.
 */
typedef struct
{
    T_U8               dev_list_index[2];
    T_U8               front;
    T_U8               rear;
}T_DEV_INT_QUEUE;
#define  QUEUE_INVALID_INDXE    0xFF


static T_VOID detector_unregister(T_VOID);
static T_VOID detector_set2default(T_VOID);




static T_BOOL            bdetect_module_init = AK_FALSE;    ///< detect module init flag
static T_DETECT_DEV_LIST dev_list[DETECT_DEV_MAX];          ///< device node list
static T_DEV_TIMER       dev_timer[DEV_TIMER_MAX];          ///< timer list
static T_DEV_INT_QUEUE   dev_int_queue[DEV_INT_QUEUE_LEN];  ///< queue list for gpio interrupted



/**
 * @brief       Get the device node by device name
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device.
 * @return      T_DETECT_DEV_LIST
 * @retval      If the return value is not AK_NULL, means that the return 
                value is the device node;
 *              If the return value is AK_NULL, means that the device named
 *              by devname is no exist.
 */ 
static T_DETECT_DEV_LIST * get_dev_list_node_byname(T_pCSTR devname)
{
    static T_DETECT_DEV_LIST * plast_access_node = AK_NULL; //cache for run fast
    T_U32                      i;

    if((AK_NULL == devname) || (0 == devname[0]))
    {
        akprintf(C2, M_DRVSYS, "Device name is empty\n");
        return AK_NULL;
    }
    
    if((AK_NULL != plast_access_node) && 
        (!strcmp(devname, plast_access_node->pdevname)))
    {
        return plast_access_node;
    }

    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            akprintf(C2, M_DRVSYS, "Device %s not exist\n", devname);
            return AK_NULL;
        }

        if(!strcmp(devname, dev_list[i].pdevname))
        {
            plast_access_node = &dev_list[i];
            return plast_access_node;
        }
    }

    akprintf(C2, M_DRVSYS, "Device %s not exists.\n", devname);
    return AK_NULL;
    
}


/**
 * @brief       Get the connect state of the device by gpio level.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   gpionum
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @return      T_BOOL
 * @retval      If the return value is AK_TRUE, means the device is 
 *              connected.
 *              If the return value is AK_FALSE, means the device is 
 *              disconnected.
 */ 
static T_BOOL gpio_get_state_bynum(T_U32 gpionum, T_BOOL active_level)
{
    if(gpio_get_pin_level(gpionum) == active_level)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}


/**
 * @brief       Get the connect state of the device named by devname.
 *              The device is detected by a gpio.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a T_BOOL type variable for fetching 
 *                  the connecting state.
 * @param[in]   
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL gpio_get_state_byname(T_pCSTR devname, T_BOOL *pState)
{
    T_DETECT_DEV_LIST * pdev_list_node;


    pdev_list_node = get_dev_list_node_byname(devname);
    if(AK_NULL == pdev_list_node)
    {
        return AK_FALSE;
    }
    
    *pState = gpio_get_state_bynum(
        ((T_DETECT_GPIO_INFO *)(pdev_list_node->pInfo))->gpionum,
        ((T_DETECT_GPIO_INFO *)(pdev_list_node->pInfo))->active_level);

    return AK_TRUE;
}


/**
 * @brief       Insert the device node index into the queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   dev_list_index
 *                  Device node index.
 * @return      T_BOOL              
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL queue_in_int_dev(T_U8 dev_list_index)
{
    T_U32 i;


    for(i=0; i<DEV_INT_QUEUE_LEN; ++i)
    {
        if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) || 
            (dev_list_index == dev_int_queue[i].dev_list_index[0]))
        {
            break;
        }
    }

    if(i >= DEV_INT_QUEUE_LEN)
    {
        return AK_FALSE;
    }

    if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) || 
        (dev_int_queue[i].front == dev_int_queue[i].rear)) //queue empty
    {
        dev_int_queue[i].dev_list_index[dev_int_queue[i].rear] = 
            dev_list_index;

        /* Enqueue, so
         * rear = (rear + 1) % queue_lenght
         * But for this case, queue_length = 2, so rear = 0 or 1;
         * so do this simply like followed.
         */
        dev_int_queue[i].rear = dev_int_queue[i].rear ^ 0x1;
    }

    //queue full , do nothing
    
    return AK_TRUE;
}



/**
 * @brief       Remove the device node index from the queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   queue_node
 *                  The device node index in the queue which index is 
 *                  queue_node will be remove.
 * @return      T_BOOL              
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL queue_out_int_dev(T_U32 queue_node)
{    
    /*empty ?*/
    if((queue_node >= QUEUE_INVALID_INDXE) || 
        (dev_int_queue[queue_node].front == dev_int_queue[queue_node].rear))
    {
        return AK_FALSE;
    }

    /* dequeue, so
     * front = (front + 1) % queue_lenght
     * But for this case, queue_length = 2, so front = 0 or 1;
     * so do this simply like followed.
     */
    dev_int_queue[queue_node].front = dev_int_queue[queue_node].front ^ 0x1; 

    return AK_TRUE;;
}


/**
 * @brief       Read the device node index from the queue,but not remove the
 *              device node index.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pqueue_node
 *                  Ponit to a variable which indicate the first index of the 
 *                  queue.
 *                  When return ,the variable indicate the index of queue which
 *                  contain the device node index.
 * @return      T_U8              
 * @retval      If the function succeeds, the return value is the device node 
 *              index;
 *              If the function fails, the return value is QUEUE_INVALID_INDXE.
 */ 
static T_U8 queue_read_int_dev(T_U32 *pqueue_node)
{
    T_U32 i;

    for(i=(*pqueue_node); i<DEV_INT_QUEUE_LEN; ++i)
    {
        if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) ||
            (dev_int_queue[i].front != dev_int_queue[i].rear))
        {
            break;
        }
    }

    if((i >= DEV_INT_QUEUE_LEN) || 
        (QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]))
    {
        return QUEUE_INVALID_INDXE;          }

    *pqueue_node = i;

    return dev_int_queue[i].dev_list_index[dev_int_queue[i].front];    
}



/**
 * @brief       Call back function of the gpio interrupt,used to insert
 *              the device node index into the dev_int_queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   gpionum
 *                  Gpio which has generated an interrupt;
 * @param[in]   level
 *                  The logic level of the gpio when it generated an 
 *                  interrupt.
 * @return      T_VOID
 */ 
static T_VOID gpio_int_callback( T_U32 gpionum, T_U8 level)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_U32               param[7];
    T_U32               i;

    
    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }
        
        if(eDETECT_OT_GPIO_INT == dev_list[i].type)
        {
            pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
            if(pgpio_info->gpionum == gpionum)
            {                
                if(!queue_in_int_dev(i))
                {
                    akprintf(C2, M_DRVSYS,"interrupt is too frequent!\n");                    
                }

                level = gpio_get_pin_level(gpionum);
                gpio_set_int_p(gpionum, ! level);
                gpio_int_control(gpionum, AK_TRUE);
                return;
            }
        }
    }

    akprintf(C2, M_DRVSYS,"\nMemory may be mussed!!!!\n\n");
    gpio_int_control(gpionum, AK_FALSE);
}



/**
 * @brief       gpio check function.if the device connected or disconnected, 
 *              the pcallbackfunc function seted by detector_set_callback()
 *              will be called.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pdev_list_node
 *                  Pointer pointed to the device node.
 * @param[in]   bshake_chk
 *                  Avoid shaking flag.
 *                  AK_TRUE: checking with avoid_shaking;
 *                  AK_FALSE:checking without avoid_shaking;
 * @param[in]   bcallback
 *                  AK_TRUE: The call back function will be called;
 *                  AK_FALSE:The call back function won't be called;
 * @param[in]   pstate
 *                  If the pstate is not AK_NULL, the connect state will be
 *                  returned by pstate;
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL gpio_check_state(T_DETECT_DEV_LIST *pdev_list_node, 
                    T_BOOL bshake_chk, T_BOOL bcallback, T_BOOL *pstate)
{
    T_DETECT_GPIO_INFO       *pgpio_info;
    T_BOOL                    connect_state;


    pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;

    if(eDETECT_OT_GPIO_CHK == pdev_list_node->type)
    {
        GPIO_PROTECT(pgpio_info->gpionum);         
        gpio_set_pin_as_gpio(pgpio_info->gpionum);
        //gpio_set_pin_dir(pgpio_info->gpionum, GPIO_DIR_INPUT);
        //gpio_set_pull_up_r(pgpio_info->gpionum, AK_FALSE);        
    }

    connect_state = gpio_get_state_bynum(pgpio_info->gpionum, 
        pgpio_info->active_level);

    if(eDETECT_OT_GPIO_CHK == pdev_list_node->type)
    {
        GPIO_UNPROTECT(pgpio_info->gpionum);
    }

    if(AK_NULL != pstate)
    {
        *pstate = connect_state;
    }


    if(AK_FALSE == bcallback)
    {
        return AK_TRUE;
    }
    

    if(bshake_chk && connect_state)
    {
        pgpio_info->shake_count = 
            pgpio_info->shake_count + pgpio_info->interval_ms;
        if(pgpio_info->shake_count < SHAKE_CHECK_TIME)
        {            
            return AK_FALSE;
        }
    }

    pgpio_info->shake_count = 0;
    
    if(pgpio_info->connect_state != connect_state)
    {
        if(pgpio_info->enable)
        {
            pgpio_info->connect_state = connect_state;
            pdev_list_node->cb(connect_state);
        }        
    }

    return AK_TRUE;
}


/**
 * @brief       ADC check function.if the device connected or disconnected, 
 *              the pcallbackfunc function seted by detector_set_callback()
 *              will be called.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pdev_list_node
 *                  Pointer pointed to the device node.
 * @param[in]   bshake_chk
 *                  Avoid shaking flag.
 *                  AK_TRUE: checking with avoid_shaking;
 *                  AK_FALSE:checking without avoid_shaking;
 * @param[in]   bcallback
 *                  AK_TRUE: The call back function will be called;
 *                  AK_FALSE:The call back function won't be called;
 * @param[in]   pstate
 *                  If the pstate is not AK_NULL, the connect state will be
 *                  returned by pstate;
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL adc_check_state(T_DETECT_DEV_LIST *pdev_list_node, 
                    T_BOOL bshake_chk, T_BOOL bcallback, T_BOOL *pstate)
{
    static const T_VOLTAGE_TABLE   *plast_access_node = AK_NULL; //cache for run fast
    const T_VOLTAGE_TABLE          *pvoltage_table;
    T_DETECT_ADC_INFO              *padc_info;
    T_U32                           ad5value;    
    T_U32                           connect_state;
    T_U32                           connect_change_bit;    
    T_U32                           i;
    T_BOOL                          bconnect;

  
    padc_info = (T_DETECT_ADC_INFO *)pdev_list_node->pInfo;

    ad5value = analog_getvalue_ad5();

    if(bshake_chk)
    {
        if(((ad5value > padc_info->ad_value) && 
            (ad5value - padc_info->ad_value > padc_info->voltage_departure)) ||
            ((ad5value < padc_info->ad_value) && 
            (padc_info->ad_value - ad5value > padc_info->voltage_departure)))
        {            
            padc_info->ad_value = ad5value;
            padc_info->ad_chk_count = VOLTAGE_CHECK_TIME;
            return AK_FALSE;
        }

        if(padc_info->ad_chk_count >=  padc_info->interval_ms)
        {
            padc_info->ad_chk_count =  
                padc_info->ad_chk_count - padc_info->interval_ms;
            return AK_FALSE;
        }

    }
    

    connect_state = padc_info->connect_state;

    if((AK_NULL != plast_access_node) && 
        ((ad5value >= plast_access_node->min_voltage) && 
        (ad5value <= plast_access_node->max_voltage)))
    {
        connect_state = plast_access_node->dev_connect_state;
    }
    else
    {
        pvoltage_table = padc_info->pvoltage_table;
        for(i=0; i < padc_info->voltageitem_num; ++i)
        {
            if((ad5value >= pvoltage_table[i].min_voltage) && 
                (ad5value <= pvoltage_table[i].max_voltage))
            {
                connect_state = pvoltage_table[i].dev_connect_state;
                plast_access_node = &pvoltage_table[i];
                break;
            }
        }
     
    }


    if(AK_NULL != pstate)
    {
        i = pdev_list_node - &dev_list[padc_info->dev_list_start];
        
        *pstate = connect_state & (1<<i) ? AK_TRUE : AK_FALSE;
    }

    
    if(AK_FALSE == bcallback)
    {
        return AK_TRUE;
    }

    connect_change_bit = connect_state ^ padc_info->connect_state;

    if(0 == connect_change_bit)
    {
        return AK_TRUE;
    }
    
    for(i=0; connect_change_bit; ++i)
    {
        if(i > DETECT_DEV_MAX-1)
        {
            akprintf(C2, M_DRVSYS, "data error\n");
            break;
        }

        ad5value = connect_change_bit & 0x1;    //ad5value use for tmp        
        connect_change_bit = connect_change_bit >> 1;
        
        if(ad5value)
        {
            bconnect = connect_state & (1<<i) ? AK_TRUE : AK_FALSE;

            if(bshake_chk && bconnect)
            {
                padc_info->pshake_count[i] = 
                    padc_info->pshake_count[i] + padc_info->interval_ms;
                
                if(padc_info->pshake_count[i] < 
                    SHAKE_CHECK_TIME - VOLTAGE_CHECK_TIME)
                {
                    continue;                    
                }
            }

            padc_info->pshake_count[i] = 0;
                
            if(padc_info->enable_bit & (1<<i))
            {
                if(bconnect)
                {
                    padc_info->connect_state |= 1<<i;
                }
                else
                {
                    padc_info->connect_state &= ~(1<<i);
                }
                
                dev_list[padc_info->dev_list_start + i].cb(bconnect);
            }
        }
    }

    return AK_TRUE;    
}



/**
 * @brief       Call back function of the timer,used to check the gpio or
 *              ADC.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   timer_id
 *                  timer_id is used to map timer to the device;
 * @param[in]   delay
 * @return      T_VOID
 */ 
static T_VOID timer_chk_callback(T_TIMER timer_id, T_U32 delay)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32               dev_list_index;
    T_U32               i;
    

    if(timer_id == dev_timer[DEV_TIMER_MAX - 1].hTimer)
    {
        /*
         * careful, i must be 0,when call queue_read_int_dev()
         * at first each time.
         */
        i = 0;          
        dev_list_index = queue_read_int_dev(&i);
        while(QUEUE_INVALID_INDXE != dev_list_index)
        {
            if(gpio_check_state(&dev_list[dev_list_index], AK_TRUE, 
                AK_TRUE, AK_NULL))
            {
                queue_out_int_dev(i);
            }

            /*careful, check from the next queue*/
            ++i;       
            dev_list_index = queue_read_int_dev(&i);
        }

        return ;
    }

    for(i=0; (i < DEV_TIMER_MAX - 1) && (ERROR_TIMER != dev_timer[i].hTimer); ++i)
    { 
        if(timer_id == dev_timer[i].hTimer)
        {   		
            switch((dev_timer[i].pdev_list_node)->type)
            {
                case eDETECT_OT_GPIO_CHK: 
                    pgpio_info = (T_DETECT_GPIO_INFO *)
                        ((dev_timer[i].pdev_list_node)->pInfo);
                    if(AK_FALSE == pgpio_info->enable)
                    {
                        break;
                    }
                    
                    gpio_check_state(dev_timer[i].pdev_list_node, AK_TRUE, 
                        AK_TRUE, AK_NULL);
                    break;
                case eDETECT_OT_ADC_CHK:
                    padc_info = (T_DETECT_ADC_INFO *)
                        ((dev_timer[i].pdev_list_node)->pInfo);
                    if(0 == padc_info->enable_bit)
                    {
                        break;
                    }
                    
                    adc_check_state(dev_timer[i].pdev_list_node, AK_TRUE, 
                        AK_TRUE, AK_NULL);
                    break;
                default:
                    return;
                    break;
            }
            
            break;
        }
    }
}


/**
 * @brief       Init detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID 
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_init(T_VOID)
{
    T_U32  i;

    if(AK_TRUE == bdetect_module_init)
    {
        return AK_TRUE;
    }
    
    for(i=0; i < DEV_TIMER_MAX; ++i)
    {
        dev_timer[i].hTimer = ERROR_TIMER;
    }

    for(i=0; i < DEV_INT_QUEUE_LEN; ++i)
    {
        dev_int_queue[i].dev_list_index[0] = 
            dev_int_queue[i].dev_list_index[1] = QUEUE_INVALID_INDXE;
        dev_int_queue[i].front = dev_int_queue[i].rear = 0;
    }
    
    bdetect_module_init = AK_TRUE;
    return AK_TRUE;    
}

/**
 * @brief       Free detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_free(T_VOID)
{
    if(AK_FALSE == bdetect_module_init)
    {
        return AK_FALSE;
    }
    
    detector_set2default();    
    bdetect_module_init = AK_FALSE;
    return AK_TRUE;
}




/**
 * @brief       Set the call back function of device named by devname,the call 
 *              back function will be call when the device inserted or removed.
 *              After seting the call back function, the dettector of the 
 *              device will start automatically.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pcallbackfunc
 *                  Call back function of the device.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_set_callback(T_pCSTR devname,
            T_fDETECTOR_CALLBACK pcallbackfunc)
{
    T_DETECT_DEV_LIST * pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;    
    T_BOOL              gpio_level;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32               adc_no;
    T_U32               i,j;



    if(AK_FALSE == bdetect_module_init)
    {
        akprintf(C2, M_DRVSYS, "Must call detector_init() first!\n");
        return AK_FALSE;
    }
    
    if(AK_NULL == pcallbackfunc)
    {
        akprintf(C2, M_DRVSYS, "Callback function is null!\n");
        return AK_FALSE;
    }

    pdev_list_node = get_dev_list_node_byname(devname);
    if(AK_NULL == pdev_list_node)
    {
        return AK_FALSE;
    }
    
    pdev_list_node->cb = pcallbackfunc;


    if((eDETECT_OT_GPIO_INT == pdev_list_node->type) || 
        (eDETECT_OT_GPIO_CHK == pdev_list_node->type))
    {
        pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);

        gpio_set_pin_as_gpio(pgpio_info->gpionum);
        gpio_set_pin_dir(pgpio_info->gpionum, GPIO_DIR_INPUT);
        gpio_set_pull_up_r(pgpio_info->gpionum, AK_FALSE);
        mini_delay(1);              // discharge 
        gpio_level = gpio_get_pin_level(pgpio_info->gpionum);        
    }


    switch(pdev_list_node->type)    
    {
        case eDETECT_OT_GPIO_INT:
            i = DEV_TIMER_MAX-1;
            if(AK_NULL == dev_timer[i].pdev_list_node)
            {
                dev_timer[i].hTimer = vtimer_start(MAX_INTERVAL_CHECK, 
                    AK_TRUE, timer_chk_callback);
                if(ERROR_TIMER == dev_timer[i].hTimer)
                {
                    akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                    return AK_FALSE;
                }
                dev_timer[i].pdev_list_node = pdev_list_node;                 
            }

            pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);
            gpio_register_int_callback(pgpio_info->gpionum, ! gpio_level, 
                AK_TRUE, gpio_int_callback);

            pgpio_info->interval_ms = MAX_INTERVAL_CHECK;
                       

            /* If the device is connected when the system start up,
             * there is not a interrupt, so do ... 
             */
            gpio_check_state(pdev_list_node, AK_FALSE, 
                    AK_FALSE, &(pgpio_info->connect_state)); 
            pgpio_info->enable = AK_TRUE; 
            if(pgpio_info->connect_state)
            {
                pcallbackfunc(AK_TRUE);
            }
            
            break;
            
        case eDETECT_OT_GPIO_CHK:
            for(i=0; i<DEV_TIMER_MAX; ++i)
            {
                if(AK_NULL == dev_timer[i].pdev_list_node)
                {
                    break;
                }
            }

            if(i >= DEV_TIMER_MAX)
            {
                akprintf(C1, M_DRVSYS, "Timer number is over %d\n", DEV_TIMER_MAX);
                return AK_FALSE;
            }

            pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);

            j = pgpio_info->interval_ms < MAX_INTERVAL_CHECK ? 
                    MAX_INTERVAL_CHECK : pgpio_info->interval_ms;

            dev_timer[i].hTimer = vtimer_start(j, AK_TRUE, timer_chk_callback);
            if(ERROR_TIMER == dev_timer[i].hTimer)
            {
                akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                return AK_FALSE;
            }
            dev_timer[i].pdev_list_node = pdev_list_node;
            pgpio_info->interval_ms     = j;

            gpio_check_state(pdev_list_node, AK_FALSE, 
                    AK_FALSE, &(pgpio_info->connect_state)); 
            
            pgpio_info->enable = AK_TRUE; 
            
            if(pgpio_info->connect_state)
            {
                pcallbackfunc(AK_TRUE);
            }
            
            break;
            
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            adc_no = pdev_list_node - &dev_list[padc_info->dev_list_start];            
            pdev_list_node = &dev_list[padc_info->dev_list_start];
            
            for(i=0; i<DEV_TIMER_MAX; ++i)
            {
                if((pdev_list_node == dev_timer[i].pdev_list_node) || 
                    (AK_NULL == dev_timer[i].pdev_list_node))
                {
                    break;
                }
            }

            if(i >= DEV_TIMER_MAX)
            {
                akprintf(C2, M_DRVSYS, "Timer number is over %d\n", DEV_TIMER_MAX);
                return AK_FALSE;
            }

            if(AK_NULL == dev_timer[i].pdev_list_node)
            {
                j = padc_info->interval_ms < MAX_INTERVAL_CHECK ? 
                        MAX_INTERVAL_CHECK : padc_info->interval_ms;
                
                dev_timer[i].hTimer = vtimer_start(j, AK_TRUE, timer_chk_callback);
                if(ERROR_TIMER == dev_timer[i].hTimer)
                {
                    akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                    return AK_FALSE;
                }
                
                dev_timer[i].pdev_list_node = pdev_list_node;
                padc_info->interval_ms      = j; 
            } 

            adc_check_state(pdev_list_node + adc_no, AK_FALSE, 
                        AK_FALSE, &gpio_level);     //gpio_level use for tmp

            if(gpio_level)
            {
                padc_info->connect_state |= 1<<adc_no;
            }

            padc_info->enable_bit |= 1<<adc_no;            

            if(gpio_level)
            {                      
                pcallbackfunc(AK_TRUE);
            }
            
            break;
            
        default:
            akprintf(C2, M_DRVSYS,"\nMemory may be mussed!!!!\n\n");
            return AK_FALSE;
    }

    return AK_TRUE;
}


/**
 * @brief       Enable or disable the detector.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   benable
 *                  AK_TRUE：Enable the detector;
 *                  AK_FALSE：Disable the detector;
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 * @remark      It's not suggested to disable the detector,if the detector
 *              is disable, the connecting state of the device can't be 
 *              informed the user.
 *
 */ 
T_BOOL detector_enable(T_pCSTR devname,T_BOOL benable)
{    
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32               i;

    pdev_list_node = get_dev_list_node_byname(devname);
    if(AK_NULL == pdev_list_node)
    {
        return AK_FALSE;
    }

    if(AK_NULL == pdev_list_node->cb)
    {
        akprintf(C2, M_DRVSYS, "Must call detector_set_callback() first!\n");
        return AK_FALSE;
    }


    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            gpio_int_control(pgpio_info->gpionum, benable);
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            pgpio_info->enable = benable;
            break;
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            i = pdev_list_node - &dev_list[padc_info->dev_list_start];
            if(benable)
            {
                padc_info->enable_bit |= 1<<i;
            }
            else
            {
                padc_info->enable_bit &= ~(1<<i);
            }
            break;
        default:
            break;
    }

    return AK_TRUE;
}

/**
 * @brief       Determine whether the specified window is enabled.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pbenable
 *                  Pointer to a T_BOOL type variable for fetching the 
 *                  detector state.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_is_enabled(T_pCSTR devname, T_BOOL *pbenable)
{
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32               i;

    pdev_list_node = get_dev_list_node_byname(devname);
    if(AK_NULL == pdev_list_node)
    {
        return AK_FALSE;
    }


    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            *pbenable  = pgpio_info->enable;
            break;
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            i = pdev_list_node - &dev_list[padc_info->dev_list_start];
         
            *pbenable = (padc_info->enable_bit & (1<<i))? AK_TRUE : AK_FALSE;
            break;
        default:
            break;
    }

    return AK_TRUE;
}



/**
 * @brief       Get the connecting state of the device named by devname.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a T_BOOL type variable for fetching the 
 *                  connecting state.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_get_state(T_pCSTR devname, T_BOOL *pState)
{
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32               adc_no;
    T_BOOL              state;


    pdev_list_node = get_dev_list_node_byname(devname);
    if(AK_NULL == pdev_list_node)
    {
        return AK_FALSE;
    }
    

    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            if(AK_FALSE == pgpio_info->enable)
            {
                gpio_check_state(pdev_list_node, AK_FALSE, AK_FALSE, &state);
            }
            else
            {                
                state = pgpio_info->connect_state;                
            }
            break;            
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)pdev_list_node->pInfo;
            adc_no = pdev_list_node - &dev_list[padc_info->dev_list_start];
            if(0 == (padc_info->enable_bit & (1<<adc_no)))
            {
                adc_check_state(pdev_list_node, AK_FALSE, AK_FALSE, &state);
            }  
            else
            {       
                state = (padc_info->connect_state) & (1<<adc_no) ? AK_TRUE : AK_FALSE;
            }
            break;
        default:
            break;         
    }

    *pState = state;
    return AK_TRUE;
}



/**
 * @brief       Set the detect module the default state just after registered.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
static T_VOID detector_set2default(T_VOID)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    T_U32 i;

    for(i=0; i < DEV_TIMER_MAX; ++i)
    {
        if(AK_NULL != dev_timer[i].pdev_list_node)
        {
            vtimer_stop(dev_timer[i].hTimer);
        }
    }
    memset(dev_timer, 0, sizeof(dev_timer));    

    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }

        switch(dev_list[i].type)
        {
            case eDETECT_OT_GPIO_INT:
                pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
                gpio_int_control(pgpio_info->gpionum, AK_FALSE);
            case eDETECT_OT_GPIO_CHK:
                pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
                pgpio_info->enable = AK_FALSE;
                pgpio_info->connect_state = AK_FALSE;
                break;
            case eDETECT_OT_ADC_CHK:
                padc_info = (T_DETECT_ADC_INFO *)(dev_list[i].pInfo);         
                padc_info->enable_bit = 0;   
                padc_info->connect_state = 0;
                break;
            default:
                break;
        }
    } 
    
    memset(&dev_int_queue, 0, sizeof(dev_int_queue));    }
    

/**
 * @brief       Unregister all the devices that has registered to the
 *              detect module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID 
 * @return      T_VOID
 */ 
static T_VOID detector_unregister(T_VOID)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_VOID             *ptmp = AK_NULL;
    T_U32 i;


    detector_set2default();

    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }
        
        switch(dev_list[i].type)
        {
            case eDETECT_OT_GPIO_INT:
            case eDETECT_OT_GPIO_CHK:
                drv_free(dev_list[i].pInfo);
                break;
            case eDETECT_OT_ADC_CHK:
                if(AK_NULL == ptmp)
                {
                    ptmp = dev_list[i].pInfo;
                    continue;
                }
                else if(ptmp == dev_list[i].pInfo)
                {
                    continue;
                }
                else     //(ptmp != dev_list[i].pInfo)
                {
                    drv_free(ptmp);
                    ptmp = dev_list[i].pInfo;
                }

                break;
            default:
                break;
        }
    }

    if(AK_NULL != ptmp)
    {
        drv_free(ptmp);
    }

    memset(dev_list, 0, sizeof(dev_list)); 
    
}



/**
 * @brief       Register the detector of GPIO type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 *                  devname must point to a const string, because the detect 
 *                  module won't hold a copy of the device name. 
 * @param[in]   gpio_num
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @param[in]   interrupt_mode
 *                  Detect type, AK_TRUE: interrupt, AK_FALSE: time.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */
T_BOOL detector_register_gpio(T_pCSTR devname, T_U32 gpio_num, 
            T_BOOL active_level, T_BOOL interrupt_mode, T_U32 interval_ms)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_U32               i;

    /* Checking if the device name is effective ?
     * Is the device has been registered ?
     */
    if((AK_NULL == devname) || (0 == devname[0]))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "device name is empty\n");
        return AK_FALSE;
    }

    if(AK_NULL != get_dev_list_node_byname(devname))
    {
        akprintf(C1, M_DRVSYS, "Device %s has registered\n", devname);
        return AK_FALSE;
    }    
    /* Checking if the gpio number is effective ?
     */
    if((gpio_num == INVALID_GPIO) || (gpio_num >= GPIO_NUMBER))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "gpio number is invalid\n");
        return AK_FALSE;   
    }
    
    /* Get a valid device node     
     */
    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            break;
        }
    }

    if(i >= DETECT_DEV_MAX)
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "detect device number is over %d\n", DETECT_DEV_MAX);
        return AK_FALSE;
    }

    /* Malloc memory for adc info.
     */
    pgpio_info = (T_DETECT_GPIO_INFO *)drv_malloc(sizeof(T_DETECT_GPIO_INFO));
    if(AK_NULL == pgpio_info)
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): malloc failed\n");
        return AK_FALSE;
    }

    pgpio_info->gpionum       = gpio_num;
    pgpio_info->active_level  = active_level;
    pgpio_info->connect_state = AK_FALSE;
    pgpio_info->interval_ms   = interval_ms;
    pgpio_info->enable        = AK_FALSE;
    pgpio_info->shake_count   = 0;

    dev_list[i].pdevname = devname;
    dev_list[i].type     = interrupt_mode ? eDETECT_OT_GPIO_INT : eDETECT_OT_GPIO_CHK;
    dev_list[i].pInfo    = pgpio_info;
    dev_list[i].cb       = AK_NULL;
    
    return AK_TRUE;    
}


 
 /**
  * @brief       Register the detector of ADC type.
  * @author      wangguotian
  * @date        2012.03.09
  * @param[in]   devname_list
  *                  Name list of the devices to be detected.
  *                  Each pointer in the name list must point to a const 
  *                  string, because the detect module won't hold a copy 
  *                  of the device name. 
  * @param[in]   devnum
  *                  Number of devices to be detected.
  * @param[in]   pvoltage_table
  *                  Voltage table .
  *                  pvoltage_table must point to a const memory, because
  *                  the detect module won't hold a copy of the Voltage 
  *                  table.
  * @param[in]   voltageitem_num
  *                  Number of voltage item of voltage table.
  * @param[in]   interval_ms
  *                  The interval of checking, in ms.
  * @return      T_BOOL
  * @retval      If the function succeeds, the return value is AK_TRUE;
  *              If the function fails, the return value is AK_FALSE.
  *
  * @remark      The member dev_connect_state of T_VOLTAGE_TABLE is a bitmap 
  *              of the devices' connecting state.1 means the device is 
  *              connected,and 0 means the device is disconnected.
  *              The bitmap order must correspond to the device's name in 
  *              devname_list, that is, bit 0 of dev_connect_state correspond
  *              to the first device named by devname_list[0], bit 1 of 
  *              dev_connect_state correspond to the second device named by 
  *              devname_list[1].
  *              All the devices specified by devname_list, must have different
  *              name, or else, the action of detector is not foreseeable.
  */ 
T_BOOL detector_register_adc(T_pCSTR *devname_list, T_U32 devnum, 
            const T_VOLTAGE_TABLE  *pvoltage_table, T_U32 voltageitem_num, 
            T_U32 interval_ms)
{
    T_DETECT_ADC_INFO *padc_info;
    T_U32              voltage_departure;
    T_U32              i,j;

    /* Checking if device list and each device name are effective ?
     * Is the device has been registered ?
     */
    for(i=0; i<devnum; ++i)
    {
        if((AK_NULL == devname_list) || (AK_NULL == devname_list[i]) || 
            (0 == devname_list[i][0]))
        {
            akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                "device name is empty\n");  
            return AK_FALSE;
        }

        if(AK_NULL != get_dev_list_node_byname(devname_list[i]))
        {
            akprintf(C1, M_DRVSYS, "Device %s has registered\n", devname_list[i]);
            return AK_FALSE;
        }    }
    
    if(AK_NULL == pvoltage_table)
    {
        akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                "voltage_table is empty\n");  
        return AK_FALSE;
    }


    /* Checking if dev_connect_state in pvoltage_table is effective ?
     * Should be dev_connect_state < (1<<devnum) 
     */
    for(i=0; i < voltageitem_num; ++i)
    {        
        if((pvoltage_table[i].max_voltage < pvoltage_table[i].min_voltage) ||
            (pvoltage_table[i].dev_connect_state >= (1<<devnum)))
        {
            akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                           "voltage_table data error\n");  
            return AK_FALSE;
        }
    }
    

    /* Get a valid device node     
     */
    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(AK_NULL == dev_list[i].pdevname)
        {
            break;
        }
    }

    if((i >= DETECT_DEV_MAX) || (devnum > (DETECT_DEV_MAX - i)))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "detect device number is over %d\n", DETECT_DEV_MAX);
        return AK_FALSE;
    }


    /* Malloc memory for adc info and shake_count.
     */
    padc_info = (T_DETECT_ADC_INFO *)drv_malloc(sizeof(T_DETECT_ADC_INFO) + 
        sizeof(*(padc_info->pshake_count)) * devnum);    
    if(AK_NULL == padc_info)
    {
        akprintf(C1, M_DRVSYS, "detector_register_adc(): malloc failed\n");
        return AK_FALSE;
    }

    

    padc_info->pvoltage_table   = pvoltage_table;
    padc_info->voltageitem_num  = voltageitem_num;
    padc_info->interval_ms      = interval_ms;
    padc_info->dev_list_start   = i;
    padc_info->connect_state    = 0;
    padc_info->enable_bit       = 0;
    padc_info->ad_value         = 0;
    padc_info->ad_chk_count     = 0;
    padc_info->pshake_count     = 
        (T_VOID *)((T_U32)padc_info + sizeof(T_DETECT_ADC_INFO));

    memset(padc_info->pshake_count, 0, 
        sizeof(*(padc_info->pshake_count)) * devnum);

    /* Get the min voltage departure
     */
    padc_info->voltage_departure = ~0;
    for(j=0; j < voltageitem_num; ++j)
    {        
        voltage_departure = 
            pvoltage_table[j].max_voltage - pvoltage_table[j].min_voltage;
        voltage_departure = voltage_departure >> 1;
        if(voltage_departure < padc_info->voltage_departure)
        {
            padc_info->voltage_departure = voltage_departure;
        }     
    }


    while(devnum--)
    {
        dev_list[i].pdevname = *devname_list;
        dev_list[i].type     = eDETECT_OT_ADC_CHK;
        dev_list[i].pInfo    = padc_info;
        dev_list[i].cb       = AK_NULL;

        ++devname_list;
        ++i;
    }
    
    return AK_TRUE;
}



