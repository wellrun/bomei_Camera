/*
 * Anyka MAC driver
 * Features
 * Copyright (C) 2010 ANYKA
 * AUTHOR 
 * 10-11-01 09:08:08
 */
#include "drv_api.h"     
#include "machw.h"
#include "phyhw.h"
#include "interrupt.h"
#include "sysctl.h"
#include "mac.h"
#include "arch_mac.h"

#include "drv_module.h"
#include "drv_share_pin.h"

#define TPD_RING_SIZE 0x40
#define RFD_RING_SIZE 0x40
#define RRD_RING_SIZE 0x40

#define TPD_ELEMENT_SIZE 16
#define RFD_ELEMENT_SIZE 8
#define RRD_ELEMENT_SIZE 16

#define TPD_BUFFER_SIZE 0x600
#define RFD_BUFFER_SIZE 0x600

#define FREQ_MIN_MAC 60000000

#define GPIO_78   78
#define GPIO_2    2

#define CTOI(c) (isdigit(c) ? (c - '0') : (c - 'A' + 10))

#define FLAG(_off)				((unsigned int)1 << (_off))
#define BIT_SET(_val, _off)		( (_val) |= FLAG(_off) ) //set the bit
#define BIT_CLEAR(_val, _off)	( (_val) &= ~FLAG(_off) ) // clear the bit
#define BIT_TEST(_val, _off)	(0 != ( (_val) & FLAG(_off) ) ) //test the bit


typedef struct tagMAC_STRUCT
{
    T_U8 gpio_pwr;
    T_U8 gpio_rst;
    
    T_MAC_DMA mac_dma;
    T_MAC_INFO mac_info;

    T_hFreq freq_handle;

    fMAC_RECV_CALLBACK recv_cbk;
    fMAC_STATUS_CALLBACK status_cbk;
}
T_MAC_STRUCT;

static T_MAC_STRUCT m_mac;

static T_BOOL mac_interrupt(T_VOID);
static T_VOID mac_free(T_VOID);

static /*inline*/ T_U32 mac_reg_read(T_U32 offset)
{
	T_U32 macbug;
    T_U32 reg;

    reg = REG32(MAC_MODULE_BASE_ADDR + offset);

    return reg;
}

static /*inline*/ T_VOID mac_reg_write(T_U32 offset, T_U32 value)
{
	T_U32 macbug;
    T_U32 reg;

    REG32(MAC_MODULE_BASE_ADDR + offset) = value;
}

T_VOID mac_delay(T_U32 us)
{
	T_U32 i =0;
	
	for (i=0; i< 10*us*1000; i++);
}

/** * @brief Read Phy Register 
* Read Phy Register from MII Interface 
* @author Tang Anyang
* @date 2010-11-16 
* @param unsigned long RegAddr: Phy Register address
* @retval unsigned long: the value of Phy Register. 
*/
T_U32 MIIRead(T_U32 RegAddr)
{
	T_U32 Val;
	T_U32 i;    
	T_U32 phyVal;

	Val = MDIO_CTRL_REG_ADDR(RegAddr) |	MDIO_CTRL_START | MDIO_CTRL_READ;

	mac_reg_write(REG_MDIO_CTRL, Val);

	for (i=0; i <MDIO_MAX_AC_TIMER; i++)
	{
        Val = mac_reg_read(REG_MDIO_CTRL);

		if (0 == (Val & FLAG(MDIO_CTRL_BUSY_OFF)))
		{
			phyVal = Val & 0xFFFF;        
			goto mr_exit;
		}
		mac_delay(10);
	}

	phyVal = 0;

mr_exit:
	return phyVal;
}

/** * @brief Wrtie Phy Register 
* Write dedicated value to  Phy Register from MII Interface 
* @author Tang Anyang
* @date 2010-11-16 
* @param unsigned long RegAddr: Phy Register address
* @param  unsigned long phyVal: dedicated value. 
*/
T_VOID MIIWrite(T_U32 RegAddr, T_U32 phyVal)
{
	T_U32 Val;
	T_U32 i;

    mac_reg_write(REG_MDIO_CTRL, 0);

	mac_delay(30);
	for(i = 0; i < MDIO_MAX_AC_TIMER; i++)
	{
		Val = mac_reg_read(REG_MDIO_CTRL);

		if (0 == (Val & FLAG(MDIO_CTRL_BUSY_OFF)))
		{
			break;        
		}
		mac_delay(10);
	}

	Val = 
		MDIO_CTRL_DATA(phyVal) |
		MDIO_CTRL_REG_ADDR(RegAddr) |MDIO_CTRL_WRITE|		
		MDIO_CTRL_START;

    mac_reg_write(REG_MDIO_CTRL, Val);

	for (i=0; i <MDIO_MAX_AC_TIMER; i++)
	{
		Val = mac_reg_read(REG_MDIO_CTRL);

		if (0 == (Val & FLAG(MDIO_CTRL_BUSY_OFF)))
		{

			return;
		}
		mac_delay(10);
	}
}

