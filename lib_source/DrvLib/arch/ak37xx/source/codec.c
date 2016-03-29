/**
 * @FILENAME: codec.c
 * @BRIEF jpeg codec module
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liao_zhijun
 * @DATE 2007.10.23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "drv_api.h"
#include "drv_module.h"
#include "interrupt.h"

#define EVENT_CODEC_FINISH 1

static T_BOOL codec_intr_handler()
{
    //mask interrupt
    INTR_DISABLE(IRQ_MASK_JPEG);

    //set event
    DrvModule_SetEvent(DRV_MODULE_JPEG_CODEC, EVENT_CODEC_FINISH);
    
    return AK_TRUE;
}

/**
* @brief wait jpeg codec finish
* @author liao_zhijun
* @date 2010-10-17
* @return T_BOOL
*/
T_BOOL codec_wait_finish()
{
    T_S32 ret;
    
    //unmask interrupt
    INTR_ENABLE(IRQ_MASK_JPEG);

    //wait event
    ret = DrvModule_WaitEvent(DRV_MODULE_JPEG_CODEC, EVENT_CODEC_FINISH, 10000);

    if(DRV_MODULE_SUCCESS == ret)
        return AK_TRUE;
    else
        return AK_FALSE;
}

/**
* @brief enable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return T_BOOL
*/
T_BOOL codec_intr_enable()
{
    //creat task 
    if(!DrvModule_Create_Task(DRV_MODULE_JPEG_CODEC))
    {
        akprintf(C1, M_DRVSYS, "creat LCD tast failed\n");
        return AK_FALSE;
    }

    //register jpeg codec interrupt
    int_register_irq(INT_VECTOR_JPEG, codec_intr_handler);

    //mask interrupt
    INTR_DISABLE(IRQ_MASK_JPEG);   
    
    akprintf(C3, M_DRVSYS, "codec_intr_enable()\n");
    
    return AK_TRUE;
}

/**
* @brief disable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return T_VOID
*/
T_VOID codec_intr_disable()
{
    //mask interrupt
    INTR_DISABLE(IRQ_MASK_JPEG);   
    
    //destroy task
    DrvModule_Terminate_Task(DRV_MODULE_JPEG_CODEC);

    akprintf(C3, M_DRVSYS, "codec_intr_disable()\n");
}

