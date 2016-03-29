/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: Fwl_net.c
  * @Function:  network interface
  * @Author:    songmengxing
  * @Date:      2014-06-19
  * @Version:   1.0
  */

  
#include "eng_debug.h"

#ifdef SUPPORT_NETWORK
#include "fwl_net.h"
#include "arch_mac.h"
#include "gpio_config.h"
#include "fha_asa.h"
#include "fwl_spiflash.h"
#include "eng_dataconvert.h"
#include "fwl_osmalloc.h"
#include "akos_api.h"


#include "../../Library/lwip/include/ip_addr.h"
#include "../../Library/lwip/include/netif.h"
#include "../../Library/lwip/include/tcpip.h"
#include "../../Library/lwip/include/mac_if.h"
#include "../../Library/lwip/include/api.h"
#include "../../Library/lwip/include/etharp.h"
#include "../../Library/lwip/include/tcp.h"

#define MACADDR_NAME        "MACADDR"
#define MACADDR_LEN_MAX     30


#define LWIP_RECV_BUF_SIZE  (1024<<3)

#define NETWORK_RECV_BUF_SIZE     (LWIP_RECV_BUF_SIZE)	//don't less than LWIP_RECV_BUF_SIZE


struct netif ak_netif;

/**
* @brief cycle receive buffer init
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_CYC_BUF *recvbuf:receive buffer pointer
* @param in T_U32 size:size
* @return T_BOOL
* @retval
*/
static T_BOOL CycBuf_Init(T_CYC_BUF *recvbuf, T_U32 size)
{
	AK_ASSERT_PTR(recvbuf, "CycBuf_Init(): recvbuf err", AK_FALSE);
	AK_ASSERT_VAL(size > 0, "CycBuf_Init(): size err", AK_FALSE);
	
	recvbuf->pBuf = Fwl_Malloc(size);
	AK_ASSERT_PTR(recvbuf->pBuf, "CycBuf_Init(): recvbuf->pBuf malloc failed", AK_FALSE);
	memset(recvbuf->pBuf, 0, size);

	recvbuf->size = size;
	recvbuf->pRead = recvbuf->pBuf;
	recvbuf->pWrite = recvbuf->pBuf;

	recvbuf->bEmpty = AK_TRUE;
	recvbuf->bFull = AK_FALSE;
	return AK_TRUE;
}

/**
* @brief cycle receive buffer free
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_CYC_BUF *recvbuf:receive buffer pointer
* @return T_VOID
* @retval
*/
static T_VOID CycBuf_Free(T_CYC_BUF *recvbuf)
{
	AK_ASSERT_PTR_VOID(recvbuf, "CycBuf_Free(): recvbuf err");
	
	recvbuf->pBuf = Fwl_Free(recvbuf->pBuf);
}

/*
static T_VOID CycBuf_Clean(T_CYC_BUF *recvbuf)
{
	AK_ASSERT_PTR_VOID(recvbuf, "CycBuf_Free(): recvbuf err");
	
	if (AK_NULL == recvbuf->pBuf)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_Clean: recvbuf->pBuf is null!");
		return;
	}
	
	memset(recvbuf->pBuf, 0, recvbuf->size);
	recvbuf->pRead = recvbuf->pBuf;
	recvbuf->pWrite = recvbuf->pBuf;

	recvbuf->bEmpty = AK_TRUE;
	recvbuf->bFull = AK_FALSE;
}*/

/**
* @brief get remain data size
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_CYC_BUF *recvbuf:receive buffer pointer
* @return T_U32
* @retval remain data size
*/
static T_U32 CycBuf_GetRemainDataSize(T_CYC_BUF *recvbuf)
{
	AK_ASSERT_PTR(recvbuf, "CycBuf_GetRemainDataSize(): recvbuf err", 0);
	
	if (AK_NULL == recvbuf->pBuf)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_GetRemainDataSize: recvbuf->pBuf is null!");
		return 0;
	}

	if (recvbuf->bEmpty)
	{
		return 0;
	}
	else if (recvbuf->bFull)
	{
		return recvbuf->size;
	}
	else if (recvbuf->pRead < recvbuf->pWrite)
	{
		return recvbuf->pWrite - recvbuf->pRead;
	}
	else
	{
		return recvbuf->pWrite - recvbuf->pRead + recvbuf->size ;
	}
}