T_BOOL mac_hw_stop(T_VOID)
{
	T_U32 Val, i;
    T_BOOL ret = AK_FALSE;

    Val = mac_reg_read(REG_RXQ_CTRL);

	BIT_CLEAR(Val, RXQ_CTRL_EN_OFF);
	BIT_CLEAR(Val, RXQ_CTRL_Q1_EN_OFF);
	BIT_CLEAR(Val, RXQ_CTRL_Q2_EN_OFF);
	BIT_CLEAR(Val, RXQ_CTRL_Q3_EN_OFF);
	
	mac_reg_write(REG_RXQ_CTRL, Val);

    Val = mac_reg_read(REG_TXQ_CTRL);


	BIT_CLEAR(Val, TXQ_CTRL_EN_OFF);   

	mac_reg_write(REG_TXQ_CTRL, Val);

	//  waiting for rxq/txq be idle 
	for (i=0; i<50; i++)
	{     
        Val = mac_reg_read(REG_IDLE_STATUS);

		if (BIT_TEST(Val, IDLE_STATUS_RXQ_OFF) ||
			BIT_TEST(Val, IDLE_STATUS_TXQ_OFF))
		{
			mac_delay(20);
		}
		else
			break;
	}

	// stop mac tx/rx   
    Val = mac_reg_read(REG_MAC_CTRL);

	BIT_CLEAR(Val, MAC_CTRL_RXEN_OFF);
	BIT_CLEAR(Val, MAC_CTRL_TXEN_OFF);
	
	mac_reg_write(REG_MAC_CTRL, Val);

	mac_delay(10);

	for (i=0; i<50; i++)
	{        
        Val = mac_reg_read(REG_IDLE_STATUS);

		if (0 == (unsigned char)Val)
		{
		    ret = AK_TRUE;
			break;
        }
        
		mac_delay(20);
	}
    
    //disable 25M
    REG32(CHIP_CONF_BASE_ADDR + 0x58) |= (1<<10);

    //close clock
	sysctl_clock(~CLOCK_MAC_ENABLE);

	return ret;
}

void mac_reset(void)
{
	T_U32 Val =0;
	T_U32 i;

	// clear to unmask the corresponding INTs
	mac_reg_write(REG_IMR, 0x00);
	
	// disable interrupt
	mac_reg_write(REG_ISR, FLAG(ISR_DIS_OFF));

	mac_hw_stop();

	// reset whole-MAC safely 
	Val = mac_reg_read(REG_MASTER_CTRL);
	
	BIT_SET(Val, MASTER_CTRL_MAC_SOFT_RST_OFF);
	
	mac_reg_write(REG_MASTER_CTRL, Val);

	mac_delay(50);

	for (i=0; i<50; i++) // wait atmost 1ms 
	{   
		Val = mac_reg_read(REG_IDLE_STATUS);
		if (0 == (unsigned char)Val)
		{
			return ;
		}        
		mac_delay(20);
	}

	return;
}

