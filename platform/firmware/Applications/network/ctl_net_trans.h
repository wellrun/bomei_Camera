/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: ctl_net_trans.h
  * @Function:  network transmit interface
  * @Author:    songmengxing
  * @Date:      2014-07-23
  * @Version:   1.0
  */


#ifndef __CTL_NET_TRANS_H__
#define __CTL_NET_TRANS_H__

#ifdef SUPPORT_NETWORK

#include "gbl_macrodef.h"

#include "fwl_net.h"
#include "fwl_osfs.h"

#define NT_CHANNEL_MAX		5

#define NT_KEEP_INFO_MAX	3




typedef struct {
	T_pFILE		fp;
	T_U32		filelen;
	T_U32		translen;
	T_U32		count;
	T_STR_FILE	filename;
	T_SYSTIME	starttime;
	T_SYSTIME	stoptime;
} T_NET_TRANS_FILE;

typedef struct {
	T_BOOL		bSend;	//send or receive
	T_U32		filelen;
	T_STR_FILE	filename;
	T_SYSTIME	starttime;
	T_SYSTIME	stoptime;
} T_NET_TRANS_INFO;

typedef struct {
	
	T_NETCONN_STRUCT*	pNetConn;	//net connect handle
	
	T_hTask 			recvtask;
	T_pVOID 			pRecvStackAddr;
	
	T_hTask 			sendtask;
	T_pVOID 			pSendStackAddr;

	T_U64				recvTotal;	//total receive size
	T_U64				sendTotal;	//total send size

	T_BOOL				bRecving;
	T_BOOL				bSending;
	T_BOOL				bRecvdata;	//receive data or file
	T_BOOL				bShowing;
	T_BOOL				bRecvExit;

	T_hSemaphore		semaphore;

	T_NET_TRANS_FILE	curRecvFile;	//current recving file
	T_NET_TRANS_FILE	curSendFile;	//current sending file
	T_NET_TRANS_INFO	transInfo[NT_KEEP_INFO_MAX];
	
} T_NET_TRANS;


/**
* @brief network transmit handle init
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_NET_TRANS*
* @retval network transmit handle
*/
T_NET_TRANS* NetTrans_Init(T_VOID);

/**
* @brief network transmit handle free
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_VOID*
* @retval AK_NULL
*/
T_VOID* NetTrans_Free(T_NET_TRANS* pTrans);

/**
* @brief create receive task
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
T_BOOL NetTrans_CreateRecvTask(T_NET_TRANS* pTrans);

/**
* @brief try to send
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
T_BOOL NetTrans_TrySend(T_NET_TRANS* pTrans);





#endif
#endif
