/**
 * @filename usb_slave_drv.c
 * @brief: frameworks of usb driver.
 *
 * This file describe udriver of usb in slave mode.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-26
 * @version 1.0
 * @ref
 */
#ifdef OS_ANYKA

#include    "anyka_cpu.h"
#include    "usb_slave_drv.h"
#include    "sysctl.h"
#include    "interrupt.h"
#include    "drv_api.h"
#include    "usb_common.h"
#include    "hal_usb_s_state.h"
#include    "hal_usb_s_std.h"
#include    "hal_usb_std.h"
#include    "l2.h"
#include    "usb.h"

#define MAX_EP_NUM       4
#define DMA_PER_BUF_SIZE (128*1024)

typedef enum
{
    EP_RX_FINISH = 0,
    EP_RX_RECEIVING,
    EP_TX_FINISH,
    EP_TX_SENDING
}USB_EP_STATE;


typedef struct
{
    T_U32 EP_TX_Count;
    USB_EP_STATE EP_TX_State;
    T_fUSB_TX_FINISH_CALLBACK TX_Finish; 
    T_U8  *EP_TX_Buffer;
    T_U8  L2_Buf_ID;
}USB_EP_TX;

typedef struct
{
    T_U32 EP_RX_Count;
    USB_EP_STATE EP_RX_State;
    T_fUSB_NOTIFY_RX_CALLBACK RX_Notify;
    T_fUSB_RX_FINISH_CALLBACK RX_Finish;
    T_U8  *EP_RX_Buffer;
    T_U8  L2_Buf_ID;
    T_BOOL bDmaStart;
}USB_EP_RX;

typedef union
{
    USB_EP_RX rx;
    USB_EP_TX tx;
}USB_EP;

typedef struct
{
    T_U32 tx_count;
    T_CONTROL_TRANS ctrl_trans;
    T_fUSB_CONTROL_CALLBACK std_req_callback;
    T_fUSB_CONTROL_CALLBACK class_req_callback;
    T_fUSB_CONTROL_CALLBACK vendor_req_callback;
}
T_USB_SLAVE_CTRL;

typedef struct
{
    T_U32 ulInitMode;                           ///<expected mode when init
    T_U32 mode;
    T_U32 state;
    T_U32 usb_max_pack_size;
    T_fUSB_RESET_CALLBACK reset_callback;
    T_fUSB_SUSPEND_CALLBACK suspend_callback;
    T_fUSB_RESUME_CALLBACK resume_callback;
    T_fUSB_CONFIGOK_CALLBACK configok_callback;
    USB_EP ep[6];
    T_BOOL bInit;
    T_BOOL usb_need_zero_packet;
}USB_SLAVE;


static void usb_slave_dma_send_mode0(T_U8 EP_index, T_U32 addr, T_U32 count);

static T_VOID usb_slave_reset(T_VOID);
static T_VOID usb_slave_suspend();
static T_VOID usb_slave_reset_ep(T_U32 EP_index, T_U16 wMaxPacketSize, T_U8 ep_type, T_U8 dma_surport);

static T_U32 usb_slave_get_intr_type(T_U8 *usb_int, T_U16 *usb_ep_int_tx, T_U16 * usb_ep_int_rx);
static T_BOOL usb_slave_intr_handler(T_VOID);
static T_VOID usb_slave_ep0_rx_handler();
static T_VOID usb_slave_ep0_tx_handler();
static T_VOID usb_slave_common_intr_handler(T_U8 usb_int);

static T_VOID usb_slave_tx_handler(EP_INDEX EP_index);
static T_VOID usb_slave_rx_handler(EP_INDEX EP_index);

static T_VOID usb_slave_write_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 value);
static T_VOID usb_slave_read_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 *value);
static T_VOID usb_slave_read_int_reg(T_U8 *value0, T_U16 *value1, T_U16 *value2);

static T_U32 usb_slave_receive_data(EP_INDEX EP_index);
static T_U32 usb_slave_send_data(EP_INDEX EP_index);
static T_U32 usb_slave_dma_start(EP_INDEX EP_index);

static T_U32 usb_slave_ctrl_in(T_U8 *data, T_U32 len);
static T_U32 usb_slave_ctrl_out(T_U8 *data);
static T_BOOL usb_slave_ctrl_callback(T_U8 req_type);

volatile USB_SLAVE usb_slave;
volatile T_USB_SLAVE_CTRL usb_ctrl;

static volatile T_U16 m_ep_status[MAX_EP_NUM]={0};

/**
 * @brief   initialize usb slave global variables, and set buffer for control tranfer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param buffer [IN] buffer to be set for control transfer
 * @param buf_len [IN] buffer length
 * @return  T_BOOL
 * @retval AK_TRUE init successfully
 * @retval AK_FALSE init fail
 */
T_BOOL usb_slave_init(T_U8 *buffer, T_U32 buf_len)
{
    //check param
    if(AK_NULL == buffer || buf_len == 0)
    {
        return AK_FALSE;
    }

    //init global variables
    memset((T_pVOID)&usb_slave, 0, sizeof(usb_slave));
    usb_slave.bInit = AK_TRUE;

    usb_slave.usb_max_pack_size = 64;

    //alloc buffer
    usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID = l2_alloc(ADDR_USB_EP2);
    usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID = l2_alloc(ADDR_USB_EP1);

    if(BUF_NULL == usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID || BUF_NULL == usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID)
    {
        akprintf(C2, M_DRVSYS, "malloc L2 buffer id error\n");
        return AK_FALSE;
    }

    akprintf(C2, M_DRVSYS, "rx buf id %x\n", usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID);
    akprintf(C2, M_DRVSYS, "tx buf id %x\n", usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID);

    usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID = usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID;
    usb_ctrl.ctrl_trans.buffer = buffer;
    usb_ctrl.ctrl_trans.buf_len = buf_len;    

    return AK_TRUE;
    
}
T_VOID usb_slave_free(T_VOID)
{
    memset((T_pVOID)&usb_slave, 0, sizeof(usb_slave));
    //free l2 buffer
    l2_free(ADDR_USB_EP1);
    l2_free(ADDR_USB_EP2);
    
    usb_ctrl.ctrl_trans.buffer = AK_NULL;
    usb_ctrl.ctrl_trans.buf_len= 0;
}