/**
* @brief cycle receive buffer read
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_CYC_BUF *recvbuf:receive buffer pointer
* @param out T_U8* pbuf:dest buf
* @param in/out T_U32* size:in: want size;out: read size
* @return T_BOOL
* @retval
*/
static T_BOOL CycBuf_Read(T_CYC_BUF *recvbuf, T_U8* pbuf, T_U32* size)
{
	T_U32 len = 0;
	T_U32 remain = 0;

	AK_ASSERT_PTR(recvbuf, "CycBuf_Read(): recvbuf err", AK_FALSE);
	AK_ASSERT_PTR(pbuf, "CycBuf_Read(): pbuf err", AK_FALSE);
	AK_ASSERT_PTR(size, "CycBuf_Read(): size err", AK_FALSE);
	AK_ASSERT_VAL(*size > 0, "CycBuf_Read(): *size err", AK_FALSE);
	
	if (AK_NULL == recvbuf->pBuf)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_Read: recvbuf->pBuf is null!");
		return AK_FALSE;
	}

	if (recvbuf->bEmpty)
	{
		return AK_FALSE;
	}

	remain = CycBuf_GetRemainDataSize(recvbuf);

	if (remain < *size)
	{
		if (recvbuf->pRead < recvbuf->pWrite)
		{
			memcpy(pbuf, recvbuf->pRead, remain);
			recvbuf->pRead += remain;
		}
		else
		{
			len = recvbuf->size - (recvbuf->pRead - recvbuf->pBuf);
			memcpy(pbuf, recvbuf->pRead, len);
			memcpy(pbuf + len, recvbuf->pBuf, remain - len);
			recvbuf->pRead = recvbuf->pBuf + (remain - len);
		}

		*size = remain;
	}
	else
	{
		if (recvbuf->pRead < recvbuf->pWrite)
		{
			memcpy(pbuf, recvbuf->pRead, *size);
			recvbuf->pRead += *size;
		}
		else
		{
			len = recvbuf->size - (recvbuf->pRead - recvbuf->pBuf);

			if (len < *size)
			{
				memcpy(pbuf, recvbuf->pRead, len);
				memcpy(pbuf + len, recvbuf->pBuf, *size - len);
				recvbuf->pRead = recvbuf->pBuf + (*size - len);
			}
			else
			{
				memcpy(pbuf, recvbuf->pRead, *size);
				recvbuf->pRead += *size;
			}
		}
	}

	recvbuf->bFull = AK_FALSE;

	if (recvbuf->pRead == recvbuf->pWrite)
	{
		recvbuf->bEmpty = AK_TRUE;
	}

	return AK_TRUE;		
}

/**
* @brief cycle receive buffer write
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_CYC_BUF *recvbuf:receive buffer pointer
* @param in T_U8* pbuf:src buf
* @param in T_U32 size:size
* @return T_BOOL
* @retval
*/
static T_BOOL CycBuf_Write(T_CYC_BUF *recvbuf, T_U8* pbuf, T_U32 size)
{
	T_U32 len = 0;

	AK_ASSERT_PTR(recvbuf, "CycBuf_Write(): recvbuf err", AK_FALSE);
	AK_ASSERT_PTR(pbuf, "CycBuf_Write(): pbuf err", AK_FALSE);
	AK_ASSERT_VAL(size > 0, "CycBuf_Write(): size err", AK_FALSE);
	
	if (AK_NULL == recvbuf->pBuf)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_Write: recvbuf->pBuf is null!");
		return AK_FALSE;
	}

	if (recvbuf->bFull)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_Write: recvbuf->pBuf is full!");
		return AK_FALSE;
	}

	if (recvbuf->size - CycBuf_GetRemainDataSize(recvbuf) < size)
	{
		Fwl_Print(C2, M_NETWORK, "Buf_Write: no enough space!");
		return AK_FALSE;
	}

	if (recvbuf->pWrite + size < recvbuf->pBuf + recvbuf->size)
	{
		memcpy(recvbuf->pWrite, pbuf, size);
		recvbuf->pWrite += size;
	}
	else
	{
		len = recvbuf->size - (recvbuf->pWrite - recvbuf->pBuf);
		memcpy(recvbuf->pWrite, pbuf, len);
		memcpy(recvbuf->pBuf, pbuf + len, size - len);
		recvbuf->pWrite = recvbuf->pBuf + (size - len);
	}

	recvbuf->bEmpty = AK_FALSE;

	if (recvbuf->pRead == recvbuf->pWrite)
	{
		recvbuf->bFull = AK_TRUE;
	}

	return AK_TRUE;		
}