/* alloc buffer for ringbuf ,rfd, tpd */
T_BOOL mac_mem_alloc(T_MAC_DMA *p_mac_dma)
{
	int i;
	T_U32 *tempp;

    akprintf(C1, M_DRVSYS, "enter mac_mem_alloc\n");

    //alloc tfd ring
	p_mac_dma->tpd_ring_base = drv_malloc(TPD_RING_SIZE * TPD_ELEMENT_SIZE);
	if(AK_NULL == p_mac_dma->tpd_ring_base)
	    return AK_FALSE;

    memset(p_mac_dma->tpd_ring_base, 0, TPD_RING_SIZE * TPD_ELEMENT_SIZE);

    //alloc rfd ring
	p_mac_dma->rfd_ring_base = drv_malloc(RFD_RING_SIZE * RFD_ELEMENT_SIZE);
    if(AK_NULL == p_mac_dma->rfd_ring_base)
        goto ALLOC_RFD_RING_FAIL;

    memset(p_mac_dma->rfd_ring_base, 0, RFD_RING_SIZE * RFD_ELEMENT_SIZE);


    //alloc rrd ring
	p_mac_dma->rrd_ring_base = drv_malloc(RRD_RING_SIZE * RRD_ELEMENT_SIZE);
    if(AK_NULL == p_mac_dma->rrd_ring_base)
        goto ALLOC_RRD_RING_FAIL;

    memset(p_mac_dma->rrd_ring_base, 0, RRD_RING_SIZE * RRD_ELEMENT_SIZE);

    //alloc tx fifo
    p_mac_dma->tpd_fifo_base = drv_malloc(TPD_RING_SIZE * TPD_BUFFER_SIZE);
    if(AK_NULL == p_mac_dma->tpd_fifo_base)
        goto ALLOC_TPD_FIFO_FAIL;

    memset(p_mac_dma->tpd_fifo_base, 0, TPD_RING_SIZE * TPD_BUFFER_SIZE);

    //alloc rx fifo
    p_mac_dma->rfd_fifo_base = drv_malloc(RFD_RING_SIZE * RFD_BUFFER_SIZE);
    if(AK_NULL == p_mac_dma->rfd_fifo_base)
        goto ALLOC_RFD_FIFO_FAIL;

    memset(p_mac_dma->rfd_fifo_base, 0, RFD_RING_SIZE * RFD_BUFFER_SIZE);

	tempp = (T_U32 *)p_mac_dma->rfd_ring_base;
	for(i = 0; i < RFD_RING_SIZE; i++)
	{
	    //low byte of receive buffer
		*tempp = (T_U32)(p_mac_dma->rfd_fifo_base + i*RFD_BUFFER_SIZE);	
		tempp++;

        //high byte of receive buffer
		*tempp =0x00;
		tempp++;
	}

    MMU_Clean_All_DCache();

	return AK_TRUE;

ALLOC_RFD_FIFO_FAIL:
    p_mac_dma->rfd_fifo_base = drv_free(p_mac_dma->rfd_fifo_base);

ALLOC_TPD_FIFO_FAIL:
    p_mac_dma->tpd_fifo_base = drv_free(p_mac_dma->tpd_fifo_base);

ALLOC_RRD_RING_FAIL:
    p_mac_dma->rrd_ring_base = drv_free(p_mac_dma->rrd_ring_base);

ALLOC_RFD_RING_FAIL:
    p_mac_dma->tpd_ring_base = drv_free(p_mac_dma->tpd_ring_base);

    return AK_FALSE;
}
/*
 *  Set AK98 MAC multicast address
 */
static T_VOID mac_hash_table(T_VOID)
{
	T_U32  mac_ctrl_data;
	//unsigned int macbug;
	

	/* Check for Promiscuous and All Multicast modes */
	mac_ctrl_data = mac_reg_read(REG_MAC_CTRL);
    //mac_ctrl_data |= FLAG(MAC_CTRL_PROM_MODE_OFF);
    //mac_ctrl_data |= FLAG(MAC_CTRL_MUTI_ALL_OFF);
    //mac_ctrl_data &= ~(FLAG(MAC_CTRL_PROM_MODE_OFF));
    mac_ctrl_data |= FLAG(MAC_CTRL_PROM_MODE_OFF);

#if 0
	if (ndev->flags & IFF_PROMISC) {
		mac_ctrl_data |= FLAG(MAC_CTRL_PROM_MODE_OFF);
	} 
	else if ((ndev->flags & IFF_ALLMULTI) || (ndev->flags&IFF_MULTICAST))
	{
		mac_ctrl_data |= FLAG(MAC_CTRL_MUTI_ALL_OFF);
		mac_ctrl_data &= ~(FLAG(MAC_CTRL_PROM_MODE_OFF));
	} else {
		mac_ctrl_data &= ~(FLAG(MAC_CTRL_MUTI_ALL_OFF) | FLAG(MAC_CTRL_PROM_MODE_OFF));
	}
#endif

	mac_reg_write(REG_MAC_CTRL, mac_ctrl_data);

	/* clear the old settings from the multicast hash table */
	mac_reg_write(REG_RX_HASH_TABLE, 0);

	mac_reg_write(REG_RX_HASH_TABLE + 4, 0);
}

static void mac_phy_reset(T_VOID)
{
	/* first set phy level low */
    gpio_set_pin_as_gpio(m_mac.gpio_rst);
    gpio_set_pin_dir(m_mac.gpio_rst, 1);

    //set rst to low for 10ms
    gpio_set_pin_level(m_mac.gpio_rst, 0);
	us_delay(10000);
    gpio_set_pin_level(m_mac.gpio_rst, 1);

    //wait 1ms
	us_delay(1000);
}