/**
 * @brief   enable usb interrupt, used in usb download
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_BOOL
 * @retval AK_TRUE init successfully
 * @retval AK_FALSE init fail
 */
T_BOOL usb_boot_init()
{
    //enble irq for usb
    int_register_irq(INT_VECTOR_USB, usb_slave_intr_handler);

    return AK_TRUE;
}


/**
 * @brief   set control transfer call back function
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param type [IN] request type, must be one of (REQUEST_STANDARD, REQUEST_CLASS, REQUEST_VENDOR)
 * @param callback [In] callback function
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_ctrl_callback(T_U8 type, T_fUSB_CONTROL_CALLBACK callback)
{
    //standard request
    if(REQUEST_STANDARD == type)
        usb_ctrl.std_req_callback = callback;

    //class request
    if(REQUEST_CLASS == type)
        usb_ctrl.class_req_callback = callback;

    //vendor request
    if(REQUEST_VENDOR == type)
        usb_ctrl.vendor_req_callback = callback;

    return AK_TRUE;
}

/**
 * @brief   set usb event(reset, suspend, resume, configok) callback.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param reset_callback [IN] callback function for reset interrupt
 * @param suspend_callback [IN] callback function for suspend interrupt
 * @param resume_callback [IN] callback function for resume interrupt
 * @param configok_callback [IN] callback function for config ok event
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_callback(T_fUSB_RESET_CALLBACK reset_callback, T_fUSB_SUSPEND_CALLBACK suspend_callback, T_fUSB_RESUME_CALLBACK resume_callback, T_fUSB_CONFIGOK_CALLBACK configok_callback)
{
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.reset_callback = reset_callback;
    usb_slave.suspend_callback = suspend_callback;
    usb_slave.resume_callback = resume_callback;
    usb_slave.configok_callback = configok_callback;

    return AK_TRUE;
}

/**
 * @brief   Register a callback function to notify tx send data finish.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0??
 * @param  callback_func [in]  T_fUSB_TX_FINISH_CALLBACK can be null
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_tx_callback(EP_INDEX EP_index, T_fUSB_TX_FINISH_CALLBACK callback_func)
{
    T_U32 tx_index = EP_index;
    
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.ep[tx_index].tx.TX_Finish = callback_func;
    
    return AK_TRUE;
}

/**
 * @brief   Register a callback function to notify rx receive data finish and rx have data.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  notify_rx [in] rx notify callbakc function, can be null
 * @param  rx_finish [in] rx finish callbakc function, can be null
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_rx_callback(EP_INDEX EP_index, T_fUSB_NOTIFY_RX_CALLBACK notify_rx, T_fUSB_RX_FINISH_CALLBACK rx_finish)
{
    T_U32 rx_index = EP_index;
    
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.ep[rx_index].rx.RX_Notify = notify_rx;
    usb_slave.ep[rx_index].rx.RX_Finish = rx_finish;

    return AK_TRUE;
}


//********************************************************************
/**
 * @brief   enable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] T_U32 usb mode
 * @return  T_VOID
 */
T_VOID usb_slave_device_enable(T_U32 mode)
{
    T_U32 regvalue;

    //check init
    if(!usb_slave.bInit)
    {
        akprintf(C3, M_DRVSYS, "usb slave isn't init\n");
        return;
    }
    
    //enable clock, USB PLL, USB 2.0
    sysctl_clock(CLOCK_USB_ENABLE);

    //Enable the usb transceiver and suspend enable
    regvalue = REG32(USB_CONTROL_REG);
    regvalue &= (~0x7); 
    regvalue |= (0x6); 
    REG32(USB_CONTROL_REG) = regvalue;

    //config usb phy
    regvalue = REG32(USB_NEW_CFG_REG);

    //enable otg
    regvalue |= (0x1<<8); 

#if 0
    //set usb fsdry match_rs to 42, do not ask me why
    regvalue &= (0x7<<9);
    regvalue |= (0x4<<9);

    //set usb ibias select to 4.4mA
    regvalue &= ~(0x3<<12);
    regvalue |= (0x1<<12);

    //set clock mode to low poer
    regvalue &= ~(0x3<<14);
    regvalue |= (0x1<<14);
#endif

    //use USB1
    regvalue &= ~(0x1<<17); 

    REG32(USB_NEW_CFG_REG) = regvalue;

    //ak37xx dosen't have vbus and id pin,so these bit must be set 
    REG32(RTC_USB_CTRL_REG) &= ~(1UL<<31);
    REG32(RTC_USB_CTRL_REG) |= (1<<30);
    REG32(RTC_USB_CTRL_REG) &= ~(1<<29);
    REG32(RTC_USB_CTRL_REG) |= (1<<28);
    REG32(RTC_USB_CTRL_REG) |= (1<<27);
    REG32(RTC_USB_CTRL_REG) |= (1<<26);

    //set speed mode
    if(USB_MODE_20 == (USB_MODE_20 & mode))
    {
        REG8(USB_REG_POWER) = 0x20;
        usb_slave.ulInitMode = USB_MODE_20;
        usb_slave.usb_max_pack_size = 512;
        usb_slave.mode = USB_MODE_20;   
        akprintf(C2, M_DRVSYS, "high speed!\n");
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;
        usb_slave.ulInitMode = USB_MODE_11;
        usb_slave.usb_max_pack_size = 64;
        usb_slave.mode = USB_MODE_11;   
        akprintf(C2, M_DRVSYS, "full speed!\n");
    }

    //set status
    usb_slave_set_state(USB_CONFIG);

    //enable usb irq
    int_register_irq(INT_VECTOR_USB, usb_slave_intr_handler);

    //enable usb irq
    int_register_irq(INT_VECTOR_USB_DMA, usb_slave_intr_handler);
}

