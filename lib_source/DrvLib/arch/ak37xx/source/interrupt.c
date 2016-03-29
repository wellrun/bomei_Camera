/**
 * @file interrupt.c
 * @brief interrupt function file
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#include "interrupt.h" 
#include "drv_api.h"

#define MAX_STACK_DEPTH         (10)

#define MAX_REGISTER_INTR_NUM   (64)
#define MAX_INTR_HANDLER_NUM    (8)
#define INTR_NBITS              (3)

static volatile T_U32 gb_interrupt_val[MAX_STACK_DEPTH] = {0};
static volatile T_U32 gb_fastinterrupt_val[MAX_STACK_DEPTH] = {0};
static volatile T_U32   ucStackTop = 0;
static volatile T_U32   ucStackTop_fiq = 0;

static volatile T_U8 registered_irqs[MAX_REGISTER_INTR_NUM]={0xff};
static volatile T_U8 registered_fiqs[MAX_REGISTER_INTR_NUM]={0xff};

typedef struct _tagint
{
    T_INTR_HANDLER ptr;
    struct _tagint *next;
    INT_VECTOR     v;
}T_INTR_NODE;

static volatile T_INTR_NODE intr_pointers[INTR_NBITS*MAX_INTR_HANDLER_NUM]={0};

/**
* @brief store the irq mask to global variable and disable all irq interrupt
* @author liao_zhijun
* @date 2010-06-18
* @return T_VOID
*/
T_VOID store_all_int(T_VOID)
{
    T_U32 mask_reg;

    if (ucStackTop >= MAX_STACK_DEPTH)
    {
        akprintf(C3, M_DRVSYS, "-->store_all_int(): interrupt stack overfollow, ucStackTop=%d\r\n", ucStackTop);
        while(1);
    }

    irq_mask();
    
    mask_reg = REG32(IRQINT_MASK_REG);
    REG32(IRQINT_MASK_REG) = 0x0;

    gb_interrupt_val[ucStackTop] = mask_reg;

    ucStackTop++;

    irq_unmask();
}

/**
* @brief recover irq interrupt from last store_all_int call
* @author liao_zhijun
* @date 2010-06-18
* @return T_VOID
*/
T_VOID restore_all_int(T_VOID)
{
    if (ucStackTop == 0)
    {
        akprintf(C3, M_DRVSYS, "-->restore_all_int(): Reach interrupt stack bottom\r\n");
        while(1);
    }

    irq_mask();

    ucStackTop--;
    REG32(IRQINT_MASK_REG) = gb_interrupt_val[ucStackTop];

    irq_unmask();
}

/* 
map to real vector by different chips 
the vector is the status bits location
*/
static T_U8 map2real_vector(INT_VECTOR v)
{
    const T_U8 vector_table[INT_VECTOR_MAX] = {
        1/*LCD*/, 2/*camera*/,3/*motion*/,4/*jpeg*/,
        8/*dac*/, 9/*adc*/, 10/*l2*/,
        14/*UART3*/, 15/*UART2*/, 16/*UART1*/,
        17/*spi*/, 18/*mac*/,19/*irda*/, 
        21/*mci2*/, 22/*mci2*/, 
        25/*usbotg*/, 26/*usbdma*/,
        27/*gpio*/,27/*timer*/,27/*rtc*/,27/*wgpio*/
        };
    return vector_table[v-(INT_VECTOR_MIN+1)];
}

/* map to mask bit by different chips */
static T_U32 map2mask_bit(INT_VECTOR v)
{

    const T_U32 mask_table[INT_VECTOR_MAX] = {
        IRQ_MASK_LCD_BIT/*LCD*/, IRQ_MASK_CAMERA_BIT/*camera*/, 
        IRQ_MASK_MOTION/*motion*/, IRQ_MASK_JPEG/*jpeg*/,
        IRQ_MASK_DAC_BIT/*dac*/, IRQ_MASK_SIGDELTA_ADC_BIT/*adc*/,
        IRQ_MASK_L2_BIT/*l2*/,IRQ_MASK_UART3_BIT/*UART3*/,
        IRQ_MASK_UART2_BIT/*UART2*/, IRQ_MASK_UART1_BIT/*UART1*/,
        IRQ_MASK_SPI_BIT/*spi*/, IRQ_MASK_MAC_BIT/*mac*/,
        IRQ_MASK_IRDA_BIT/*irda*/, 
        IRQ_MASK_MCI2_BIT/*mci2*/, IRQ_MASK_MCI1_BIT/*mci1*/,
        IRQ_MASK_USB_BIT/*usbotg*/, IRQ_MASK_USBDMA_BIT/*usbdma*/,
        IRQ_MASK_SYS_MODULE_BIT/*gpio*/, IRQ_MASK_SYS_MODULE_BIT/*timer*/,
        IRQ_MASK_SYS_MODULE_BIT/*rtc*/, IRQ_MASK_SYS_MODULE_BIT/*wgpio*/
        };

    return mask_table[v-(INT_VECTOR_MIN+1)];
}