/** * @brief  choice the phy clock that out of the mcu 
* 
* @author kejianping
* @date 2014-08-12
* @param pin: the GPIO num 
* @return: null
*/
T_VOID mac_phy_mcu_clk_25M(T_U8 pin)
{
	T_U8 tmp = pin;	
	 
	if (tmp != GPIO_78 && tmp != GPIO_2)
	{
		akprintf(C1, M_DRVSYS,"The GPIO of mac clk is err. \n");
		return;		
	}
	
    gpio_share_pin_set(ePIN_AS_CLK25MO, (T_U8 *)&tmp, 1);
    gpio_pin_group_cfg(ePIN_AS_CLK25MO);

}
/** * @brief mac and the phy
* 
* @author Liao_Zhijun
* @date 2014-05-21
* @param p_mac_dma: mac dma pointer
* @param p_mac_info: mac info pointer
*/
T_BOOL mac_hw_init(T_MAC_DMA *p_mac_dma, T_MAC_INFO *p_mac_info)
{
	unsigned long Val = 0;
	unsigned long IntModerate = 100;//500000/5000;

	unsigned int mac_addL;
	unsigned int mac_addH;
	
	unsigned long rrdaddress;
	unsigned int macbug;

	T_U32 i, count;
	T_U32 reg;

    akprintf(C1, M_DRVSYS, "enter mac hw init\n");

	//to enable the 25MHz oscillator
	//TODO: Need to be changed to new clock API
	//REG32(CHIP_CONF_BASE_ADDR + 0x14) |= (1 << 18);
	//REG32(CHIP_CONF_BASE_ADDR + 0x1c) |= (1 << 13);
	//REG32(CHIP_CONF_BASE_ADDR + 0x14) |= (1 << 16|1 << 18);

    //enable mac 25M
    REG32(CHIP_CONF_BASE_ADDR + 0x58) &= ~(1<<10);

    /*for(i=0; i<6; i++)
   	{
       	REG32(CHIP_CONF_BASE_ADDR + 0x14) |= (1 << 20);
    	REG32(CHIP_CONF_BASE_ADDR + 0x14) &= ~(1 << 20);
   	}*/

    //open clock
	sysctl_clock(CLOCK_MAC_ENABLE);

	//reset MAC module
	sysctl_reset(RESET_MAC);

    //share pin
    gpio_pin_group_cfg(ePIN_AS_MAC);

    //pwr gpio
    gpio_set_pin_as_gpio(m_mac.gpio_pwr);
    gpio_set_pin_dir(m_mac.gpio_pwr, 1);
    gpio_set_pin_level(m_mac.gpio_pwr, 1);
    
    //rest gpio
    mac_phy_reset();


	Val = mac_reg_read(0x140c);
	Val |= (5<< 19);
	mac_reg_write(0x140c, Val);

	us_delay(200000);
#if 0
	dbg("BMCR:0x%lx", MIIRead(MII_BMCR));
	dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
	dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
	dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
	dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
	dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
		dbg("PHYSID1:0x%lx, PHYSID2:0x%lx\r\n", MIIRead(MII_PHYSID1), MIIRead(MII_PHYSID2));
#endif	
	p_mac_info->phy_id = MIIRead(MII_PHYSID1);
	if(p_mac_info->phy_id == 0x22)
	{
		MIIWrite(MII_BMCR, MIIRead(MII_BMCR) | 0x1000);
			
	}
	else
	{
	    MIIWrite(MII_BMCR, MIIRead(MII_BMCR) | 0x1100);
	}
	//dbg("BMCR:0x%lx", MIIRead(MII_BMCR));
	//dbg("GIGA_PSSR:0x%lx", MIIRead(MII_GIGA_PSSR));

	/* set mac-address */
	mac_addL = p_mac_info->dev_addr[5] | (p_mac_info->dev_addr[4] << 8)
			| (p_mac_info->dev_addr[3] << 16) | (p_mac_info->dev_addr[2] << 24);
	mac_addH = p_mac_info->dev_addr[1] | (p_mac_info->dev_addr[0] << 8);

	mac_reg_write(REG_MAC_STA_ADDR, mac_addL);
	mac_reg_write(REG_MAC_STA_ADDR+4, mac_addH);

	// clear the Multicast HASH table 
	mac_reg_write(REG_RX_HASH_TABLE, 0x00);
	mac_reg_write(REG_RX_HASH_TABLE+4, 0x00);

	// clear any WOL setting/status /
    Val = mac_reg_read(REG_WOL_CTRL);
	mac_reg_write(REG_WOL_CTRL, 0x00);

	//set tpd address
	mac_reg_write(REG_NTPD_HDRADDR_LO, (T_U32)p_mac_dma->tpd_ring_base);
	mac_reg_write(REG_HTPD_HDRADDR_LO, (T_U32)p_mac_dma->tpd_ring_base);
	mac_reg_write(REG_TX_BASE_ADDR_HI, 0x00);
	//set tpd ring size
	mac_reg_write(REG_TPD_RING_SIZE, TPD_RING_SIZE);

	//set rfd address
	mac_reg_write(REG_RX_BASE_ADDR_HI, 0x00);

	mac_reg_write(REG_RFD0_HDRADDR_LO, (T_U32)p_mac_dma->rfd_ring_base);

	mac_reg_write(REG_RFD1_HDRADDR_LO, 0x00);
	mac_reg_write(REG_RFD2_HDRADDR_LO, 0x00);
	mac_reg_write(REG_RFD3_HDRADDR_LO, 0x00);
	//set rfd ring size
	mac_reg_write(REG_RFD_RING_SIZE, RFD_RING_SIZE);
	//set rfd buffer size
	mac_reg_write(REG_RFD_BUFFER_SIZE, 0x600);

	//set rrd address
	mac_reg_write(REG_RRD0_HDRADDR_LO, (T_U32)p_mac_dma->rrd_ring_base);
	mac_reg_write(REG_RRD1_HDRADDR_LO, 0x00);
	mac_reg_write(REG_RRD2_HDRADDR_LO, 0x00);
	mac_reg_write(REG_RRD3_HDRADDR_LO, 0x00);

	//set rrd ring size
	mac_reg_write(REG_RRD_RING_SIZE, RRD_RING_SIZE);


	mac_reg_write(REG_TXQ_TXF_BURST_L1, 0x00);
	mac_reg_write(REG_RXD_CTRL, 0x00);

	// load all base/mem ptr	 
	mac_reg_write(REG_SRAM_LOAD_PTR, FLAG(SRAM_LOAD_PTR_OFF));

	// set Interrupt Moderator Timer (max interrupt per sec)
	// we use seperate time for rx/tx 
	mac_reg_write(REG_IRQ_MODRT_INIT, IntModerate * 2);
	mac_reg_write(REG_IRQ_MODRT_RX_INIT, IntModerate);

	// set Interrupt Clear Timer
	// HW will enable self to assert interrupt event to system after
	// waiting x-time for software to notify it accept interrupt.	  

	mac_reg_write(REG_INT_RETRIG_TIMER, 10000);

	// Enable Read-Clear Interrupt Mechanism   
	Val = FLAG(MASTER_CTRL_INT_RCLR_EN_OFF);
	BIT_SET(Val, MASTER_CTRL_SA_TIMER_EN_OFF);
	mac_reg_write(REG_MASTER_CTRL, Val);

	mac_reg_write(REG_FC_RXF_HI, 0x03300400);
	mac_reg_write(REG_TXQ_JUMBO_TSO_THRESHOLD, 0xbf);

	// set MTU	 
	mac_reg_write(REG_MTU, 1540);

	// set DMA		
	mac_reg_write(REG_DMA_CTRL, 0x47C14);

	// set TXQ	 
	mac_reg_write(REG_TXQ_CTRL, 0x01000025);

	// set RXQ	
	mac_reg_write(REG_RXQ_CTRL, 0xC0800000);
	
	// rfd producer index	 
	mac_reg_write(REG_RFD0_PROD_INDEX, RFD_RING_SIZE - 1);
    //==========new add here, need confirm========
    //p_mac_dma->rfd_producer_index = RFD_RING_SIZE - 1;

	//set MAC control
	mac_reg_write(REG_MAC_CTRL, 0x06105cef);

	mac_reg_write(REG_IMR, 0x1d608);

	if(p_mac_info->phy_id == 0x22)
	{
		MIIWrite(0x1B, 0x0500);
		MIIWrite(0x1F, 0x8300);
	}
	else if (p_mac_info->phy_id == 0x243)
	{
		MIIWrite(20, 0x04);
		MIIWrite(16, 0x6002);	
		MIIWrite(20, 0x10);
		MIIWrite(29, 0x186);
		MIIWrite(17,0x8600);
	}
	else
	{
	    MIIWrite(MII_IER, 0x0c00);
	}

	mac_hash_table();

	return AK_TRUE;
}