//********************************************************************
/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_slave_device_disable(T_VOID)
{
    //disable irq
    INTR_DISABLE(IRQ_MASK_USB_BIT);

    //close USB clock
    sysctl_clock(~CLOCK_USB_ENABLE);

    //disable transceiver
    REG32(USB_CONTROL_REG) &= (~0x7); 

    //disable  usb phy
    REG32(USB_NEW_CFG_REG) &= ~(1<<8);
    
    //clear power reg
    REG8(USB_REG_POWER) = 0;

    //reset usb controller
    sysctl_reset(RESET_USB_OTG);
}


T_U32 usb_slave_get_mode(T_VOID)
{
    if( (REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE )
    {
        return USB_MODE_20;
    }
    else
    {
        return USB_MODE_11;
    }
}


//********************************************************************
static T_VOID usb_slave_reset(T_VOID)
{
    T_U32 temp;

    //set address to default addr
    usb_slave_set_address(0);

    //check which speed now on after negotiation
    if( (REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE )
    {
        REG8(USB_REG_POWER) = 0x20;
        
        usb_slave.usb_max_pack_size = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        usb_slave.mode = USB_MODE_20;   
        
        akprintf(C3, M_DRVSYS, "reset to high speed!\n");
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;
        
        usb_slave.usb_max_pack_size = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        usb_slave.mode = USB_MODE_11;
        
        akprintf(C3, M_DRVSYS, "reset to full speed!\n");
    }


    //open all common interrupt except sof
    REG8(USB_REG_INTRUSBE) = 0xFF & (~USB_INTR_SOF);      //disable the sof interrupt

    //config all endpoint
    usb_slave_reset_ep(USB_EP1_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_OUT_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP2_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_IN_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP3_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_OUT_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP4_INDEX, EP_BULK_FULLSPEED_MAX_PAK_SIZE, USB_EP_IN_TYPE, USB_DMA_UNSUPPORT);

    //enable the TX endpoint
    REG8(USB_REG_INTRTX1E) = (USB_EP0_ENABLE | USB_EP2_TX_ENABLE | USB_EP4_TX_ENABLE);

    //enable the RX endpoint
    REG8(USB_REG_INTRRX1E) = (USB_EP1_RX_ENABLE | USB_EP3_RX_ENABLE);

    //clear the interrupt
    temp = REG8(USB_REG_INTRUSB);
    temp = REG8(USB_REG_INTRTX1);
    temp = REG8(USB_REG_INTRTX2);
    temp = REG8(USB_REG_INTRRX1);
    temp = REG8(USB_REG_INTRRX2);

    //select this EP0
    REG8( USB_REG_INDEX) = USB_EP0_INDEX;

    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
}
//********************************************************************
static T_VOID usb_slave_suspend(T_VOID)
{
    //device is at full speed when in suspend ,so reinit high speed
    if (USB_MODE_20 == usb_slave.ulInitMode)
    {
        REG8(USB_REG_POWER) = 0x20;
    }
}

//********************************************************************
static T_VOID usb_slave_reset_ep(T_U32 EP_index, T_U16 wMaxPacketSize, T_U8 ep_type, T_U8 dma_surport)
{
    T_U32 fifo_size;
    T_U8 tmp;

    //select the ep
    REG8(USB_REG_INDEX) = EP_index;             /* select this EP */

    //select ep type and max packet size
    if( ep_type == USB_EP_IN_TYPE )
    {
        REG8(USB_REG_TXCSR2) = USB_TXCSR2_MODE;
        REG16(USB_REG_TXMAXP1) = wMaxPacketSize;
    }
    else if ( ep_type == USB_EP_OUT_TYPE )
    {
        REG8(USB_REG_TXCSR2) = 0;
        REG16(USB_REG_RXMAXP1) = wMaxPacketSize;
        REG8(USB_REG_RXCSR1) &=  (~USB_RXCSR1_RXPKTRDY);
    }

}

static T_U32 usb_slave_receive_data(EP_INDEX EP_index)
{
    T_U16 ret, i;
    T_U32 rx_index = EP_index;

    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    for(i = 0; i < ret; i++)
    {
        usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP0 + (EP_index << 2));
    }
        
    //change global variable status
    if(usb_slave.ep[rx_index].rx.EP_RX_Count > ret && usb_slave.ep[rx_index].rx.EP_RX_Count > usb_slave.usb_max_pack_size)
        usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    else
    {
        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;
    }

    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;

    //clear RXPKTRDY
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}

static T_U32 usb_slave_dma_start(EP_INDEX EP_index)
{
    T_U32 ret;
    T_U32 tx_index = EP_index;
    
    //set autoset/DMAReqEnable/DMAReqMode
    REG8(USB_REG_INDEX) = EP_index;
    REG8(USB_REG_TXCSR2) |= (USB_TXCSR2_DMAMODE|USB_TXCSR2_DMAENAB|USB_TXCSR2_AUTOSET); 
    
    ret = usb_slave.ep[tx_index].tx.EP_TX_Count;
    ret -= (ret % usb_slave.usb_max_pack_size);
    if (ret > DMA_PER_BUF_SIZE)
        ret = DMA_PER_BUF_SIZE;        
    
    //send data to l2
    l2_clr_status(usb_slave.ep[tx_index].tx.L2_Buf_ID);
    l2_combuf_dma((T_U32)usb_slave.ep[tx_index].tx.EP_TX_Buffer, usb_slave.ep[tx_index].tx.L2_Buf_ID, ret, MEM2BUF, AK_FALSE);
    
    REG32(USB_DMA_ADDR_1) = 0x71000000;
    REG32(USB_DMA_COUNT_1) = ret;
    
    //change global variable value    
    usb_slave.ep[tx_index].tx.EP_TX_Count -= ret;
    usb_slave.ep[tx_index].tx.EP_TX_Buffer += ret;
    
    REG32(USB_DMA_CNTL_1) = (USB_ENABLE_DMA | USB_DIRECTION_TX| USB_DMA_MODE1 | USB_DMA_INT_ENABLE| (EP_index<<4) | USB_DMA_BUS_MODE3);
    return ret;

}

static T_U32 usb_slave_send_data(EP_INDEX EP_index)
{
    T_U32 count, i;
    T_U32 tx_index = EP_index;

    count = usb_slave.ep[tx_index].tx.EP_TX_Count;
    if (0)//(usb_slave.mode == USB_MODE_20) && (usb_slave.ep[tx_index].tx.EP_TX_Count >= usb_slave.usb_max_pack_size) )
    {
        count = usb_slave_dma_start(tx_index);
    }
    else 
    {
        if (count > usb_slave.usb_max_pack_size)
            count = usb_slave.usb_max_pack_size;
        
        for(i = 0; i < count; i++)
        {
            REG8(USB_FIFO_EP0 + (EP_index << 2)) = usb_slave.ep[tx_index].tx.EP_TX_Buffer[i];
        }   
        //change global variable value    
        usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;
        usb_slave.ep[tx_index].tx.EP_TX_Count = usb_slave.ep[tx_index].tx.EP_TX_Count - count;
        usb_slave.ep[tx_index].tx.EP_TX_Buffer += count;    
        //set TXPKTRDY, start sending    
        REG8(USB_REG_INDEX) = EP_index;
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;
        
    }
    
    return count;

}


//********************************************************************
/**
 * @brief   read usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  pBuf [out] usb data buffer.
 * @param  count [in] count to be read
 * @return T_U32 data out count
 */
T_U32 usb_slave_data_out(EP_INDEX EP_index, T_VOID *pBuf, T_U32 count)
{
    T_U32 rx_index = EP_index;
    T_U32 ret = 0, res, i;

    if(EP0_INDEX == EP_index)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_out: error ep number: %d\n", EP_index);
        return 0;
    }

    if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_RECEIVING)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_out: still receiving\n");
        return 0;
    }
    
    usb_slave.ep[rx_index].rx.EP_RX_Buffer = pBuf;
    usb_slave.ep[rx_index].rx.EP_RX_Count = count;
    usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_RECEIVING;
    
    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    //read data from fifo
    for(i = 0; i < ret; i++)
        usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP0 + (EP_index << 2));

    if(usb_slave.ep[rx_index].rx.EP_RX_Count <= ret ||
        usb_slave.ep[rx_index].rx.EP_RX_Count <= usb_slave.usb_max_pack_size)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;

        if(usb_slave.ep[rx_index].rx.RX_Finish != AK_NULL)
            usb_slave.ep[rx_index].rx.RX_Finish(); 

        return ret;
    }

    usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;
    

    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[rx_index].rx.EP_RX_Count > usb_slave.usb_max_pack_size))
    {
        res = usb_slave.ep[rx_index].rx.EP_RX_Count % usb_slave.usb_max_pack_size;
        if (0 != res)
        {
            ret = usb_slave.ep[rx_index].rx.EP_RX_Count - res + usb_slave.usb_max_pack_size;
        }
        else
        {
            ret = usb_slave.ep[rx_index].rx.EP_RX_Count;
        }

        REG8(USB_REG_RXCSR2) |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);

        l2_combuf_dma((T_U32)usb_slave.ep[rx_index].rx.EP_RX_Buffer, usb_slave.ep[rx_index].rx.L2_Buf_ID, ret, BUF2MEM, AK_FALSE);

        REG32(USB_DMA_ADDR_2) = 0x70000000;
        REG32(USB_DMA_COUNT_2) = ret;

        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.bDmaStart = AK_TRUE;

        REG32(USB_DMA_CNTL_2) = (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(EP_index<<4)|USB_DMA_BUS_MODE3);
    }
    
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}