/**
* @brief mac receive call back function
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U8 *buffer:receive data buffer
* @param in T_U32 length:receive data length
* @return T_VOID
* @retval
*/
static T_VOID recv_cbk(T_U8 *buffer, T_U32 length)
{
	struct pbuf *p = AK_NULL;
	err_t ret;

	//Fwl_Print(C3, M_NETWORK, "[r:%d, type:%d,%d]\n", length, buffer[12], buffer[13]);
	
	AK_ASSERT_PTR_VOID(buffer, "recv_cbk(): buffer error");

	

	/* move received packet into a new pbuf */
	p = macif_low_level_input((&ak_netif), buffer, length);
	/* no packet could be read, silently ignore this */
	if (p != AK_NULL) 
	{
	    /* pass all packets to ethernet_input, which decides what packets it supports */
		ret = (&ak_netif)->input(p, &ak_netif);
		
	    if (ret != ERR_OK) 
		{
			Fwl_Print(C2, M_NETWORK, "ethernetif_input: IP input error:%d", ret);
			pbuf_free(p);
	    }
	}
}


/**
* @brief mac status call back function
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U8 link_status:status
* @return T_VOID
* @retval
*/
static T_VOID status_cbk(T_U8 link_status)
{
	Fwl_Print(C2, M_NETWORK, "[s: %d]", link_status);
	macif_set_linkstate(link_status);
}

/**
* @brief netif status call back function
*
* @author Songmengxing
* @date 2014-6-19
* @param in struct netif *netif:netif handle
* @return T_VOID
* @retval
*/
static T_VOID status_callback(struct netif *netif)
{
	AK_ASSERT_PTR_VOID(netif, "status_callback(): netif error");
	
	if (netif_is_up(netif)) 
	{
		Fwl_Print(C2, M_NETWORK, "status_callback==UP, local interface IP is %s", ip_ntoa(&netif->ip_addr));
	} 
	else 
	{
		Fwl_Print(C2, M_NETWORK, "status_callback==DOWN");
	}
}

/**
* @brief netif link call back function
*
* @author Songmengxing
* @date 2014-6-19
* @param in struct netif *netif:netif handle
* @return T_VOID
* @retval
*/
static T_VOID link_callback(struct netif *netif)
{
	AK_ASSERT_PTR_VOID(netif, "link_callback(): netif error");
	
	if (netif_is_link_up(netif)) 
	{
		Fwl_Print(C2, M_NETWORK, "link_callback==UP");
	} 
	else 
	{
		Fwl_Print(C2, M_NETWORK, "link_callback==DOWN");
	}
}

/**
* @brief netif init
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 ipaddr:ip addr
* @param in T_U32 netmask:net mask
* @param in T_U32 gw:gate way
* @return T_VOID
* @retval
*/
static T_VOID msvc_netif_init(T_U32 ipaddr, T_U32 netmask, T_U32 gw)
{
	ip_addr_t ip_addr, net_mask, gate_way;

	gate_way.addr = gw;
	ip_addr.addr = ipaddr;
	net_mask.addr = netmask;

	Fwl_Print(C2, M_NETWORK, "Starting lwIP, local interface IP is %s", ip_ntoa(&ip_addr));

	netif_set_default(netif_add(&ak_netif, &ip_addr, &net_mask, &gate_way, AK_NULL, macif_init, tcpip_input));

	netif_set_status_callback(&ak_netif, status_callback);

	netif_set_link_callback(&ak_netif, link_callback);

	netif_set_up(&ak_netif);

}

/**
* @brief tcp init done
*
* @author Songmengxing
* @date 2014-6-19
* @param in void * arg:arg
* @return T_VOID
* @retval
*/
static T_VOID tcpip_init_done(void * arg)
{
	sys_sem_t *init_sem;
	
	AK_ASSERT_PTR_VOID(arg, "net_init(): arg error");
	init_sem = (sys_sem_t*)arg;

	sys_sem_signal(init_sem);
}



