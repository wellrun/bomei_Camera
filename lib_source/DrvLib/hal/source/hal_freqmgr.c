/**
 * @filename hal_freq_manager.c
 * @brief: frequency manager file.
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  luheshan
 * @date    2012-05-16
 * @version 1.0
 */

#ifdef OS_ANYKA
#include    "anyka_types.h"
#include    "hal_freqmgr.h"
#include    "drv_module.h"
#include    "hal_print.h"
#include    "hal_timer.h"

#define FREQ_CHECK_CPU_UASGE_TIME       (200) //ms
#define FREQ_DELAY_DOWN_SET_TIME        (5000) //2S 5s

#define FREQ_UNLOCK                     (0) //no lock

#define FREQ_CPU_UASGE_FACTOR1          (80)
#define FREQ_CPU_UASGE_FACTOR2          (20)

#define FREQ_MAX_DRV_NUM                (32) //max driver list number

#define FREQ_MGR_PROTECT \
        do{ \
            DrvModule_Protect(DRV_MODULE_FREQ_MGR);\
        }while(0)
        
#define FREQ_MGR_UNPROTECT \
        do{ \
            DrvModule_UnProtect(DRV_MODULE_FREQ_MGR);\
        }while(0)

typedef struct 
{
    T_U32 asic_freq;
    T_U32 count;
}T_FREQ_NODE,*T_pFREQ_NODE;

typedef struct 
{
    FreqMgr_Callback        ctl_cpu_cb;
    T_TIMER                 ctl_timer;                  ///< timer id
    T_pFREQ_NODE            ctl_current;
    T_U32                   ctl_lock; //init FREQ_UNLOCK
    T_U32                   ctl_delay_down_time;
    T_U32                   ctl_add_drv_cnt;
    T_U32                   ctl_freq_node_max;
    T_U32                   ctl_drv_optimum_freq;
    T_BOOL                  ctl_init;
}T_FREQ_CONTROL,*T_pFREQ_CONTROL;

static T_FREQ_CONTROL m_freq_control;

static T_pFREQ_NODE   m_freq_node_list = AK_NULL;
static T_S32          m_drv_list[FREQ_MAX_DRV_NUM];

static T_VOID FreqMgr_CheckCpuUsageTimerCb(T_TIMER timer_id, T_U32 delay);
static T_pFREQ_NODE FreqMgr_GetNextASICFreq(T_BOOL IsUp);
static T_BOOL FreqMgr_AdjustFreq(T_pFREQ_NODE freq_node);
static T_U32 FreqMgr_GetCpuUsageFactor(T_VOID);


/**
 * @brief check timer call back,this function adjust asic frequency by cpu usage factor.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] timer_id
 * @param[in] delay
 * @return T_VOID
 * @retval T_VOID
 */
static T_VOID FreqMgr_CheckCpuUsageTimerCb(T_TIMER timer_id, T_U32 delay)
{
    T_U32 cpu_usage_factor = 0;
    T_pFREQ_NODE  node_next = AK_NULL;
    T_pFREQ_CONTROL control = &m_freq_control;
    T_BOOL is_need_adjust = AK_FALSE;
	static volatile T_U32 is_down_asic_freq = 0;
	static volatile T_U32 freq_change_count = 0;
	#define IS_DOWN_ASIC_COUNT 2
    
    FREQ_MGR_PROTECT; //protect
    
    //if lock,system do nothing
    if (control->ctl_lock != FREQ_UNLOCK)
    {
        FREQ_MGR_UNPROTECT; //unprotect
        return;
    }
        
    //get cpu usage factor.
    cpu_usage_factor = FreqMgr_GetCpuUsageFactor();
    freq_change_count++;

    if (cpu_usage_factor > FREQ_CPU_UASGE_FACTOR1) //up set asic
    {
        node_next = FreqMgr_GetNextASICFreq(AK_TRUE);
        is_need_adjust = AK_TRUE;
    }
    else if (cpu_usage_factor < FREQ_CPU_UASGE_FACTOR2) // down set asic
    {
        if (0 == is_down_asic_freq)
        {
            is_down_asic_freq = freq_change_count;
        }
        else
        {
            is_down_asic_freq++;
        }
		
        if (freq_change_count - is_down_asic_freq > IS_DOWN_ASIC_COUNT)
        {
            control->ctl_delay_down_time = 0;
            is_down_asic_freq = 0;
            FREQ_MGR_UNPROTECT; //unprotect
            return;  //wait delay time 
        }


		control->ctl_delay_down_time += delay;
        if (FREQ_DELAY_DOWN_SET_TIME > control->ctl_delay_down_time)
        {
            FREQ_MGR_UNPROTECT; //unprotect
            return;  //wait delay time 
        }
        
        //get down set asic frequency
        node_next = FreqMgr_GetNextASICFreq(AK_FALSE);
        is_need_adjust = AK_TRUE;
        control->ctl_delay_down_time = 0;
    }
    else // 20<=cpu_usage_factor<=80 no need to adjust asic frequency
    {}
        
    if (is_need_adjust && (node_next->asic_freq != control->ctl_current->asic_freq))
    {
        control->ctl_delay_down_time = 0;
        
        FreqMgr_AdjustFreq(node_next);
    }
    
    FREQ_MGR_UNPROTECT; //unprotect
}