/**
 * @brief   write usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  data [in] usb data buffer.
 * @param  count [in] count to be send.
 * @return  T_U32 data in count
 */
T_U32 usb_slave_data_in(EP_INDEX EP_index, T_U8 *data, T_U32 count)
{
    T_U32 tx_index = EP_index;
    T_U32 ret = 0;

    //check EP_index
    if(EP0_INDEX == EP_index)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_in: error ep number: %d\n", EP_index);
        return 0;        
    }

    //check status
    if(usb_slave.ep[tx_index].tx.EP_TX_State == EP_TX_SENDING)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_in: still sending\n");
        return 0;
    }

    usb_slave.ep[tx_index].tx.EP_TX_Buffer = data;
    usb_slave.ep[tx_index].tx.EP_TX_Count = count;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;

    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[tx_index].tx.EP_TX_Count > usb_slave.usb_max_pack_size))
    {
        ret = usb_slave_dma_start(tx_index);
    }
    else
    {
        ret = usb_slave_send_data(EP_index);
    }
    
    return ret;
}


/**
 * @brief   set usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  state [in] T_U8.
 * @return  T_VOID
 */
T_VOID usb_slave_set_state(T_U8 stage)
{
    usb_slave.state = stage;
    if(usb_slave.state == USB_OK)
    {
        if(usb_slave.configok_callback != AK_NULL)
            usb_slave.configok_callback();
    }
}

/**
 * @brief   get usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_U8
 */
T_U8 usb_slave_getstate(T_VOID)
{
    return usb_slave.state;
}