/**
* @brief get mac addr from fha
*
* @author Songmengxing
* @date 2014-6-19
* @param out T_U8 *pbuf:buffer to write mac addr
* @param in/out T_U32 *len:len
* @return T_BOOL
* @retval
*/
T_BOOL Fwl_Net_GetMacAddr(T_U8 *pbuf, T_U32 *len)
{
    T_U8 data_tmp[MACADDR_LEN_MAX] = {0};
    T_U32 data_len = 0;
	T_U8 i = 0;
	T_S8 j = -1;
	T_S8 chartemp = 0;
	T_U8 num = 0;

	AK_ASSERT_PTR(pbuf, "Fwl_Net_GetMacAddr(): pbuf error", AK_FALSE);
	AK_ASSERT_PTR(len, "Fwl_Net_GetMacAddr(): len error", AK_FALSE);

	if (!Fwl_SPI_Fha_Init())
	{
		Fwl_Print(C1, M_NETWORK, "Fwl_SPI_Fha_Init failed!");
        return AK_FALSE;
	}
        
    if (ASA_FILE_SUCCESS != FHA_asa_read_file(MACADDR_NAME, data_tmp, MACADDR_LEN_MAX))
    {
        *len = 0;
		Fwl_Print(C1, M_NETWORK, "FHA_asa_read_file fail!");
		Spi_FHA_close();
        return AK_FALSE;
    }

    
    memcpy(&data_len, data_tmp, sizeof(T_U32));

    if (*len < ETHARP_HWADDR_LEN)
    {
		Fwl_Print(C2, M_NETWORK, "len is too small!");
		Spi_FHA_close();
        return AK_FALSE;
    }

	for(i=0; i<data_len; i++)
    {
        chartemp = data_tmp[4 + i];
		
        if(chartemp >= '0' && chartemp <= '9')
        {
            chartemp -= 0x30;
			j++;
        }
        else if(chartemp >= 'a' && chartemp <= 'f')
        {
            chartemp -= 87;
			j++;
        }
        else if(chartemp >= 'A' && chartemp <= 'F')
        {
            chartemp -= 55;
			j++;
        }
        else
        {
            continue;
        }

		if (0 == j % 2)
		{
			num = (chartemp & 0x0f) << 4;
		}
		else
		{
			num |= chartemp & 0x0f;
			pbuf[j>>1] = num;
		}

    }

    *len = ETHARP_HWADDR_LEN;

	Spi_FHA_close();
        
    return AK_TRUE;
}

/**
* @brief convert ip addr to unicode string
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 ipaddr:ip addr
* @param out T_U16 *pbuf:unicode buf
* @return T_S32
* @retval len
*/
T_S32 Fwl_Net_Ip2str(T_U32 ipaddr, T_U16 *pbuf)
{
	ip_addr_t ip_addr;
	
	AK_ASSERT_PTR(pbuf, "Fwl_Net_Ip2str(): pbuf error", -1);

	ip_addr.addr = ipaddr;

	return Eng_StrMbcs2Ucs(ip_ntoa(&ip_addr), pbuf);
}


/**
* @brief get lwip lib version
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_pSTR
* @retval version string
*/
T_pSTR Fwl_Lwip_GetVersion(T_VOID)
{
	return Lwip_GetVersion();
}


/**
* @brief lwip init, called in VME_Main, can't free
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL Fwl_Lwip_Init(T_VOID)
{
	sys_sem_t init_sem;

	sys_sem_new(&init_sem, 0);

	tcpip_init(tcpip_init_done, &init_sem);
	
	sys_sem_wait(&init_sem);

	sys_sem_free(&init_sem);

	return AK_TRUE;

}

/**
* @brief network init
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U8 *pmac_addr:mac addr
* @param in T_U32 ipaddr:ip addr
* @param in T_U32 netmask:net mask
* @param in T_U32 gw:gate way
* @return T_BOOL
* @retval
*/
T_BOOL Fwl_Net_Init(T_U8 *pmac_addr, T_U32 ipaddr, T_U32 netmask, T_U32 gw)
{
	T_BOOL ret = AK_FALSE;

	AK_ASSERT_PTR(pmac_addr, "Fwl_Net_Init(): pmac_addr error", AK_FALSE);

	macif_set_macaddr(pmac_addr);

	ret = mac_init(pmac_addr, recv_cbk, status_cbk, GPIO_MAC_PWR, GPIO_MAC_RST);

	if (!ret)
	{
		Fwl_Print(C1, M_NETWORK, "Fwl_Net_Init mac_init failed!");
		return ret;
	}

	Fwl_Print(C2, M_NETWORK, "Fwl_Net_Init mac_init OK!");


	/* init network interfaces */
	msvc_netif_init(ipaddr, netmask, gw);

	return ret;
}