/** * @briefdriver driver module callback type define.
* 
* @author Liao_Zhijun
* @date 2014-05-21
* @param param: mac message
* @param len: message length
* @ret: VOID
*/

T_VOID mac_recv_proc(T_U32 *param, T_U32 len)
{
    receive(&m_mac.mac_dma);
}


/** * @briefdriver module callback type define.
* 
* @author Liao_Zhijun
* @date 2014-05-21
* @param param: mac message
* @param len: message length
* @ret: VOID
*/
T_VOID mac_status_proc(T_U32 *param, T_U32 len)
{
    T_MAC_MSG *p_msg = (T_MAC_MSG *)param;

    if(m_mac.status_cbk != AK_NULL)
    {
        if(MAC_EVENT_LINK_UP == p_msg->event)       
        {
            m_mac.status_cbk(1);
        }
        else if(MAC_EVENT_LINK_DOWN == p_msg->event)
        {
            m_mac.status_cbk(0);
        }
    }
}

/** * @brief Initialize Mac interface
* Initialize MAC and PHY 
* @author Liao_Zhijun
* @date 2014-05-21
* @param mac_addr: mac hardware address
* @param recv_cbk: call back function that will be called when data is received
* @param status_cbk: call back function that will be called when cable in/out
*/
T_BOOL mac_init(T_U8 *mac_addr, fMAC_RECV_CALLBACK recv_cbk, fMAC_STATUS_CALLBACK status_cbk, T_U8 gpio_pwr, T_U8 gpio_rst)
{
	volatile T_U32 count;
    T_U32 i;
    
    if(AK_NULL == mac_addr)
    {
        akprintf(C1, M_DRVSYS, "NULL MAC addr\n");
        return AK_FALSE;
    }
    else
    {  
        akprintf(C1, M_DRVSYS, "MAC addr: \n");
        for(i = 0; i < 6; i++)
        {
            akprintf(C1, M_DRVSYS, "%02x ", mac_addr[i]);
        }
        akprintf(C1, M_DRVSYS, "\n");
    }

    memset(&m_mac.mac_dma, 0, sizeof(m_mac.mac_dma));
    memset(&m_mac.mac_info, 0, sizeof(m_mac.mac_info));

    m_mac.recv_cbk = recv_cbk;
    m_mac.status_cbk = status_cbk;

    m_mac.gpio_pwr = gpio_pwr;
    m_mac.gpio_rst = gpio_rst;

    //copy mac addr
    memcpy(m_mac.mac_info.phy_addr, mac_addr, 6);

    //alloc dma memory
	if (!mac_mem_alloc(&m_mac.mac_dma))
	{
        akprintf(C1, M_DRVSYS, "NULL MAC addr\n");
		return AK_FALSE;
	}
    
    //map message
    DrvModule_Map_Message(DRV_MODULE_MAC, MAC_RECV_MSG, mac_recv_proc);
    //map message
    DrvModule_Map_Message(DRV_MODULE_MAC, MAC_STATUS_MSG, mac_status_proc);

    //create task
    if (!DrvModule_Create_Task(DRV_MODULE_MAC))
    {
	    mac_free();
        return AK_FALSE;
    }

	//hardware init
	if (!mac_hw_init(&m_mac.mac_dma, &m_mac.mac_info))
	{
	    mac_free();
		return AK_FALSE;
    }

    //alloc freq
    m_mac.freq_handle = FreqMgr_RequestFreq(FREQ_MIN_MAC);
    
    //request irq
    int_register_irq(INT_VECTOR_MAC, mac_interrupt);
    
	return AK_TRUE;
}