static T_U32 usb_slave_ctrl_in(T_U8 *data, T_U32 len)
{
    T_U32 i;

    REG8(USB_REG_INDEX) = EP0_INDEX;

    if(0 == len)
    {
        REG32(USB_EP0_TX_COUNT) = len;
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);

        return 0;
    }

    if(len > EP0_MAX_PAK_SIZE)
        len = EP0_MAX_PAK_SIZE;

    //write fifo
    for( i = 0; i < len; i++ )
    {
        REG8( USB_FIFO_EP0 ) = data[i];
    }


    //set TXPKTRDY, if last packet set data end
    if(len < EP0_MAX_PAK_SIZE)
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);
    }
    else
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY);
    }
            
    return len;
}

static T_U32 usb_slave_ctrl_out(T_U8 *data)
{
    T_U32 ret, i;
    T_U8 tmp;

    T_U32 temp_buf[512/4];
    T_U32 temp_count = 0;

    //get rx data count
    REG8(USB_REG_INDEX) = EP0_INDEX;
    ret = REG16(USB_REG_RXCOUNT1);
    
    temp_count = ret;    
    if((temp_count & 0x3) != 0)
        temp_count = (ret + 4) & ~0x3;

    //read data from usb fifo
    for(i = 0; i < ret; i++)
    {
        data[i] = REG8(USB_FIFO_EP0);
    }

    return ret;
}

static T_BOOL usb_slave_ctrl_callback(T_U8 req_type)
{
    T_BOOL ret;
    T_CONTROL_TRANS *pTrans = (T_CONTROL_TRANS *)&usb_ctrl.ctrl_trans;
    
    if((REQUEST_STANDARD == req_type) && usb_ctrl.std_req_callback)
    {
        ret = usb_ctrl.std_req_callback(pTrans);
    }
    else if((REQUEST_CLASS == req_type) && usb_ctrl.class_req_callback)
    {
        ret = usb_ctrl.class_req_callback(pTrans);
    }
    else if((REQUEST_VENDOR == req_type) && usb_ctrl.vendor_req_callback)
    {
        ret = usb_ctrl.vendor_req_callback(pTrans);
    }

    if(!ret)
    {
        usb_slave_ep_stall(EP0_INDEX);
    }

    return ret;
}


//********************************************************************
/**
 * @brief   write usb address.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  address [in]  usb device address.
 * @return  T_VOID
 */
T_VOID usb_slave_set_address(T_U8 address)
{
    REG8(USB_REG_FADDR) = address;
}

//********************************************************************
/**
 * @brief   set ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @param bStall  [in]  stall or not.
 * @return  T_VOID
 */
T_VOID usb_slave_set_ep_status(T_U8 EP_Index, T_BOOL bStall)
{
    if(EP_Index >= MAX_EP_NUM)
        return;

    if(bStall)
        m_ep_status[EP_Index] = 1;
    else
        m_ep_status[EP_Index] = 0;
}

/**
 * @brief   get ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_U16 usb_slave_get_ep_status(T_U8 EP_Index)
{
    if(EP_Index >= MAX_EP_NUM)
        return 0;

    return m_ep_status[EP_Index];
}

//********************************************************************
/**
 * @brief  stall ep
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_VOID usb_slave_ep_stall(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(USB_EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_CSR0_P_SENDSTALL;
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, AK_TRUE);
    }
    else if(EP1_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, AK_TRUE);
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, AK_TRUE);
    }
}

/**
 * @brief  clear stall
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_VOID usb_slave_ep_clr_stall(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &= (~USB_CSR0_P_SENTSTALL);
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &=(~(USB_TXCSR1_P_SENDSTALL|USB_TXCSR1_P_SENTSTALL));
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, AK_FALSE);
    }
    else if(EP1_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) &= (~(USB_RXCSR1_P_SENDSTALL | USB_RXCSR1_P_SENTSTALL));
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, AK_FALSE);
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) &= (~(USB_RXCSR1_P_SENDSTALL | USB_RXCSR1_P_SENTSTALL));
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, AK_FALSE);
    }    
}

T_VOID usb_slave_clr_toggle()
{
    REG8(USB_REG_INDEX) = USB_EP2_INDEX;
    REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;

    REG8(USB_REG_INDEX) = USB_EP1_INDEX;
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;

    REG8(USB_REG_INDEX) = USB_EP3_INDEX;
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
}


//********************************************************************
static T_VOID usb_slave_write_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 value)
{
    REG8(USB_REG_INDEX) = EP_index;
    REG16(reg) = value;
}
//********************************************************************
static T_VOID usb_slave_read_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 *value)
{
    REG8(USB_REG_INDEX) = EP_index;
    *value = REG16(reg);
}
//********************************************************************
static T_VOID usb_slave_read_int_reg(T_U8 *value0, T_U16 *value1, T_U16 *value2)
{
    *value0 = REG8(USB_REG_INTRUSB);
    *value1 = REG16(USB_REG_INTRTX1);
    *value2 = REG16(USB_REG_INTRRX1);
}
//********************************************************************

/**
 * @brief   read data count of usb end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index [in] usb end point.
 * @param cnt  [out] cnt data count.
 * @return  T_VOID
 */
T_VOID  usb_slave_read_ep_cnt(T_U8 EP_index, T_U32 *cnt)
{
    REG8(USB_REG_INDEX) = EP_index;
    *cnt = REG16(USB_REG_RXCOUNT1);
}

/**
 * @brief   set usb controller to enter test mode
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  testmode [in] T_U8 test mode, it can be one of the following value: 
 *
 *        Test_J                 0x1
 *
 *        Test_K                 0x2
 *
 *        Test_SE0_NAK       0x3
 *
 *        Test_Packet          0x4
 *
 *        Test_Force_Enable  0x5
 *
 * @return  T_VOID
 */