/**
 * @brief get up/down set next asic frequency by current frequency
 * @author luheshan
 * @date 2012-05-18
 * @param[in] IsUp:AK_TRUE,is up set,AK_FLASH,is down set.
 * @return T_pFREQ_NODE
 * @retval values of the next freq node.
 */
static T_pFREQ_NODE FreqMgr_GetNextASICFreq(T_BOOL IsUp)
{
    T_U32 cur_index;
    T_U32 next_index = 0;

    //get current frequency list index
    cur_index = m_freq_control.ctl_current - &m_freq_node_list[0];

    //get next index
    if (cur_index < m_freq_control.ctl_freq_node_max)
    {
        if (IsUp) // will to set up asic frequency,index is current to max mid value
        {
            next_index = (m_freq_control.ctl_freq_node_max + cur_index) >> 1;
        }
        else //set down asic
        {
            if (m_freq_node_list[cur_index].asic_freq == m_freq_control.ctl_drv_optimum_freq)
            {
                next_index = cur_index;
            }
            else
            {
                next_index = cur_index - 1;
            }
        }
    }
    
    return &m_freq_node_list[next_index];
}


/**
 * @brief go to adjust asic frequency
 * @author luheshan
 * @date 2012-05-18
 * @param[in] freq_node:InPut asic frequency node
 * @return T_BOOL
 * @retval values of succeeds, the return value is AK_TRUE.
 */
static T_BOOL FreqMgr_AdjustFreq(T_pFREQ_NODE freq_node)
{
    T_U32 currFreq;
    
    if (AK_FALSE == set_asic_freq(freq_node->asic_freq))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:set asic freq fail\n");
        return AK_FALSE;
    }
    else //if success,change current frequency node
    {
        m_freq_control.ctl_current = freq_node;
    }
    
    return AK_TRUE;
}


/**
 * @brief get cpu usage factor by call back function what init had seted.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] T_VOID
 * @return T_U32
 * @retval values of cpu usage is update 
 */
static T_U32 FreqMgr_GetCpuUsageFactor(T_VOID)
{
    T_U32 cpu_usage = 0;
        
    if (AK_NULL != m_freq_control.ctl_cpu_cb)
    {
        cpu_usage = m_freq_control.ctl_cpu_cb();
    }

    return cpu_usage;
}

/**
 * @brief get asic list's node by frequency.
 * @author luheshan
 * @date 2012-06-08
 * @param[in] T_U32 Freq,the asic frequency
 * @return T_U32
 * @retval values of asic list's node
 */
static T_U32 FreqMgr_GetAsicListNode(T_U32 Freq)
{
    T_U32 i; 
    T_U32 asic_node = 0;
    
    if (Freq >= m_freq_node_list[m_freq_control.ctl_freq_node_max - 1].asic_freq)
    {
        asic_node = m_freq_control.ctl_freq_node_max - 1;
    }
    else if (Freq <= m_freq_node_list[0].asic_freq)
    {
        asic_node = 0;
    }
    else
    {
        for(i = 1; i < m_freq_control.ctl_freq_node_max; i++)
        {
            if (Freq <= m_freq_node_list[i].asic_freq)
            {
                asic_node = i;
                break;
            }
        }
    }

    return asic_node;
}