/** * @brief free all the ram used buy mac
* 
* @author Liao_Zhijun
* @date 2014-05-21
*/
T_VOID mac_free(T_VOID)
{
    T_MAC_DMA *p_mac_dma = &m_mac.mac_dma;
    
    drv_free(p_mac_dma->tpd_ring_base);
    p_mac_dma->tpd_ring_base = AK_NULL;

    drv_free(p_mac_dma->rfd_ring_base);
    p_mac_dma->rfd_ring_base = AK_NULL;

    drv_free(p_mac_dma->rrd_ring_base);
    p_mac_dma->rrd_ring_base = AK_NULL;

    drv_free(p_mac_dma->tpd_fifo_base);
    p_mac_dma->tpd_fifo_base = AK_NULL;

    drv_free(p_mac_dma->rfd_fifo_base);
    p_mac_dma->rfd_fifo_base = AK_NULL;
}

T_U32 get_recv_count(T_MAC_DMA *p_mac_dma)
{
    T_U32 cnt = 0;

	p_mac_dma->rfd_consumer_index = mac_reg_read(REG_RFD0_CONS_INDEX);

    if(p_mac_dma->rfd_consumer_index >= p_mac_dma->rfd_producer_index)
    {
        cnt = p_mac_dma->rfd_consumer_index - p_mac_dma->rfd_producer_index;
    }
    else
    {
        cnt = p_mac_dma->rfd_consumer_index + RFD_RING_SIZE - p_mac_dma->rfd_producer_index;
    }
    
	return cnt;
}

long recv_packet(T_MAC_DMA *p_mac_dma)
{
	PRrdDescr_t prrd;
	short length;
	static unsigned long g_wait_count = 0;
	unsigned char *recvbuffer;
	//unsigned int macbug;

	MMU_InvalidateDCache();
	
	prrd = (PRrdDescr_t)(p_mac_dma->rrd_ring_base + p_mac_dma->rfd_producer_index * 16);
	
	length = prrd->frm_len; 
	if (length == 0)
	{
		if (prrd->nor == 0)
		{
		}
		length = 0;

		p_mac_dma->rfd_producer_index++;
		p_mac_dma->rfd_producer_index %= RFD_RING_SIZE;

        //mac_reg_write(REG_RFD0_PROD_INDEX, p_mac_dma->rfd_producer_index);
		return length;
	}

	if (prrd->updt == 0)
	{
		g_wait_count++;

		if (g_wait_count < 5)
		{
			length = -1;
			
            akprintf(C2, M_DRVSYS, "[1]");
			return length;
		}
	}
	g_wait_count = 0;
	prrd->updt = 0;
	prrd->frm_len = 0;

	recvbuffer = (unsigned char *)(p_mac_dma->rfd_fifo_base + (p_mac_dma->rfd_producer_index)*RFD_BUFFER_SIZE);

    if(m_mac.recv_cbk)
        m_mac.recv_cbk(recvbuffer, length);

    mac_reg_write(REG_RFD0_PROD_INDEX, p_mac_dma->rfd_producer_index);

    p_mac_dma->rfd_producer_index++;
    p_mac_dma->rfd_producer_index %= RFD_RING_SIZE;
    	
	return length;
}