T_VOID usb_slave_enter_testmode(T_U8 testmode)
{
    const unsigned char test_packet_data[64] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
    0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0x7E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };   //MENTOR DESIGNE
    
    switch(testmode)
    {
    case Test_SE0_NAK:
        REG8(USB_REG_TESEMODE) = (1 << 0);
        break;

    case Test_J:
        REG8(USB_REG_TESEMODE) = (1 << 1);
        break;

    case Test_K:
        REG8(USB_REG_TESEMODE) = (1 << 2);
        break;

    case Test_Packet:
        {
            T_U32 i;
            
            // write data to usb fifo
            for( i = 0; i < 53; i++)
            {
                REG8(USB_FIFO_EP0) = test_packet_data[i];  
            }

            REG8(USB_REG_TESEMODE) = (1 << 3);

            REG8(USB_REG_CSR0) = USB_CSR0_TXPKTRDY;

            //delay
            for( i = 0; i < 3; i++);
        }
        break;

    default:
        break;
    }
}

//********************************************************************
T_VOID  usb_slave_start_send(EP_INDEX EP_index)
{
    T_U32 tx_index = EP_index;

    usb_slave.ep[tx_index].tx.EP_TX_Count = 0;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;
    usb_slave.usb_need_zero_packet = AK_FALSE;
}

T_U32 usb_slave_get_intr_type(T_U8 *usb_int, T_U16 *usb_ep_int_tx, T_U16 * usb_ep_int_rx)
{
    T_U16 usb_ep_csr;
    T_U32 tmp;
    T_U32 intr_ep = EP_UNKNOWN;
    T_U32 usb_dma_int;
    T_U32 dma_ep_index, dma_intr;

    if(USB_NOTUSE == usb_slave.state)
    {
        return EP_UNKNOWN;
    }
    
    usb_dma_int = REG32(USB_DMA_INTR);
    
    if( (usb_dma_int & DMA_CHANNEL1_INT)  == DMA_CHANNEL1_INT)
    {
        REG8(USB_REG_INDEX) = USB_EP2_INDEX;
        REG8(USB_REG_TXCSR2) = USB_TXCSR_MODE1;
        REG32(USB_DMA_CNTL_1) = 0;

        l2_combuf_wait_dma_finish(usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID);
        return EP2_DMA_INTR;
    }

    if( (usb_dma_int & DMA_CHANNEL2_INT)  == DMA_CHANNEL2_INT)
    {
        if(usb_slave.ep[EP3_INDEX].rx.bDmaStart)
        {
            dma_ep_index = EP3_INDEX;
            dma_intr = EP3_DMA_INTR;
        }
        else
        {
            dma_ep_index = EP1_INDEX;
            dma_intr = EP1_DMA_INTR;
        }
        
        REG8(USB_REG_INDEX) = dma_ep_index;
        REG8(USB_REG_RXCSR2) = 0;
        REG32(USB_DMA_CNTL_2) = 0;
         
        l2_combuf_wait_dma_finish(usb_slave.ep[dma_ep_index].rx.L2_Buf_ID);
        return dma_intr;
    }
    
    usb_slave_read_int_reg(usb_int, usb_ep_int_tx, usb_ep_int_rx);

    //common interrupt
    if( 0 != (*usb_int & USB_INTR_RESET) ||
        0 != (*usb_int & USB_INTR_SUSPEND) ||
        0 != (*usb_int & USB_INTR_RESUME))
        
    {
        return USB_INTR;
    }

    //EP0 INTR
    if(0 != (*usb_ep_int_tx & USB_INTR_EP0))
    {
         intr_ep |= EP0_INTR;
    }

    //EP1 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP4))
    {
        usb_slave_read_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY))
        {
            intr_ep |= EP4_INTR;
        }
        //clear underrun        
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {            
            usb_slave_write_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    
    //EP2 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP2))
    {
        usb_slave_read_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY) && 0 == (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            intr_ep |= EP2_INTR;
        }

        //clear underrun        
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {               
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    
    //receive EP1 INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP1))
    {
        usb_slave_read_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, &usb_ep_csr);

        if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))
        {
            intr_ep |= EP1_INTR;
        }
    
        //clear over run
        if (0 != (usb_ep_csr & USB_RXCSR1_P_OVERRUN))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_OVERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_RXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }

    //receive EP3 INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP3))
    {
        usb_slave_read_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, &usb_ep_csr);

        if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))
        {
            intr_ep |= EP3_INTR;
        }
    
        //clear over run
        if (0 != (usb_ep_csr & USB_RXCSR1_P_OVERRUN))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_OVERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_RXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }

    //EP2 rx INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP2))
    {
        akprintf(C1, M_DRVSYS, "EP2 RX INT!\n");
        return EP_UNKNOWN;
    }
    
    //EP3 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP3))
    {
        akprintf(C1, M_DRVSYS, "EP3 TX INT!\n");
        return EP_UNKNOWN;
    }
    
    return intr_ep;
}


static T_VOID usb_slave_common_intr_handler(T_U8 usb_int)
{
    //RESET
    if(0 != (usb_int & USB_INTR_RESET))
    {
        //prepare to receive the enumeration
        usb_slave_reset();

        if(usb_slave.reset_callback != AK_NULL)
            usb_slave.reset_callback(usb_slave.mode);

        akprintf(C3, M_DRVSYS, "R\n");
        
        return;
    }
    //SUSPEND
    else if(0 != (usb_int & USB_INTR_SUSPEND))
    {
        usb_slave_suspend();
        //enter the suspend mode
        if(USB_OK == usb_slave.state)
        {
            akprintf(C3, M_DRVSYS, "suspend in config,usb done\n");

            if(usb_slave.suspend_callback != AK_NULL)
                usb_slave.suspend_callback();

            usb_slave_set_state(USB_SUSPEND);
        }
        
        return;
    }
    //RESUME
    else if(0 != (usb_int & USB_INTR_RESUME))
    {
        usb_slave_set_state(USB_OK);

        if(usb_slave.resume_callback != AK_NULL)
            usb_slave.resume_callback();
        
        akprintf(C3, M_DRVSYS, "RESUME\n");
        
        return;
    }
}