/**
 * @brief If multi-driver request freqency,this function will get the optimum frequency.
 * @author luheshan
 * @date 2012-06-08
 * @param[in] T_VOID
 * @return T_pFREQ_NODE
 * @retval values of the optimum frequency's node
 */
static T_pFREQ_NODE FreqMgr_GetDriverOptimumFreq(T_VOID)
{
    T_U32 i; 
    T_U32 sum = 0;
    T_U32 asic_node = 0;

    //get all driver request frequency summation.
    for (i = 0; i < m_freq_control.ctl_freq_node_max; i++)
    {
        if (0 != m_freq_node_list[i].count)
        {
            sum += (m_freq_node_list[i].asic_freq * m_freq_node_list[i].count);
        }
    }

    //no need add to the system min frequency
    sum -= m_freq_node_list[0].asic_freq;
        
    asic_node = FreqMgr_GetAsicListNode(sum);
    
    m_freq_control.ctl_drv_optimum_freq = m_freq_node_list[asic_node].asic_freq;
        
    akprintf(C3, M_DRVSYS, "FreqMgr:driver_optimum_freq:%d\n" , m_freq_control.ctl_drv_optimum_freq);
    
    return &m_freq_node_list[asic_node];
}

/**
 * @brief frequency manager mode to initial.
 *        1.will init asic frequency and driver list, 
 *        2.start A timer to check cpu usage factor
 *        3.set the pll and defaule asic,cpu 2X.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] cpu_max_freq: input the cpu max value,37xx chip the max is 280000000 (hz) 
 * @param[in] asic_max_freq: input the asic min value,(hz)
 * @param[in] FreqMgr_Callback: get cpu usage factor call back function.
 * @return T_BOOL
 * @retval values of  succeeds, the return value is AK_TRUE.
 */
T_BOOL FreqMgr_Init(T_U32 cpu_max_freq, T_U32 asic_min_freq, FreqMgr_Callback CpuCb)
{
    T_U32 i;
    T_pFREQ_CONTROL control = &m_freq_control;
    T_U32 *pAsic_list = AK_NULL;
    T_U32 pll_freq;

    if (control->ctl_init)
    {
        return AK_TRUE;
    }

    if (AK_NULL == CpuCb)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,call back function is null\n");
        return AK_FALSE;
    }

    if (!set_pll_value(cpu_max_freq/1000000))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,set pll is fail\n");
        return AK_FALSE;
    }
    
    //init frequency node list,min to max
    pll_freq = get_pll_value() * 1000000;
    if (!get_asic_node(pll_freq, asic_min_freq, AK_NULL, &(control->ctl_freq_node_max)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,get asic node list is fail 1\n");
        return AK_FALSE;
    }
    
    pAsic_list = (T_U32 *)drv_malloc(sizeof(T_U32) * control->ctl_freq_node_max);
    if (AK_NULL == pAsic_list)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,malloc is fail 1\n");
        return AK_FALSE;
    }

    if (!get_asic_node(pll_freq, asic_min_freq, &pAsic_list, &(control->ctl_freq_node_max)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,get asic node list is fail 2\n");
        return AK_FALSE;
    }
    
    m_freq_node_list = (T_pFREQ_NODE)drv_malloc(sizeof(T_FREQ_NODE) * control->ctl_freq_node_max);
    if (AK_NULL == m_freq_node_list)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,malloc is fail 2\n");
        drv_free(pAsic_list);
        return AK_FALSE;
    }
    
    for (i = 0; i < control->ctl_freq_node_max; i++)
    {
        m_freq_node_list[i].asic_freq = pAsic_list[i];
        m_freq_node_list[i].count = 0;
    }
    m_freq_node_list[0].count = 1;

    drv_free(pAsic_list);

    //set the defaule asic,cpu 2X
    if ((!set_asic_freq(m_freq_node_list[control->ctl_freq_node_max - 1].asic_freq)) \
            || (!set_cpu_2x_asic(AK_TRUE)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init freq fail\n");
        drv_free(m_freq_node_list);
        m_freq_node_list = AK_NULL;
        return AK_FALSE;
    }

    //start check timer
    control->ctl_timer = vtimer_start(FREQ_CHECK_CPU_UASGE_TIME, AK_TRUE, FreqMgr_CheckCpuUsageTimerCb);
    if (ERROR_TIMER == control->ctl_timer)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:timer start fail\n");
        drv_free(m_freq_node_list);
        m_freq_node_list = AK_NULL;
        return AK_FALSE;
    }
    
    akprintf(C3, M_DRVSYS, "FreqMgr:init success,pll:%d,cpu:%d,asic:%d,max node num:%d\n",\
             get_pll_value(), get_cpu_freq(), get_asic_freq(), control->ctl_freq_node_max);

    //init driver list
    for (i = 0; i < FREQ_MAX_DRV_NUM; i++)
    {
        m_drv_list[i] = FREQ_INVALID_HANDLE;
    }
    
    //init conrtol variable
    control->ctl_current = &m_freq_node_list[control->ctl_freq_node_max - 1];
    control->ctl_lock = FREQ_UNLOCK;
    control->ctl_delay_down_time = 0;
    control->ctl_add_drv_cnt = 0;
    control->ctl_cpu_cb = CpuCb;
    control->ctl_drv_optimum_freq = m_freq_node_list[0].asic_freq;
    control->ctl_init = AK_TRUE;
    
    return AK_TRUE;
}


