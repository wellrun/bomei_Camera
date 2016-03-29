

#ifndef __ARCH_MAC_H__
#define __ARCH_MAC_H__

//
typedef T_VOID (*fMAC_RECV_CALLBACK)(T_U8 *buffer, T_U32 length);

typedef T_VOID (*fMAC_STATUS_CALLBACK)(T_U8 link_status);

/** * @brief Initialize Mac interface
* Initialize MAC and PHY 
* @author Liao_Zhijun
* @date 2014-05-21
* @param mac_addr: mac hardware address
* @param recv_cbk: call back function that will be called when data is received
* @param status_cbk: call back function that will be called when cable in/out, indicate the status of phy
*/
T_BOOL mac_init(T_U8 *mac_addr, fMAC_RECV_CALLBACK recv_cbk, fMAC_STATUS_CALLBACK status_cbk, T_U8 gpio_pwr, T_U8 gpio_rst);

/** * @brief Stop the interface, release memory
* @author Liao_Zhijun
* @date 2014-05-21
*/ 
T_BOOL mac_close(T_VOID);

/** * @brief send data through mac
* @author Liao_Zhijun
* @date 2014-05-21
* @param data: buffer address
* @param data_len: data length to send
*/ 
T_U32 mac_send(T_U8 *data, T_U32 data_len);


/** * @brief  choice the phy clock that out of the mcu 
* 
* @author kejianping
* @date 2014-08-12
* @param pin: the GPIO num 
* @return: null
*/
T_VOID mac_phy_mcu_clk_25M(T_U8 pin);


#endif