/**
* @brief network free
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL Fwl_Net_Free(T_VOID)
{
	netif_remove(&ak_netif);
	
	mac_close();

	return AK_TRUE;
}

/**
* @brief net connect new
*
* @author Songmengxing
* @date 2014-6-19
* @param in NETWORKCONN_TYPE type:type
* @return T_NETCONN_STRUCT*
* @retval net connect struct pointer
*/
T_NETCONN_STRUCT *Fwl_Net_Conn_New(NETWORKCONN_TYPE type)
{
	struct netconn *pConn = NULL;
	T_NETCONN_STRUCT *pConnStruct = AK_NULL;

	pConnStruct = (T_NETCONN_STRUCT *)Fwl_Malloc(sizeof(T_NETCONN_STRUCT));
	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_New(): pConnStruct malloc failed", AK_NULL);
	memset(pConnStruct, 0, sizeof(T_NETCONN_STRUCT));
		
	pConn = netconn_new(type);

	if (AK_NULL == pConn)
	{
		Fwl_Print(C1, M_NETWORK, "netconn_new failed!");
		pConnStruct = Fwl_Free(pConnStruct);
		return AK_NULL;
	}

	netconn_set_recvbufsize(pConn, LWIP_RECV_BUF_SIZE);

	if (!CycBuf_Init(&pConnStruct->recvbuf, NETWORK_RECV_BUF_SIZE))
	{
		Fwl_Print(C1, M_NETWORK, "CycBuf_Init failed!");
		netconn_delete(pConn);
		pConnStruct = Fwl_Free(pConnStruct);
		return AK_NULL;
	}
	
	pConnStruct->pConn = (T_VOID*)pConn;

	return pConnStruct;
}

/**
* @brief net connect delete
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval 
*/
T_BOOL   Fwl_Net_Conn_Delete(T_NETCONN_STRUCT *pConnStruct)
{
	err_t err;
	T_BOOL ret = AK_FALSE;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Delete(): pConnStruct error", ret);

	CycBuf_Free(&pConnStruct->recvbuf);

	err = netconn_delete((struct netconn *)(pConnStruct->pConn));

	if (ERR_OK == err)
	{
		ret = AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_delete err:%d", err);
	}
	
	Fwl_Free(pConnStruct);

	return ret;
}