static T_VOID usb_slave_ep0_rx_handler()
{
    T_U32 data_len;
    T_U8 req_type;
    T_U8 data_dir;
    T_UsbDevReq *pDevReq = (T_UsbDevReq *)&usb_ctrl.ctrl_trans.dev_req;
    
    //stage: idle, setup packet comes
    if(CTRL_STAGE_IDLE == usb_ctrl.ctrl_trans.stage)
    {
        //receive data
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer);
        if(data_len != SETUP_PKT_SIZE)
        {
            akprintf(C1, M_DRVSYS, "error setup packet size %d\r\n",data_len);
            return;
        }

        memcpy(&usb_ctrl.ctrl_trans.dev_req, usb_ctrl.ctrl_trans.buffer, SETUP_PKT_SIZE);
    
        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_SETUP;
        usb_ctrl.tx_count = 0;
        usb_ctrl.ctrl_trans.data_len = 0;

        //analysis bmRequest Type
        req_type = (pDevReq->bmRequestType >> 5) & 0x3;
        data_dir = pDevReq->bmRequestType >> 7;
        
        if(0 == pDevReq->wLength)
        {
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            //no data stage
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;

            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
            if(!data_dir) //data out
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_OUT;
            }
            else         //data in
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_IN;
                //start send
                data_len = usb_ctrl.ctrl_trans.data_len;
                if(data_len > EP0_MAX_PAK_SIZE)
                {
                    data_len = EP0_MAX_PAK_SIZE;
                }
                else if(data_len < EP0_MAX_PAK_SIZE)
                {
                    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
                }

                usb_ctrl.tx_count = data_len;
                usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer, data_len);
            }
        }

        return;
    }

    if(CTRL_STAGE_DATA_OUT == usb_ctrl.ctrl_trans.stage)
    {
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer + usb_ctrl.ctrl_trans.data_len);
        usb_ctrl.ctrl_trans.data_len += data_len;

        if(data_len < EP0_MAX_PAK_SIZE || usb_ctrl.ctrl_trans.data_len > usb_ctrl.ctrl_trans.dev_req.wLength)
        {
            //callback            
            req_type = (pDevReq->bmRequestType >> 5) & 0x3;
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }

            //last packet
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
        }
    }
}

static T_VOID usb_slave_ep0_tx_handler()
{
    T_U8 req_type;
    T_U32 data_trans;
    
    req_type = (usb_ctrl.ctrl_trans.dev_req.bmRequestType >> 5) & 0x3;

    if(CTRL_STAGE_DATA_IN == usb_ctrl.ctrl_trans.stage)
    {
        data_trans = usb_ctrl.ctrl_trans.data_len - usb_ctrl.tx_count;
        if(data_trans > EP0_MAX_PAK_SIZE)
        {
            data_trans = EP0_MAX_PAK_SIZE;
        } 
        else if(data_trans < EP0_MAX_PAK_SIZE)
        {
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }

        //send data
        usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer + usb_ctrl.tx_count, data_trans);
        usb_ctrl.tx_count += data_trans;

        return;
    }

    if(CTRL_STAGE_STATUS == usb_ctrl.ctrl_trans.stage)
    {
        usb_slave_ctrl_callback(req_type);

        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
    }
}


T_VOID usb_slave_ep0_intr_handler(T_U16 usb_ep_int_tx)
{
    T_U16 usb_ep_csr;
    T_BOOL bError = AK_FALSE;
    
    //because EP0's all interrupt is in USB_REG_INTRTX1
    if(0 != (usb_ep_int_tx & USB_INTR_EP0))
    {
        usb_slave_read_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, &usb_ep_csr);

        //setup end
        if (0 != (usb_ep_csr & USB_CSR0_P_SETUPEND))
        {
            usb_slave_write_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, USB_CSR0_P_SVDSETUPEND);
            bError = AK_TRUE;

            //back to idle stage after steup end
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
        }
        //stall
        else if(0 != (usb_ep_csr & USB_CSR0_P_SENTSTALL))
        {
            //clear stall
            usb_slave_ep_clr_stall(EP0_INDEX);
            bError = AK_TRUE;

            //back to idle stage after stall
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE; 
        }
        //rec pkt
        if (0 != (usb_ep_csr & USB_CSR0_RXPKTRDY))
        {
            usb_slave_ep0_rx_handler();
        }
        //send pkt or status end
        else if(!bError)
        {
            usb_slave_ep0_tx_handler();
        }
    }
    

}

static T_VOID usb_slave_tx_handler(EP_INDEX EP_index)
{
    T_U32 tx_index = EP_index;

    if(usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_SENDING)
    {
        return;
    }

    if(usb_slave.ep[tx_index].tx.EP_TX_Count == 0)
    {
        
        if(usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_FINISH)
        {
            usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;
         
            if(usb_slave.ep[tx_index].tx.TX_Finish != AK_NULL)
                usb_slave.ep[tx_index].tx.TX_Finish();
        }

        return;
    }

    usb_slave_send_data(EP_index);
}


static T_VOID usb_slave_rx_handler(EP_INDEX EP_index)
{
    T_U32 i;
    T_U32 *p32;
    T_U16 ret;
    T_U8 *p8;

    T_U32 rx_index = EP_index;
    
    if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
    {
        if(usb_slave.ep[rx_index].rx.RX_Notify != AK_NULL)
            usb_slave.ep[rx_index].rx.RX_Notify();
    }
    else
    {
        usb_slave_receive_data(EP_index);

        if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
        {
            if(usb_slave.ep[rx_index].rx.RX_Finish != AK_NULL)
                usb_slave.ep[rx_index].rx.RX_Finish();    
        }
    }
}


