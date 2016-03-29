/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: Fwl_net.h
  * @Function:  network interface
  * @Author:    songmengxing
  * @Date:      2014-06-18
  * @Version:   1.0
  */


#ifndef __FWL_NET_H__
#define __FWL_NET_H__

#ifdef SUPPORT_NETWORK

#include "anyka_types.h"

/* Flags for Fwl_Net_Conn_Send*/
#define NETCONN_NOFLAG    0x00
#define NETCONN_NOCOPY    0x00 /* Only for source code compatibility */
#define NETCONN_COPY      0x01
#define NETCONN_MORE      0x02
#define NETCONN_DONTBLOCK 0x04

#define IPADDR_CALC(ipaddr, a,b,c,d) \
        *(ipaddr) = ((T_U32)((d) & 0xff) << 24) | \
        					((T_U32)((c) & 0xff) << 16) | \
							((T_U32)((b) & 0xff) << 8)  | \
							((T_U32)((a) & 0xff))


/** Protocol family and type of the netconn */
typedef enum {
  NETWORKCONN_INVALID    = 0,
  /* NETCONN_TCP Group */
  NETWORKCONN_TCP        = 0x10,
  /* NETCONN_UDP Group */
  NETWORKCONN_UDP        = 0x20,
  NETWORKCONN_UDPLITE    = 0x21,
  NETWORKCONN_UDPNOCHKSUM= 0x22,
  /* NETCONN_RAW Group */
  NETWORKCONN_RAW        = 0x40
}NETWORKCONN_TYPE;

typedef struct {
	
	T_U32		LocalIp;
	T_U32		LocalPort;
	T_U32		RemoteIp;
	T_U32		RemotePort;
	T_BOOL 		bTcpType;
	
} T_CONNECT_INFO;

typedef struct {
	
	T_U8* 	pBuf;
	T_U32 	size;
	T_U8* 	pRead;
	T_U8* 	pWrite;
	T_BOOL 	bEmpty;
	T_BOOL	bFull;
	
} T_CYC_BUF;


typedef enum {
	NET_CLOSE_FLAG_NONE    = 0,
  	NET_CLOSE_FLAG_CLOSING,
  	NET_CLOSE_FLAG_CLOSED

}NET_CLOSE_FLAG;


typedef struct {
	
	T_VOID*			pConn;		//内部使用，外部请勿访问
	T_CYC_BUF		recvbuf;	//内部使用，外部请勿访问
	T_CONNECT_INFO	info;
	NET_CLOSE_FLAG	closeflag;
} T_NETCONN_STRUCT;

/**
* @brief get lwip lib version
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_pSTR
* @retval version string
*/
T_pSTR Fwl_Lwip_GetVersion(T_VOID);

/**
* @brief lwip init, called in VME_Main, can't free
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL Fwl_Lwip_Init(T_VOID);


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
T_S32 Fwl_Net_Ip2str(T_U32 ipaddr, T_U16 *pbuf);


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
T_BOOL Fwl_Net_GetMacAddr(T_U8 *pbuf, T_U32 *len);


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
T_BOOL Fwl_Net_Init(T_U8 *pmac_addr, T_U32 ipaddr, T_U32 netmask, T_U32 gw);

/**
* @brief network free
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL Fwl_Net_Free(T_VOID);

/**
* @brief net connect new
*
* @author Songmengxing
* @date 2014-6-19
* @param in NETWORKCONN_TYPE type:type
* @return T_NETCONN_STRUCT*
* @retval net connect struct pointer
*/
T_NETCONN_STRUCT *Fwl_Net_Conn_New(NETWORKCONN_TYPE type);


/**
* @brief net connect delete
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval 
*/
T_BOOL   Fwl_Net_Conn_Delete(T_NETCONN_STRUCT *pConnStruct);


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
T_BOOL   Fwl_Net_Conn_Bind(T_NETCONN_STRUCT *pConnStruct, T_U32 ipaddr, T_U16 port);


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
T_BOOL   Fwl_Net_Conn_Connect(T_NETCONN_STRUCT *pConnStruct, T_U32 ipaddr, T_U16 port);


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
T_BOOL   Fwl_Net_Conn_Disconnect (T_NETCONN_STRUCT *pConnStruct);


/**
* @brief net connect listen, only for TCP server
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NETCONN_STRUCT *pConnStruct:net connect struct pointer
* @return T_BOOL
* @retval
*/
T_BOOL   Fwl_Net_Conn_Listen(T_NETCONN_STRUCT *pConnStruct);


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
T_BOOL   Fwl_Net_Conn_Accept(T_NETCONN_STRUCT *pConnStruct, T_NETCONN_STRUCT **ppNewConnStruct);


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
T_BOOL   Fwl_Net_Conn_Recv(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32* size, T_U32 flag, T_U32 timeout);

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
T_BOOL   Fwl_Net_Conn_Sendto(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32 size, T_U32 ipaddr, T_U16 port, T_U32 flag);

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
T_BOOL   Fwl_Net_Conn_Send(T_NETCONN_STRUCT *pConnStruct, T_U8 *pdata, T_U32 size, T_U32 flag);


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
T_BOOL   Fwl_Net_Conn_Close(T_NETCONN_STRUCT *pConnStruct);


#endif
#endif