/**
* @brief net connect bind
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param in T_U32 ipaddr:local ip addr
* @param in T_U16 port:local port, 0 means appointed by the protocol. 
						if don't care the value of local port, such as tcp client, suggest to use 0 to solve
						the problem of port reuse .
				
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Bind(T_NETCONN_STRUCT *pConnStruct, T_U32 ipaddr, T_U16 port)
{
	err_t err;
	ip_addr_t ip_addr;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Bind(): pConnStruct error", AK_FALSE);

	ip_addr.addr = ipaddr;

	err = netconn_bind((struct netconn *)(pConnStruct->pConn), &ip_addr, port);

	if (ERR_OK == err)
	{
		pConnStruct->info.LocalPort = ((struct netconn *)(pConnStruct->pConn))->pcb.tcp->local_port;
		
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_bind err:%d", err);
		return AK_FALSE;
	}
}

/**
* @brief net connect
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param in T_U32 ipaddr:remote ip addr
* @param in T_U16 port:remote port
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Connect(T_NETCONN_STRUCT *pConnStruct, T_U32 ipaddr, T_U16 port)
{
	err_t err;
	ip_addr_t ip_addr;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Connect(): pConnStruct error", AK_FALSE);

	ip_addr.addr = ipaddr;

	err = netconn_connect((struct netconn *)(pConnStruct->pConn), &ip_addr, port);

	if (ERR_OK == err)
	{
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_connect err:%d", err);
		return AK_FALSE;
	}
}

//only for UDP
/**
* @brief net connect disconnect only for UDP
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Disconnect (T_NETCONN_STRUCT *pConnStruct)
{
	err_t err;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Disconnect(): pConnStruct error", AK_FALSE);

	err = netconn_disconnect((struct netconn *)(pConnStruct->pConn));

	if (ERR_OK == err)
	{
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_disconnect err:%d", err);
		return AK_FALSE;
	}
}

/**
* @brief net connect listen, only for TCP server
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Listen(T_NETCONN_STRUCT *pConnStruct)
{
	err_t err;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Listen(): pConnStruct error", AK_FALSE);

	err = netconn_listen((struct netconn *)(pConnStruct->pConn));

	if (ERR_OK == err)
	{
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_listen err:%d", err);
		return AK_FALSE;
	}
}

/**
* @brief net connect accept, only for TCP server
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param out T_NETCONN_STRUCT **ppNewConnStruct:new net connect struct pointer
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Accept(T_NETCONN_STRUCT *pConnStruct, T_NETCONN_STRUCT **ppNewConnStruct)
{
	err_t err;
	struct netconn *ppnew_conn = AK_NULL;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Accept(): pConnStruct error", AK_FALSE);
	AK_ASSERT_PTR(ppNewConnStruct, "Fwl_Net_Conn_Accept(): ppNewConnStruct error", AK_FALSE);

	err = netconn_accept((struct netconn *)(pConnStruct->pConn), &ppnew_conn);

	if (ERR_OK == err)
	{
		*ppNewConnStruct = (T_NETCONN_STRUCT *)Fwl_Malloc(sizeof(T_NETCONN_STRUCT));
		AK_ASSERT_PTR(*ppNewConnStruct, "Fwl_Net_Conn_Accept(): ppNewConnStruct malloc failed", AK_FALSE);
		memset(*ppNewConnStruct, 0, sizeof(T_NETCONN_STRUCT));

		netconn_set_recvbufsize(ppnew_conn, LWIP_RECV_BUF_SIZE);

		if (!CycBuf_Init(&((*ppNewConnStruct)->recvbuf), NETWORK_RECV_BUF_SIZE))
		{
			Fwl_Print(C1, M_NETWORK, "Fwl_Net_Conn_Accept CycBuf_Init failed!");
			netconn_delete(ppnew_conn);
			*ppNewConnStruct = Fwl_Free(*ppNewConnStruct);
			return AK_FALSE;
		}

		(*ppNewConnStruct)->pConn = (T_VOID*)ppnew_conn;

		(*ppNewConnStruct)->info.RemoteIp = ppnew_conn->pcb.tcp->remote_ip.addr;
		(*ppNewConnStruct)->info.RemotePort = ppnew_conn->pcb.tcp->remote_port;
		
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_accept err:%d", err);
		return AK_FALSE;
	}
}


/**
* @brief net connect receive
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param out T_U8 *pdata:data buf
* @param in/out T_U32* size:size
* @param in T_U32 flag:flag(reserved)
* @param in T_U32 timeout:timeout in ms, 0 means forever
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Recv(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32* size, T_U32 flag, T_U32 timeout)
{
	err_t err;
	struct netbuf *buf = AK_NULL;
	struct pbuf *p = AK_NULL;
	T_U16 len = 0;
	T_U16 copylen = 0;
	T_U8* pTemp = AK_NULL;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Recv(): pConnStruct error", AK_FALSE);
	AK_ASSERT_PTR(pdata, "Fwl_Net_Conn_Recv(): pdata error", AK_FALSE);
	AK_ASSERT_PTR(size, "Fwl_Net_Conn_Recv(): size error", AK_FALSE);

	netconn_set_recvtimeout((struct netconn *)(pConnStruct->pConn), timeout);

	if (CycBuf_Read(&pConnStruct->recvbuf, pdata, size))
	{
		return AK_TRUE;
	}
	
	err = netconn_recv((struct netconn *)(pConnStruct->pConn), &buf);

	if (ERR_OK == err)
	{		
		if (*size >= buf->p->tot_len)
		{
			p = buf->p;
			pTemp = pdata;
				
			while(AK_NULL != p)
			{
				memcpy(pTemp, p->payload, p->len);	
				pTemp += p->len;
				p = p->next;
			}
			
			*size = buf->p->tot_len;
		}
		else
		{
			p = buf->p;
			pTemp = pdata;
			
			while((AK_NULL != p) && (len < *size))
			{
				if (len + p->len < *size)
				{
					memcpy(pTemp, p->payload, p->len);
					pTemp += p->len;
					len += p->len;
					p = p->next;
				}
				else if (len + p->len == *size)
				{
					memcpy(pTemp, p->payload, p->len);
					pTemp += p->len;
					len += p->len;
					p = p->next;
					copylen = 0;
					break;
				}
				else
				{
					copylen = *size - len;
					memcpy(pTemp, p->payload, copylen);
					len = *size;
					
					break;
				}
			}

			while (AK_NULL != p)
			{			
				CycBuf_Write(&pConnStruct->recvbuf, (T_U8*)p->payload + copylen, (T_U32)(p->len - copylen));
				p = p->next;
				copylen = 0;
			}
		}

		netbuf_delete(buf);

		return AK_TRUE;
	}
	else
	{
		*size = 0;
		Fwl_Print(C2, M_NETWORK, "netconn_recv err:%d", err);

		if (ERR_CLSD == err)
		{
			pConnStruct->closeflag = NET_CLOSE_FLAG_CLOSED;
		}
		
		return AK_FALSE;
	}
}

//only for UDP
/**
* @brief net connect send to, only for UDP
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param in T_U8 *pdata:data buf
* @param in T_U32 size:size
* @param in T_U32 ipaddr:remote ip addr
* @param in T_U16 port:remote port
* @param in T_U32 flag:flag(reserved)
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Sendto(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32 size, T_U32 ipaddr, T_U16 port, T_U32 flag)
{
	err_t err;
	ip_addr_t ip_addr;
	struct netbuf *buf = AK_NULL;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Sendto(): pConnStruct error", AK_FALSE);
	AK_ASSERT_PTR(pdata, "Fwl_Net_Conn_Sendto(): pdata error", AK_FALSE);

	ip_addr.addr = ipaddr;

	buf = netbuf_new();
	netbuf_ref(buf, pdata, size);
	
	err = netconn_sendto((struct netconn *)(pConnStruct->pConn), buf, &ip_addr, port);

	netbuf_delete(buf);

	if (ERR_OK == err)
	{
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_sendto err:%d", err);
		return AK_FALSE;
	}
}

//only for TCP
/**
* @brief net connect send, only for TCP
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @param in T_U8 *pdata:data buf
* @param in T_U32 size:size
* @param in T_U32 flag:flag
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Send(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32 size, T_U32 flag)
{
	err_t err;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Send(): pConnStruct error", AK_FALSE);
	AK_ASSERT_PTR(pdata, "Fwl_Net_Conn_Send(): pdata error", AK_FALSE);

	err = netconn_write((struct netconn *)(pConnStruct->pConn), pdata, size, flag);

	if (ERR_OK == err)
	{
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_write err:%d", err);
		return AK_FALSE;
	}
}

//only for TCP
/**
* @brief net connect close only for TCP
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Close(T_NETCONN_STRUCT *pConnStruct)
{
	err_t err;
	T_U32 count = 0;

	AK_ASSERT_PTR(pConnStruct, "Fwl_Net_Conn_Close(): pConnStruct error", AK_FALSE);

	pConnStruct->closeflag = NET_CLOSE_FLAG_CLOSING;

	/*close连接时，设置为非阻塞，防止发送一直不返回导致close失败。
	(若正在发送，设置为非阻塞后，发送会在2秒内退出。)
	*/
	netconn_set_nonblocking((struct netconn *)(pConnStruct->pConn), AK_TRUE);

NET_CONN_CLOSE:
	
	err = netconn_close((struct netconn *)(pConnStruct->pConn));

	if (ERR_OK == err)
	{
		pConnStruct->closeflag = NET_CLOSE_FLAG_CLOSED;
		return AK_TRUE;
	}
	else
	{
		Fwl_Print(C2, M_NETWORK, "netconn_close err:%d", err);

		if ((ERR_INPROGRESS == err)
			&& (count < 30))
		{
			AK_Sleep(100);
			count++;
			
			goto NET_CONN_CLOSE;
		}
		
		return AK_FALSE;
	}
}
#endif