static T_BOOL usb_slave_intr_handler(T_VOID)
{
    T_U8 usb_int;
    T_U16 usb_ep_csr;
    T_U32 tmp, usb_dma_int;
    T_U16 usb_ep_int_tx;
    T_U16 usb_ep_int_rx;
    T_U32 intr_ep = EP_UNKNOWN;
    T_S32 status;

    intr_ep = usb_slave_get_intr_type(&usb_int, &usb_ep_int_tx, &usb_ep_int_rx);

    if((intr_ep & USB_INTR) == USB_INTR)
    {
        usb_slave_common_intr_handler(usb_int);
    }
    
    if((intr_ep & EP0_INTR) == EP0_INTR)
    {
        usb_slave_ep0_intr_handler(usb_ep_int_tx);
    }

    if((intr_ep & EP4_INTR) == EP4_INTR)
    {        
        usb_slave_tx_handler(EP4_INDEX);
    }
    
    if((intr_ep & EP2_INTR) == EP2_INTR)
    {
        usb_slave_tx_handler(EP2_INDEX);
    }
    if ((intr_ep & EP2_DMA_INTR) == EP2_DMA_INTR)
    {
        usb_slave_tx_handler(EP2_INDEX);
    }
    if((intr_ep & EP1_INTR) == EP1_INTR)
    {
        if (usb_slave.ep[EP1_INDEX].rx.bDmaStart)
        {
            usb_slave.ep[EP1_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;       
            usb_slave.ep[EP1_INDEX].rx.bDmaStart = AK_FALSE;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
            l2_combuf_stop_dma(usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID);
        }
        usb_slave_rx_handler(EP1_INDEX);
    }
    
    if((intr_ep & EP3_INTR) == EP3_INTR)
    {

        if (usb_slave.ep[EP3_INDEX].rx.bDmaStart)
        {
            usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;       
            usb_slave.ep[EP3_INDEX].rx.bDmaStart = AK_FALSE;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
            l2_combuf_stop_dma(usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID);
        }
        usb_slave_rx_handler(EP3_INDEX);
    }
    
    if ((intr_ep & EP1_DMA_INTR) == EP1_DMA_INTR)
    {
        usb_slave.ep[EP1_INDEX].rx.bDmaStart = AK_FALSE;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_Count = 0;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_State = EP_RX_FINISH;
        if(usb_slave.ep[EP1_INDEX].rx.RX_Finish != AK_NULL)
                usb_slave.ep[EP1_INDEX].rx.RX_Finish();    
    }

    if ((intr_ep & EP3_DMA_INTR) == EP3_DMA_INTR)
    {
        usb_slave.ep[EP3_INDEX].rx.bDmaStart = AK_FALSE;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Count = 0;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_State = EP_RX_FINISH;
        if(usb_slave.ep[EP3_INDEX].rx.RX_Finish != AK_NULL)
                usb_slave.ep[EP3_INDEX].rx.RX_Finish();    
    }

    return AK_TRUE;
}

T_BOOL usb_detect()
{
    T_U32 i,j = 0;
    T_BOOL stat = AK_FALSE;
    T_U32 tmp;

    //temp disable ... for host not figure out yet 
    //if(usb_host_is_device_connected())
    //{
    //    return AK_FALSE;
    //}

    if(usb_slave.state != USB_NOTUSE)
        return AK_TRUE;

    INTR_DISABLE(IRQ_MASK_USB_BIT);

	tmp = REG32(USB_NEW_CFG_REG);
	tmp &= ~(0x1<<17); 
	REG32(USB_NEW_CFG_REG) = tmp;  //select the slave



    // enable clock, USB PLL, USB 2.0
    sysctl_clock(CLOCK_USB_ENABLE);

    REG32(USB_CONTROL_REG) &= (~0x7); 
    REG32(USB_CONTROL_REG) |= (0x6);        //Enable the usb transceiver and suspend enable

    //ak37xx dosen't have vbus and id pin,so these bit must be set 
    REG32(RTC_USB_CTRL_REG) &= ~(1UL<<31);
    REG32(RTC_USB_CTRL_REG) |= (1<<30);
    REG32(RTC_USB_CTRL_REG) &= ~(1<<29);
    REG32(RTC_USB_CTRL_REG) |= (1<<28);
    REG32(RTC_USB_CTRL_REG) |= (1<<27);
    REG32(RTC_USB_CTRL_REG) |= (1<<26);

    REG8(USB_REG_POWER) = 0x20;

    #define WAIT_USB_DETECT  2000 //ms
    i = get_tick_count();
    
    while(j < WAIT_USB_DETECT)
    {
        j = get_tick_count();
        if (++j > i)
        {
            j -= i;
        }
        else
        {
            j = 0xFFFFFFFF - i + j;
        }
        
        HAL_READ_UINT32(INT_STATUS_REG, tmp);
        if((tmp & 0x2000000) == 0x2000000)
        {
            *( volatile T_U32* )INT_STATUS_REG &= ~0x2000000;
            //HAL_READ_UINT8(USB_REG_INTRUSB, usb_int);         
            if(REG8(USB_REG_INTRUSB)&USB_INTR_SOF)
            {
                stat = AK_TRUE;
                break;
            }
        }
    }

    //close USB clock
    sysctl_clock(~CLOCK_USB_ENABLE);

    REG32(USB_CONTROL_REG) &= (~0x7); 

    REG8(USB_REG_POWER) = 0;

    REG32(RESET_CTRL_REG) |= RESET_CTRL_USB;
    REG32(RESET_CTRL_REG) &= ~RESET_CTRL_USB;
    akprintf(C2, M_DRVSYS, "usb detect %d\n", stat);
    //this delay is necessary when host is mac or linux,otherwise it will be no usb reset before enum when phy is opened again
    mini_delay(500);

    return(stat);
   
}

#endif