T_U32 send_packet(T_MAC_DMA *p_mac_dma, unsigned char *sendbuffer, unsigned long length)
{
	T_U32 tpdvalue = 0x80000000;	
	T_U8 *RingbufVa;
	T_U32 tpdbufv; 
	T_U32 trans_len;

    //check is buffer full
    m_mac.mac_dma.tpd_consumer_index = mac_reg_read(REG_HTPD_CONS_INDEX);

    if(
        ((p_mac_dma->tpd_producer_index < p_mac_dma->tpd_consumer_index) && 
        (1 == (p_mac_dma->tpd_consumer_index - p_mac_dma->tpd_producer_index)))
        ||
        ((0 == p_mac_dma->tpd_consumer_index) &&
        ((TPD_RING_SIZE-1) == p_mac_dma->tpd_producer_index))
    )
    {
        akprintf(C1, M_DRVSYS, "[mac: tx buffer full]\n");
        return 0;
    }

    trans_len = length;
    if(trans_len > TPD_BUFFER_SIZE)
    {
        trans_len = TPD_BUFFER_SIZE;
    }

	RingbufVa = (unsigned char *)(p_mac_dma->tpd_fifo_base + p_mac_dma->tpd_producer_index*TPD_BUFFER_SIZE);
	memcpy(RingbufVa, sendbuffer, trans_len);

	tpdbufv = (T_U32)(p_mac_dma->tpd_ring_base + p_mac_dma->tpd_producer_index*16); 

	REG32(tpdbufv)= (unsigned long)0x3aa00000+trans_len;

	tpdbufv += 4;
	REG32(tpdbufv)= tpdvalue;

	tpdbufv += 4;

	REG32(tpdbufv)= (T_U32)(p_mac_dma->tpd_fifo_base + p_mac_dma->tpd_producer_index*TPD_BUFFER_SIZE);

	tpdbufv += 4;
	
	p_mac_dma->tpd_producer_index++;
	p_mac_dma->tpd_producer_index %= TPD_RING_SIZE;

    MMU_Clean_All_DCache();
	
	mac_reg_write(REG_HTPD_PROD_INDEX, p_mac_dma->tpd_producer_index);
	
	return trans_len;
}

/*
 * Set AK98 MAC address
 */
static int set_mac_address(T_MAC_INFO *p_mac_info, T_U8 *mac_addr)
{
	unsigned int macbug;
	T_U32 reg;

	memcpy(p_mac_info->dev_addr, mac_addr, 6);

	/* set the Ethernet address */
	reg = p_mac_info->dev_addr[0] | (p_mac_info->dev_addr[1] << 8)
			| (p_mac_info->dev_addr[2] << 16) | (p_mac_info->dev_addr[3] << 24);
    mac_reg_write(REG_MAC_STA_ADDR, reg);

	reg = p_mac_info->dev_addr[4] | (p_mac_info->dev_addr[5] << 8);
    mac_reg_write(REG_MAC_STA_ADDR + 4, reg);

	return 0;
}

int receive(T_MAC_DMA *p_mac_dma)
{
	int length = 0;
	
	while (get_recv_count(p_mac_dma))
	{
		length = recv_packet(p_mac_dma);
		
		if (length == 0)
			break;
		else if (length == -1)
			continue;
	}
	
	return length;
}

/*
  * check whether the mac is connect,
  * if connect, judge the full-duplex or half-duplex
  * and set mac control register 
  */