/**
 * @brief frequency manager mode to request asic min frequency for the specifically driver.
 *        1.if a driver is need a specifically asic frequency for runing, this function will be call
 *        2.if call this function,must be call FreqMgr_CancelFreq function to cancel.
 *        3.when the driver is runing and can't to change frequency, we will call FreqMgr_Lock to lock.
 *    notes: if request asic > current asic,and is lock,will return fail.
 *         The DrvA request the least frequency frist,and then,DrvB request.them need runing the some time.
 *         If the DrvA must be locked,the DrvB request frequency success,but will no to adjust asic frequency.
 *         If DrvA'least frequency less then DrvB,the system may be make a mistake.
 *         so ,the driver request frequency and need lock,the max value asic must be request frist.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] ReqFreq: input the driver specifically min frequency for runing.
 * @return T_hFreq:request frequency handle.
 * @retval values of  fail, the return value is FREQ_INVALID_HANDLE.
 */
T_hFreq FreqMgr_RequestFreq(T_U32 ReqFreq)
{
    T_U32 i;
    T_U32 asic_node = 0;
    T_hFreq freq_handle = FREQ_INVALID_HANDLE;
    T_pFREQ_CONTROL control = &m_freq_control;
    T_pFREQ_NODE optimum_freq_node;

    if (!control->ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return FREQ_INVALID_HANDLE;
    }
    
    FREQ_MGR_PROTECT; //protect
        
    //if Lock and request freq > current freq,return fail
    if (control->ctl_lock != FREQ_UNLOCK)
    {
        if (ReqFreq > control->ctl_current->asic_freq)
        {
            akprintf(C1, M_DRVSYS, "FreqMgr:freq is lock and request > current,return fail\n");
            FREQ_MGR_UNPROTECT; //unprotect
            return FREQ_INVALID_HANDLE;
        }
    }
    
    //get the frequency node for drvier request freq
    asic_node = FreqMgr_GetAsicListNode(ReqFreq);
    
    //add to driver list,will add the new driver to the list next one which is the last time add
    for (i = 0; i < FREQ_MAX_DRV_NUM; i++)
    {
        if (FREQ_MAX_DRV_NUM <= control->ctl_add_drv_cnt)
        {
            control->ctl_add_drv_cnt = 0;
        }
                    
        if (FREQ_INVALID_HANDLE == m_drv_list[control->ctl_add_drv_cnt])
        {
            m_freq_node_list[asic_node].count++;
            m_drv_list[control->ctl_add_drv_cnt] = asic_node;
            freq_handle = control->ctl_add_drv_cnt;
            control->ctl_add_drv_cnt++;
            break;
        }
        
        control->ctl_add_drv_cnt++;
    }

    if (i == FREQ_MAX_DRV_NUM)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:add drv to list is the max.:%d\n", FREQ_MAX_DRV_NUM);
        FREQ_MGR_UNPROTECT; //unprotect
        return FREQ_INVALID_HANDLE;
    }

    //get the optimum freq node
    optimum_freq_node = FreqMgr_GetDriverOptimumFreq();
    
    if (optimum_freq_node->asic_freq > control->ctl_current->asic_freq)
    {
        FreqMgr_AdjustFreq(optimum_freq_node);
    }

    FREQ_MGR_UNPROTECT; //unprotect
    //return handle
    return freq_handle;
}