static T_INTR_NODE* check_node(T_INTR_NODE *node, INT_VECTOR v)
{
    T_INTR_NODE *p;

    p = node;
    while(p)
    {
        if (p->v == v)
            break;
        p = p->next;
    }
    return p;
}

static T_INTR_NODE* alloc_node(T_INTR_NODE *head)
{
    T_U8 index;
    T_INTR_NODE *p=AK_NULL, *node; 

    /* find an empty place first */
    for (index=MAX_INTR_HANDLER_NUM; index<MAX_INTR_HANDLER_NUM*INTR_NBITS; index++)
    {
        if (!intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE *)&intr_pointers[index];
            break;
        }
    }
    /* no more empty place */
    if (!p)
        return AK_NULL;
    
    if (head)
    {
        /* there is a head, link the empty node to it */
        node = head;
        while(node->next)
        {
            node = node->next;
        };
        node->next = p;     
    }
    return p;
}

/**
* @brief interrupt init, called before int_register_irq()
* @author liao_zhijun
* @date 2010-06-18
* @return T_VOID
*/
T_VOID interrupt_init(T_VOID)
{
    T_U32 i;
    for (i=0; i<MAX_REGISTER_INTR_NUM; i++)
    {
        registered_irqs[i] = 0xff;
        registered_fiqs[i] = 0xff;
    }
}

/**
* @brief register irq interrupt
* @author liao_zhijun
* @data 2010-06-18
* @return T_BOOL
*/
T_BOOL int_register_fiq(INT_VECTOR v, T_INTR_HANDLER handler)
{
    T_U8    index, vector;
    T_U32   mask;
    T_INTR_NODE *p;

    /* check vector valid */
    if (v >= INT_VECTOR_MAX)
    {
        akprintf(C3, M_DRVSYS, "invalid vector %d\n", v);
        return AK_FALSE;
    }
    
    vector = map2real_vector(v);

    if (handler)
    {
        /* register it now */
        if (registered_fiqs[vector] != 0xff)
        {
            index = registered_fiqs[vector];
            /* check it has registered or not */
            p = check_node((T_INTR_NODE*)&intr_pointers[index], v);
            if (p)
            {
                /* it has registered, just replace the handler*/
                p->ptr = handler;
            }
            else
            {
                /* alloc a node and add it to the tail of list */               
                p = alloc_node((T_INTR_NODE*)&intr_pointers[index]);
                if (p)
                {
                    p->ptr  = handler;
                    p->next = AK_NULL;
                    p->v = v;
                    
                }
                else
                {
                    akprintf(C3, M_DRVSYS, "no more place for registeration, exit!!\n");
                    return AK_FALSE;
                }
            }
        }
        else
        {
            /* find an empty place for it */
            index = 0;
            for (index=0; index<MAX_INTR_HANDLER_NUM; index++)
            {
                if (!intr_pointers[index].ptr)
                    break;
            }
            if (index < MAX_INTR_HANDLER_NUM)
            {
                intr_pointers[index].ptr = handler;
                intr_pointers[index].next= AK_NULL;
                intr_pointers[index].v = v;
                registered_fiqs[vector] = index;
            }
            else
            {
                akprintf(C3, M_DRVSYS, "no more place for register, exit1!!\n");
                return AK_FALSE;
            }
        }
        /* enable interrupt */
        FIQ_INTR_ENABLE(map2mask_bit(v));
    }
    else
    {
        /* unregister it */
        
    }
    return AK_TRUE;
}