T_S8 mac_check_link(T_MAC_INFO *p_mac_info)
{
	unsigned long status;
	unsigned long ctrl_reg;
	T_S8 link_status;
    T_MAC_MSG msg;

	ctrl_reg = mac_reg_read(REG_MAC_CTRL);

	status = MIIRead(MII_BMSR);
	status = MIIRead(MII_BMSR);
	
	if(status == 0xFFFF)
    {
        akprintf(C1, M_DRVSYS, "====status error====\n");
        return 0;
    }
	
	if ((status & BMSR_LINK_STATUS) == 0) {
		akprintf(C1, M_DRVSYS, "link down\n");

        msg.event = MAC_EVENT_LINK_DOWN;
        DrvModule_Send_Message(DRV_MODULE_MAC, MAC_STATUS_MSG, (T_U32 *)&msg);
		return 0;
	}
	else
	{
		if ((status & 0x20) == 0) {
    		akprintf(C1, M_DRVSYS, "auto negotiation is not resolved\n");
    		return -1;
		}
		else if (status & 0x4000) {
			akprintf(C1, M_DRVSYS, "link up, full duplex, 100Mb\n");
		}
		else if(status & 0x2000)
		{
		    akprintf(C1, M_DRVSYS, "link up, half duplex, 100Mb\n");
		}
		else if(status & 0x1000)
		{
			akprintf(C1, M_DRVSYS, "link up, full duplex, 10Mb\n");
		}
		else 
		{
		    akprintf(C1, M_DRVSYS, "link up, half duplex, 10Mb\n");
		}

        msg.event = MAC_EVENT_LINK_UP;
        DrvModule_Send_Message(DRV_MODULE_MAC, MAC_STATUS_MSG, (T_U32 *)&msg);
        return 1;
	}	
}

static T_VOID phy_int_handler(T_MAC_INFO *p_mac_info)
{
    if(p_mac_info->phy_id==0x22)
    {
        MIIRead(0x1B);
    }
    else if(p_mac_info->phy_id==0x243)
    {
        MIIRead(17);
    }
    else
    {
        MIIRead(MII_ISR);               
    }
    
    mac_reg_write(REG_ISR, ISR_GPHY_OFF);
    
    mac_check_link(&m_mac.mac_info);
}

static T_BOOL mac_interrupt(T_VOID)
{
	T_MAC_INFO *p_mac_info = &m_mac.mac_info;
	unsigned long flags;
	unsigned long IntStatus = 0;

	//read interrupt status
    IntStatus = mac_reg_read(REG_ISR);

    //phy interrupt
	if(IntStatus & ISR_GPHY_OFF)
	{
        phy_int_handler(p_mac_info);
	}

	//send finish
	if(IntStatus & ISR_TX_PKT_OFF) {
		mac_reg_write(REG_ISR, ISR_TX_PKT_OFF);
	}

	//receive a packet
	if(IntStatus & ISR_RX0_PKT_OFF) {
		mac_reg_write(REG_ISR, ISR_RX0_PKT_OFF);

		DrvModule_Send_Message(DRV_MODULE_MAC, MAC_RECV_MSG, AK_NULL);
	}

	//rx fifo full
	if(IntStatus & ISR_RXF_OV_OFF) {
	    akprintf(C1, M_DRVSYS, "[rfull]");
		mac_reg_write(REG_ISR, ISR_RXF_OV_OFF);

		DrvModule_Send_Message(DRV_MODULE_MAC, MAC_RECV_MSG, AK_NULL);
	}

	//DMAW_OFF
	if(IntStatus & ISR_DMAW_OFF)
	{
		akprintf(C1, M_DRVSYS, "[DMAW timeout]");
		mac_hw_init(&m_mac.mac_dma, &m_mac.mac_info);
	}

	//DMAR_OFF
	if(IntStatus & ISR_DMAR_OFF)
	{
		akprintf(C1, M_DRVSYS, "[DMAR timeout]");
		mac_hw_init(&m_mac.mac_dma, &m_mac.mac_info);
	}

	//TXQ_OFF
	if(IntStatus & ISR_TXQ_OFF)
	{
		akprintf(C1, M_DRVSYS, "[TXQ timeout]");
		mac_hw_init(&m_mac.mac_dma, &m_mac.mac_info);
	}

	return AK_TRUE;
}



/** * @brief Stop the interface, release memory
* @author Liao_Zhijun
* @date 2014-05-21
*/ 
T_BOOL mac_close(T_VOID)
{
    //disable irq
    INTR_DISABLE(IRQ_MASK_MAC_BIT);

    mac_hw_stop();
    
    mac_phy_reset();                     //saving the power    
    gpio_set_pin_as_gpio(m_mac.gpio_rst);
    gpio_set_pin_dir(m_mac.gpio_rst, 0);


    mac_free();

    //cancel freq
    FreqMgr_CancelFreq(m_mac.freq_handle);
    m_mac.freq_handle = FREQ_INVALID_HANDLE;

    DrvModule_Terminate_Task(DRV_MODULE_MAC);

	return AK_TRUE;
}

/** * @brief send data through mac
* @author Liao_Zhijun
* @date 2014-05-21
* @param data: buffer address
* @param data_len: data length to send
*/ 
T_U32 mac_send(T_U8 *data, T_U32 data_len)
{
    T_U32 ret;

    if((AK_NULL == data) || (0 == data_len))
        return 0;

    ret = send_packet(&m_mac.mac_dma, data, data_len);

    return ret;
}