/**
 * @brief frequency manager mode to cancle asic min frequency what had requested for the specifically driver.
 *        1.A driver had requested specifically asic frequency this function will be call by cancle request.
 *        2.If the driver had locked, when cancel,we will call FreqMgr_Lock to UnLock.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] hFreq: input a handle what request freq return the handle.
 * @return T_BOOL
 * @retval values of  succeeds, the return value is AK_TRUE.
 */
T_BOOL FreqMgr_CancelFreq(T_hFreq hFreq)
{
    T_S32 freq_node;

    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return AK_FALSE;
    }
        
    if ((FREQ_INVALID_HANDLE >= hFreq) ||(FREQ_MAX_DRV_NUM <= hFreq))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:handle is invalid,may be has canceled 1!\n");
        return AK_FALSE;
    }
    
    FREQ_MGR_PROTECT; //protect
    
    //get frequency list node
    freq_node = m_drv_list[hFreq]; 

    if (FREQ_INVALID_HANDLE == freq_node)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:handle is invalid,may be has canceled 2!\n");
        FREQ_MGR_UNPROTECT; //unprotect
        return AK_FALSE;
    }
    
    //clean freq list drv request notes
    m_freq_node_list[freq_node].count--; 
    m_drv_list[hFreq] = FREQ_INVALID_HANDLE;

    //this function will change driver optimum freq.
    FreqMgr_GetDriverOptimumFreq();

    FREQ_MGR_UNPROTECT; //unprotect
    
    return AK_TRUE;
}


/**
 * @brief frequency manager mode to lock asic frequency not to be changed.
 *        1.if a driver when is runing and can't to change asic frequency, this function will be call
 *        2.if call this function,must be keep lock and unlock one by one.
 * @author luheshan
 * @date 2012-05-18
 * @paramT_VOID:
 * @return T_VOID:
 * @retval values of  T_VOID.
 */
T_VOID FreqMgr_Lock(T_VOID)
{
    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return;
    }
    
    FREQ_MGR_PROTECT; //protect
    if (m_freq_control.ctl_lock != ~0)
    {
        m_freq_control.ctl_lock++;
    }
    FREQ_MGR_UNPROTECT; //unprotect
}

/**
 * @brief frequency manager mode to UnLock asic frequency.
 * @author luheshan
 * @date 2012-05-18
 * @paramT_VOID:
 * @return T_VOID:
 * @retval values of  T_VOID.
 */
T_VOID FreqMgr_UnLock(T_VOID)
{
    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return;
    }
    
    FREQ_MGR_PROTECT; //protect
    
    if (FREQ_UNLOCK == m_freq_control.ctl_lock)
    {
        akprintf(C3, M_DRVSYS, "FreqMgr: warning: has been reUnLocked!!!\n");
    }
    else
    {
        m_freq_control.ctl_lock--;
    }
    
    FREQ_MGR_UNPROTECT; //unprotect
    
}

/**
 * @brief frequency manager mode to get lock status
 * @author luheshan
 * @date 2012-05-22
 * @paramT_VOID:
 * @return T_BOOL:
 * @retval values of  TRUE is lock.
 */
T_BOOL FreqMgr_IsLock(T_VOID)
{
    T_BOOL ret = AK_FALSE;

    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return ret;
    }
    
    FREQ_MGR_PROTECT; //protect
    if (FREQ_UNLOCK != m_freq_control.ctl_lock)
    {
        ret = AK_TRUE;
    }
    FREQ_MGR_UNPROTECT; //unprotect

    return ret;
}

#endif