/**
* @brief register irq interrupt
* @author liao_zhijun
* @data 2010-06-18
* @return T_BOOL
*/
T_BOOL int_register_irq(INT_VECTOR v, T_INTR_HANDLER handler)
{
    T_U8    index, vector;
    T_U32   mask;
    T_INTR_NODE *p;

    /* check vector valid */
    if (v >= INT_VECTOR_MAX)
    {
        akprintf(C3, M_DRVSYS, "invalid vector %d\n", v);
        return AK_FALSE;
    }
    
    vector = map2real_vector(v);

    if (handler)
    {
        /* check its fiq has registered or not */
        if (registered_fiqs[vector] != 0xff)
        {
            akprintf(C3, M_DRVSYS, "its fiq has regisetered, exit!!\n");
            return AK_FALSE;
        }

        /* register it now */
        if (registered_irqs[vector] != 0xff)
        {
            index = registered_irqs[vector];
            /* check it has registered or not */
            p = check_node((T_INTR_NODE*)&intr_pointers[index], v);
            if (p)
            {
                /* it has registered, just replace the handler*/
                p->ptr = handler;
            }
            else
            {
                /* alloc a node and add it to the tail of list */               
                p = alloc_node((T_INTR_NODE*)&intr_pointers[index]);
                if (p)
                {
                    p->ptr  = handler;
                    p->next = AK_NULL;
                    p->v = v;
                    
                }
                else
                {
                    akprintf(C3, M_DRVSYS, "no more place for registeration, exit!!\n");
                    return AK_FALSE;
                }
            }
        }
        else
        {
            /* find an empty place for it */
            index = 0;
            for (index=0; index<MAX_INTR_HANDLER_NUM; index++)
            {
                if (!intr_pointers[index].ptr)
                    break;
            }
            if (index < MAX_INTR_HANDLER_NUM)
            {
                intr_pointers[index].ptr = handler;
                intr_pointers[index].next= AK_NULL;
                intr_pointers[index].v = v;
                registered_irqs[vector] = index;
            }
            else
            {
                akprintf(C3, M_DRVSYS, "no more place for register, exit1!!\n");
                return AK_FALSE;
            }
        }
        /* enable interrupt */
        INTR_ENABLE(map2mask_bit(v));
    }
    else
    {
        /* unregister it */
        
    }
    return AK_TRUE;
}

T_VOID irq_dispatch_handler(T_U8 irq)
{
    T_U8 index;
    T_INTR_NODE *p;

//  akprintf(C3, M_DRVSYS, "irq number %d\n", irq);
    if (irq >= MAX_REGISTER_INTR_NUM)
    {
        akprintf(C3, M_DRVSYS, "irq number %d is invalid!!\n", irq);
        return;
    }
    
    index = registered_irqs[irq];
    if (index < MAX_INTR_HANDLER_NUM)
    {
        if (intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE*)&intr_pointers[index];
            do
            {
                /* if handled it, exit */
                if ((*(p->ptr))() == AK_TRUE)
                    return;
                p = p->next;
            }while(p);
            /* WRONG: irq not be handled!! */
            if (irq != 27)  /* 27 is gpio */
                akprintf(C3, M_DRVSYS, "irq %d not handled!!\n", irq);
        }
        else
            akprintf(C3, M_DRVSYS, "unregistered irq %d\n", irq);
    }
    else if (irq < INT_STATUS_NBITS)
    {
        akprintf(C3, M_DRVSYS, "irq %d %x not register and it comes!!\n", irq, REG32(IRQINT_MASK_REG));
        while(1);
    }
}

T_VOID fiq_dispatch_handler(T_U8 fiq)
{
    T_U8 index;
    T_INTR_NODE *p;

    //akprintf(C3, M_DRVSYS, "fiq number %d\n", fiq);
    if (fiq >= MAX_REGISTER_INTR_NUM)
    {
        akprintf(C3, M_DRVSYS, "fiq number %d is invalid!!\n", fiq);
        return;
    }
    
    index = registered_fiqs[fiq];
    if (index < MAX_INTR_HANDLER_NUM)
    {
        if (intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE*)&intr_pointers[index];
            do
            {
                /* if handled it, exit */
                if ((*(p->ptr))() == AK_TRUE)
                    return;
                p = p->next;
            }while(p);
            /* WRONG: fiq not be handled!! */
            akprintf(C3, M_DRVSYS, "fiq %d not handled!!\n", fiq);
            
        }
        else
            akprintf(C3, M_DRVSYS, "unregistered fiq %d\n", fiq);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "fiq not register and it comes!!\n");
        while(1);
    }
}

