/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: ctl_net_trans.c
  * @Function:  network transmit interface
  * @Author:    songmengxing
  * @Date:      2014-07-23
  * @Version:   1.0
  */

#include "gbl_global.h"

#ifdef SUPPORT_NETWORK
#include "ctl_net_trans.h"
#include "eng_debug.h"
#include "eng_time.h"
#include "akos_api.h"
#include "eng_string_uc.h"
#include "eng_string.h"
#include "eng_dataconvert.h"
#include "fwl_osmalloc.h"
#include "fwl_initialize.h"




#define NT_SEND_BUF_SIZE	(1024<<3)
#define NT_RECV_BUF_SIZE	NT_SEND_BUF_SIZE	

#define NT_STACK_SIZE		(20*1024)


#define NT_FDATA_PCKT_LEN_SIZE	4

#define NT_FILE_FLAG_SIZE		2

#define NT_FILE_LEN_SIZE		4
#define NT_FDATA_PCKT_CNT_SIZE	4

#define NT_FDATA_PCKT_ID_SIZE	4

#define NT_FILE_INFO_HEAD_SIZE	(NT_FDATA_PCKT_LEN_SIZE + NT_FILE_FLAG_SIZE + NT_FILE_LEN_SIZE + NT_FDATA_PCKT_CNT_SIZE)
#define NT_FILE_DATA_HEAD_SIZE	(NT_FDATA_PCKT_LEN_SIZE + NT_FILE_FLAG_SIZE + NT_FDATA_PCKT_ID_SIZE)

#define FILE_INFO_FLAG			0x5a5a
#define FILE_DATA_FLAG			0x6b6b

/*********************************************************************
							传输文件的包格式
文件信息包
bytes:			4			2				4		4			packet len-14
content:		packet len	flag(0x5a5a)		file len	packet count		file name

文件数据包
bytes:			4			2				4		packet len-10
content:		packet len	flag(0x6b6b)		packet id		file data

**********************************************************************/
extern T_BOOL Network_Trans_DataSet(T_U8* pData, T_U32 len);
extern T_BOOL Network_Trans_InfoSet(T_NET_TRANS_INFO *pTransInfo);

/**
* @brief get empty info id
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_U8
* @retval empty info id
*/
static T_U8 NetTrans_GetEmpInfo_Id(T_NET_TRANS* pTrans)
{
	T_U8 i = 0;
	T_U8 id = NT_KEEP_INFO_MAX;

	AK_ASSERT_PTR(pTrans, "NetTrans_GetEmpInfo_Id(): pTrans null", id);

	for (i=0; i<NT_KEEP_INFO_MAX; i++)
	{
		if (0 == pTrans->transInfo[i].filename[0])
		{
			id = i;
			break;
		}
	}

	if (id >= NT_KEEP_INFO_MAX)
	{		
		for (i=0; i<NT_KEEP_INFO_MAX - 1; i++)
		{
			memcpy(&pTrans->transInfo[i], &pTrans->transInfo[i+1], sizeof(T_NET_TRANS_INFO));
		}
		
		memset(&pTrans->transInfo[NT_KEEP_INFO_MAX - 1], 0, sizeof(T_NET_TRANS_INFO));

		id = NT_KEEP_INFO_MAX - 1;
	}

	return id;
}

/**
* @brief receive or send file info set
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @param in T_BOOL bSend:send or receive
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_InfoSet(T_NET_TRANS* pTrans, T_BOOL bSend)
{
	T_U8 id = NT_KEEP_INFO_MAX;
	
	AK_ASSERT_PTR(pTrans, "NetTrans_InfoSet(): pTrans null", AK_FALSE);

	AK_Obtain_Semaphore(pTrans->semaphore, AK_SUSPEND);

	id = NetTrans_GetEmpInfo_Id(pTrans);

	if (id >= NT_KEEP_INFO_MAX)
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_InfoSet, id err!");
		AK_Release_Semaphore(pTrans->semaphore);
		return AK_FALSE;
	}

	if (bSend)
	{
		pTrans->transInfo[id].bSend = AK_TRUE;
		pTrans->transInfo[id].filelen = pTrans->curSendFile.filelen;
		Utl_StrCpy(pTrans->transInfo[id].filename, pTrans->curSendFile.filename);
		pTrans->transInfo[id].starttime = pTrans->curSendFile.starttime;
		pTrans->transInfo[id].stoptime = pTrans->curSendFile.stoptime;		
	}
	else
	{
		pTrans->transInfo[id].bSend = AK_FALSE;
		pTrans->transInfo[id].filelen = pTrans->curRecvFile.filelen;
		Utl_StrCpy(pTrans->transInfo[id].filename, pTrans->curRecvFile.filename);
		pTrans->transInfo[id].starttime = pTrans->curRecvFile.starttime;
		pTrans->transInfo[id].stoptime = pTrans->curRecvFile.stoptime;
	}

	if (pTrans->bShowing)
	{
		Network_Trans_InfoSet(&pTrans->transInfo[id]);
	}

	AK_Release_Semaphore(pTrans->semaphore);

	return AK_TRUE;
}

/**
* @brief receive file packet deal
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @param in T_U8* data:receive data
* @param in T_U32 packetlen:packetlen
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_Recv_FilePktDeal(T_NET_TRANS* pTrans, T_U8*data, T_U32 packetlen)
{
	T_U16 filename_len = 0;
	T_U8	*pbuf = AK_NULL;
	T_STR_FILE filepath = {0};
	T_U16	flag = 0;
	T_U32	id = 0;
	T_U32	datalen = 0;
	T_U32	i = 0;
	T_WSTR_20 tmpstr = {0};
	T_STR_FILE filename = {0};
	T_WSTR_20 ext = {0};

	AK_ASSERT_PTR(pTrans, "NetTrans_Recv_PktDeal(): pTrans null", AK_FALSE);
	AK_ASSERT_PTR(data, "NetTrans_Recv_PktDeal(): data null", AK_FALSE);
	
	memcpy(&flag, data, NT_FILE_FLAG_SIZE);

	if (FILE_INFO_FLAG == flag)
	{
		pTrans->bRecving = AK_TRUE;
		
		filename_len = packetlen - NT_FILE_INFO_HEAD_SIZE;
		Fwl_Print(C3, M_NETWORK, "Recv filename_len = %d!", filename_len);

		memcpy(&pTrans->curRecvFile.filelen, data+NT_FILE_FLAG_SIZE, NT_FILE_LEN_SIZE);
		Fwl_Print(C3, M_NETWORK, "Recv filelen = %d!", pTrans->curRecvFile.filelen);

		memcpy(&pTrans->curRecvFile.count, data+NT_FILE_FLAG_SIZE+NT_FILE_LEN_SIZE, NT_FDATA_PCKT_CNT_SIZE);
		Fwl_Print(C3, M_NETWORK, "Recv count = %d!", pTrans->curRecvFile.count);

		Utl_StrCpyN(pTrans->curRecvFile.filename, data + NT_FILE_INFO_HEAD_SIZE - NT_FDATA_PCKT_LEN_SIZE, filename_len);
		Fwl_Print(C3, M_NETWORK, "Recv filename : %s!", pTrans->curRecvFile.filename);

		Eng_StrUcs2Mbcs(Fwl_GetDefPath(eNETWORK_PATH), filepath);
		Utl_StrCat(filepath, pTrans->curRecvFile.filename);
		Fwl_Print(C3, M_NETWORK, "Recv path : %s!", filepath);
		
		pTrans->curRecvFile.translen = 0;
		
		if (FS_INVALID_HANDLE != pTrans->curRecvFile.fp)
		{
			Fwl_FileClose(pTrans->curRecvFile.fp);
			pTrans->curRecvFile.fp = FS_INVALID_HANDLE;
		}

		pTrans->curRecvFile.fp = Fwl_FileOpenAsc(filepath, _FMODE_CREATE, _FMODE_CREATE);

		if ((FS_INVALID_HANDLE == pTrans->curRecvFile.fp) 
			&& (Fwl_FileExistAsc(filepath)))
		{
			Fwl_Print(C3, M_NETWORK, "file exist!");

			for (i=0; i<0xffffffff; i++)
			{
				SplitFileName(pTrans->curRecvFile.filename, filename, ext);
				Utl_StrCat(filename, "_");
				
				Utl_Itoa(i, tmpstr, 10);
				Utl_StrCat(filename, tmpstr);
				Utl_StrCat(filename, ".");
				Utl_StrCat(filename, ext);

				Eng_StrUcs2Mbcs(Fwl_GetDefPath(eNETWORK_PATH), filepath);
				Utl_StrCat(filepath, filename);

				if (!Fwl_FileExistAsc(filepath))
				{
					Fwl_Print(C3, M_NETWORK, "Rename:%s!", filepath);
					pTrans->curRecvFile.fp = Fwl_FileOpenAsc(filepath, _FMODE_CREATE, _FMODE_CREATE);
					Utl_StrCpy(pTrans->curRecvFile.filename, filename);
					
					break;
				}
			}
		}

		if (FS_INVALID_HANDLE == pTrans->curRecvFile.fp)
		{
			Fwl_Print(C2, M_NETWORK, "the File create failed, can't save!");
			//return AK_FALSE;
		}

		pTrans->curRecvFile.starttime = GetSysTime();

		return AK_TRUE;
	}
	else if (FILE_DATA_FLAG == flag)
	{
		memcpy(&id, data+NT_FILE_FLAG_SIZE, NT_FDATA_PCKT_ID_SIZE);
		AK_DEBUG_OUTPUT("R:%d\n", id);

		datalen = packetlen - NT_FILE_DATA_HEAD_SIZE;

		pbuf = data + NT_FILE_DATA_HEAD_SIZE - NT_FDATA_PCKT_LEN_SIZE;

		if (FS_INVALID_HANDLE == pTrans->curRecvFile.fp)
		{
			//Fwl_Print(C2, M_NETWORK, "the File is not exist, can't write!");
			//return AK_FALSE;
		}
		else
		{
			if (datalen != Fwl_FileWrite(pTrans->curRecvFile.fp, pbuf, datalen))
			{
				Fwl_Print(C2, M_NETWORK, "Fwl_FileWrite failed!");
				return AK_FALSE;
			}
		}
		
		pTrans->curRecvFile.translen += datalen;
		
		if ((id == pTrans->curRecvFile.count)
			|| (pTrans->curRecvFile.translen == pTrans->curRecvFile.filelen))
		{
			pTrans->curRecvFile.stoptime = GetSysTime();
			
			Fwl_FileClose(pTrans->curRecvFile.fp);
			pTrans->curRecvFile.fp = FS_INVALID_HANDLE;

			Fwl_Print(C2, M_NETWORK, "Recv file save OK! id:%d, count:%d, translen:%d, filelen:%d\n",
				id, pTrans->curRecvFile.count, pTrans->curRecvFile.translen, pTrans->curRecvFile.filelen);

			pTrans->bRecving = AK_FALSE;

			NetTrans_InfoSet(pTrans, AK_FALSE);
		}
		
		return AK_TRUE;
	}
	else
	{		
		Fwl_Print(C2, M_NETWORK, "Recv flag err : %d!", flag);
		
		return AK_FALSE;
	}
}


/**
* @brief send file packet make
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @param out T_U8* data:data to send
* @param out T_U32* packetlen:packetlen
* @param in T_U32 id:packet id, 0 means file info packet
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_Send_FilePktMake(T_NET_TRANS* pTrans, T_U8* data, T_U32* packetlen, T_U32 id)
{
	T_U16 flag = 0;
	T_U16 filename_len = 0;
	T_STR_FILE filepath = {0};
	T_STR_FILE path = {0};
	T_U8 *pbuf = AK_NULL;
	T_U32 datalen = 0;
	T_U32 packetid = 0;
	T_U32 fileRemainlen = 0;
	T_U32 sendbufsize = 0;

	AK_ASSERT_PTR(pTrans, "NetTrans_Send_FilePktMake(): pTrans null", AK_FALSE);
	AK_ASSERT_PTR(data, "NetTrans_Send_FilePktMake(): data null", AK_FALSE);
	AK_ASSERT_PTR(packetlen, "NetTrans_Send_FilePktMake(): packetlen null", AK_FALSE);

	if (pTrans->pNetConn->info.bTcpType)
	{
		sendbufsize = NT_SEND_BUF_SIZE;
	}
	else
	{
		sendbufsize = 1024;
	}

	if (0 == id)	//file info
	{
		Eng_StrUcs2Mbcs(gs.sendfile, filepath);
		Fwl_Print(C3, M_NETWORK, "Send path : %s!", filepath);

		SplitFilePath(filepath, path, pTrans->curSendFile.filename);
		Fwl_Print(C3, M_NETWORK, "Send name : %s!", pTrans->curSendFile.filename);

		pTrans->curSendFile.fp = Fwl_FileOpenAsc(filepath, _FMODE_READ, _FMODE_READ);

		if (FS_INVALID_HANDLE == pTrans->curSendFile.fp)
		{
			Fwl_Print(C2, M_NETWORK, "the File open failed, can't send!");
			return AK_FALSE;
		}
		
		filename_len = (T_U16)Utl_StrLen(pTrans->curSendFile.filename);
		Fwl_Print(C3, M_NETWORK, "Send filename_len = %d!", filename_len);
		
		*packetlen = NT_FILE_INFO_HEAD_SIZE + filename_len;
		memcpy(data, packetlen, NT_FDATA_PCKT_LEN_SIZE);

		flag = FILE_INFO_FLAG;
		memcpy(data+NT_FDATA_PCKT_LEN_SIZE, &flag, NT_FILE_FLAG_SIZE);
		
		pTrans->curSendFile.filelen = Fwl_GetFileLen(pTrans->curSendFile.fp);
		Fwl_Print(C3, M_NETWORK, "Send filelen = %d!", pTrans->curSendFile.filelen);
		
		memcpy(data+NT_FDATA_PCKT_LEN_SIZE+NT_FILE_FLAG_SIZE, &pTrans->curSendFile.filelen, NT_FILE_LEN_SIZE);

		if (0 == pTrans->curSendFile.filelen % (sendbufsize - NT_FILE_DATA_HEAD_SIZE))
		{
			pTrans->curSendFile.count = pTrans->curSendFile.filelen / (sendbufsize - NT_FILE_DATA_HEAD_SIZE);
		}
		else
		{
			pTrans->curSendFile.count = pTrans->curSendFile.filelen / (sendbufsize - NT_FILE_DATA_HEAD_SIZE) + 1;
		}
		
		Fwl_Print(C3, M_NETWORK, "Send count = %d!", pTrans->curSendFile.count);
		memcpy(data+NT_FDATA_PCKT_LEN_SIZE+NT_FILE_FLAG_SIZE+NT_FILE_LEN_SIZE, &pTrans->curSendFile.count, NT_FDATA_PCKT_CNT_SIZE);

		Utl_StrCpy(data + NT_FILE_INFO_HEAD_SIZE, pTrans->curSendFile.filename);

		pTrans->curSendFile.translen = 0;
	}
	else
	{
		if (id > pTrans->curSendFile.count)
		{
			Fwl_Print(C2, M_NETWORK, "NetTrans_Send_FilePktMake id err, id = %d, count = %d!", id, pTrans->curSendFile.count);
			return AK_FALSE;
		}

		packetid = id;
		fileRemainlen = pTrans->curSendFile.filelen - pTrans->curSendFile.translen;
		
		flag = FILE_DATA_FLAG;
		memcpy(data+NT_FDATA_PCKT_LEN_SIZE, &flag, NT_FILE_FLAG_SIZE);
		memcpy(data+NT_FDATA_PCKT_LEN_SIZE+NT_FILE_FLAG_SIZE, &packetid, NT_FDATA_PCKT_ID_SIZE);

		if (fileRemainlen > sendbufsize - NT_FILE_DATA_HEAD_SIZE)
		{
			datalen = sendbufsize - NT_FILE_DATA_HEAD_SIZE;
		}
		else
		{
			datalen = fileRemainlen;
		}

		*packetlen = NT_FILE_DATA_HEAD_SIZE + datalen;
		memcpy(data, packetlen, NT_FDATA_PCKT_LEN_SIZE);

		pTrans->curSendFile.translen += datalen;

		pbuf = data + NT_FILE_DATA_HEAD_SIZE;

		if (datalen != Fwl_FileRead(pTrans->curSendFile.fp, pbuf, datalen))
		{
			Fwl_Print(C2, M_NETWORK, "Fwl_FileRead failed!");
			return AK_FALSE;
		}
	}

	return AK_TRUE;
}

/**
* @brief receive
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_Recv(T_NET_TRANS* pTrans)
{
	T_U8 data[NT_RECV_BUF_SIZE] = {0};
	T_U32 size = NT_RECV_BUF_SIZE;
	T_BOOL ret = AK_FALSE;
	T_U32	packetlen = 0;
	T_U32	recvlen = 0;
	
	AK_ASSERT_PTR(pTrans, "NetTrans_Recv(): pTrans null", AK_FALSE);

	if (pTrans->bRecvdata)
	{
		if (!Fwl_Net_Conn_Recv(pTrans->pNetConn, data, &size, 0, 0))
		{
			Fwl_Print(C2, M_NETWORK, "recv data failed!");
			return ret;
		}

		pTrans->recvTotal += size;

		if (pTrans->bShowing)
		{
			Network_Trans_DataSet(data, size);
		}
		
		return AK_TRUE;
	}

	size = NT_FDATA_PCKT_LEN_SIZE;

	while (recvlen < NT_FDATA_PCKT_LEN_SIZE)
	{
		if (!Fwl_Net_Conn_Recv(pTrans->pNetConn, data + recvlen, &size, 0, 0))
		{
			Fwl_Print(C2, M_NETWORK, "recv packet len failed!");
			return ret;
		}

		pTrans->recvTotal += size;

		recvlen += size;
		size = NT_FDATA_PCKT_LEN_SIZE - recvlen;
	}

	memcpy(&packetlen, data, NT_FDATA_PCKT_LEN_SIZE);

	size = packetlen - NT_FDATA_PCKT_LEN_SIZE;

	if (size > NT_RECV_BUF_SIZE)
	{
		Fwl_Print(C2, M_NETWORK, "the packet is larger than buf!");
		return ret;
	}

	recvlen = 0;

	while (recvlen < packetlen - NT_FDATA_PCKT_LEN_SIZE)
	{
		if (!Fwl_Net_Conn_Recv(pTrans->pNetConn, data + recvlen, &size, 0, 0))
		{
			Fwl_Print(C2, M_NETWORK, "recv packet failed!");
			return ret;
		}

		pTrans->recvTotal += size;

		recvlen += size;
		size = packetlen - NT_FDATA_PCKT_LEN_SIZE - recvlen;
	}

	ret = NetTrans_Recv_FilePktDeal(pTrans, data, packetlen);

	return ret;
}

/**
* @brief send file
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_Send(T_NET_TRANS* pTrans)
{
	T_U8 data[NT_SEND_BUF_SIZE] = {0};
	T_U32 packetlen = 0;
	T_BOOL ret = AK_FALSE;
	T_U32 i = 0;
	
	AK_ASSERT_PTR(pTrans, "NetTrans_Send(): pTrans null", AK_FALSE);

	ret = NetTrans_Send_FilePktMake(pTrans, data, &packetlen, 0);

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_Send_FileInfoPktMake failed!");
		return ret;
	}
	
	if (pTrans->pNetConn->info.bTcpType)
	{
		ret = Fwl_Net_Conn_Send(pTrans->pNetConn, data, packetlen, NETCONN_COPY);
	}
	else
	{
		ret = Fwl_Net_Conn_Sendto(pTrans->pNetConn, data, packetlen, 
									pTrans->pNetConn->info.RemoteIp, 
									pTrans->pNetConn->info.RemotePort, NETCONN_COPY);
	}

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "Send file info failed!");
		Fwl_FileClose(pTrans->curSendFile.fp);
		pTrans->curSendFile.fp = FS_INVALID_HANDLE;
		return ret;
	}
	else
	{
		Fwl_Print(C3, M_NETWORK, "Send file info OK!");
		pTrans->sendTotal += packetlen;
	}

	pTrans->curSendFile.starttime = GetSysTime();

	for (i=1; i<=pTrans->curSendFile.count; i++)
	{			
		if (NET_CLOSE_FLAG_CLOSING == pTrans->pNetConn->closeflag)
		{
			Fwl_Print(C2, M_NETWORK, "Channel is closing, stop sending!");
			Fwl_FileClose(pTrans->curSendFile.fp);
			pTrans->curSendFile.fp = FS_INVALID_HANDLE;
			return AK_FALSE;
		}
		
		memset(data, 0, NT_SEND_BUF_SIZE);

		ret = NetTrans_Send_FilePktMake(pTrans, data, &packetlen, i);

		if (!ret)
		{
			Fwl_Print(C2, M_NETWORK, "NetTrans_Send_FilePktMake %d failed!", i);
			Fwl_FileClose(pTrans->curSendFile.fp);
			pTrans->curSendFile.fp = FS_INVALID_HANDLE;
			return ret;
		}
	
		if (pTrans->pNetConn->info.bTcpType)
		{
			ret = Fwl_Net_Conn_Send(pTrans->pNetConn, data, packetlen, NETCONN_COPY);
		}
		else
		{
			ret = Fwl_Net_Conn_Sendto(pTrans->pNetConn, data, packetlen, 
										pTrans->pNetConn->info.RemoteIp, 
										pTrans->pNetConn->info.RemotePort, NETCONN_COPY);
		}

		if (!ret)
		{
			Fwl_Print(C2, M_NETWORK, "Send file packet %d failed!", i);
			Fwl_FileClose(pTrans->curSendFile.fp);
			pTrans->curSendFile.fp = FS_INVALID_HANDLE;
			return ret;
		}
		else
		{
			AK_DEBUG_OUTPUT("S:%d\n", i);
			pTrans->sendTotal += packetlen;
		}

	}

	Fwl_Print(C3, M_NETWORK, "Send file packet count: %d !", pTrans->curSendFile.count);

	Fwl_FileClose(pTrans->curSendFile.fp);
	pTrans->curSendFile.fp = FS_INVALID_HANDLE;

	pTrans->curSendFile.stoptime = GetSysTime();
	
	NetTrans_InfoSet(pTrans, AK_TRUE);

	return ret;
}

/**
* @brief receive thread function
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 argc:argc
* @param in T_VOID *argv:argv
* @return T_VOID
* @retval 
*/
static T_VOID NetTrans_RecvThread(T_U32 argc, T_VOID *argv)
{
	T_NET_TRANS* pTrans = AK_NULL;
	
	if (AK_NULL == argv)
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_RecvThread argv NULL!");
		return;
	}

	pTrans = (T_NET_TRANS*)argv;

	while (1)
	{		
		if (!NetTrans_Recv(pTrans))
		{
			Fwl_Print(C2, M_NETWORK, "NetTrans_Recv failed!");
			break;
		}
	}	

	pTrans->bRecvExit = AK_TRUE;
	
}


/**
* @brief send thread function
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 argc:argc
* @param in T_VOID *argv:argv
* @return T_VOID
* @retval 
*/
static T_VOID NetTrans_SendThread(T_U32 argc, T_VOID *argv)
{
	T_NET_TRANS* pTrans = AK_NULL;
	
	if (AK_NULL == argv)
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_SendThread argv NULL!");
		return;
	}

	pTrans = (T_NET_TRANS*)argv;

	while (1)
	{
		if (pTrans->bSending)
		{
			NetTrans_Send(pTrans);	
			pTrans->bSending = AK_FALSE;
		}
	}
}

/**
* @brief network transmit handle init
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_NET_TRANS*
* @retval network transmit handle
*/
T_NET_TRANS* NetTrans_Init(T_VOID)
{
	T_NET_TRANS* pTrans = AK_NULL;

	pTrans = (T_NET_TRANS*)Fwl_Malloc(sizeof(T_NET_TRANS));
	AK_ASSERT_PTR(pTrans, "NetTrans_Init(): pTrans malloc failed", AK_NULL);
	memset(pTrans, 0, sizeof(T_NET_TRANS));

	pTrans->recvtask = AK_INVALID_TASK;
	pTrans->sendtask = AK_INVALID_TASK;

	pTrans->curRecvFile.fp = FS_INVALID_HANDLE;
	pTrans->curSendFile.fp = FS_INVALID_HANDLE;

	pTrans->semaphore = AK_Create_Semaphore(1, AK_PRIORITY);

	return pTrans;
}

/**
* @brief network transmit handle free
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_VOID*
* @retval AK_NULL
*/
T_VOID* NetTrans_Free(T_NET_TRANS* pTrans)
{
	AK_ASSERT_PTR(pTrans, "NetTrans_Free(): pTrans null", AK_NULL);

	if (FS_INVALID_HANDLE != pTrans->curRecvFile.fp)
	{
		Fwl_FileClose(pTrans->curRecvFile.fp);
		pTrans->curRecvFile.fp = FS_INVALID_HANDLE;
	}
	
	if (AK_INVALID_TASK != pTrans->recvtask)
	{
		AK_Terminate_Task(pTrans->recvtask);
		AK_Delete_Task(pTrans->recvtask);
		pTrans->recvtask = AK_INVALID_TASK;
			
		pTrans->pRecvStackAddr = Fwl_Free(pTrans->pRecvStackAddr);
	}

	if (FS_INVALID_HANDLE != pTrans->curSendFile.fp)
	{
		Fwl_FileClose(pTrans->curSendFile.fp);
		pTrans->curSendFile.fp = FS_INVALID_HANDLE;
	}

	if (AK_INVALID_TASK != pTrans->sendtask)
	{
		AK_Terminate_Task(pTrans->sendtask);
		AK_Delete_Task(pTrans->sendtask);
		pTrans->sendtask = AK_INVALID_TASK;
			
		pTrans->pSendStackAddr = Fwl_Free(pTrans->pSendStackAddr);
	}

	AK_Delete_Semaphore(pTrans->semaphore);

	return Fwl_Free(pTrans);
}


/**
* @brief create receive task
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
T_BOOL NetTrans_CreateRecvTask(T_NET_TRANS* pTrans)
{
	AK_ASSERT_PTR(pTrans, "NetTrans_CreateRecvTask(): pTrans null", AK_FALSE);
	AK_ASSERT_PTR(pTrans->pNetConn, "NetTrans_CreateRecvTask(): pTrans->pNetConn null", AK_FALSE);

	pTrans->pRecvStackAddr = Fwl_Malloc(NT_STACK_SIZE);
	AK_ASSERT_PTR(pTrans->pRecvStackAddr, "pTrans->pRecvStackAddr malloc error", AK_FALSE);
	memset(pTrans->pRecvStackAddr, 0, NT_STACK_SIZE);

	pTrans->recvtask = AK_Create_Task((T_VOID*)NetTrans_RecvThread, "recv", 1, 
		pTrans, pTrans->pRecvStackAddr, NT_STACK_SIZE, 110, 5, 
		AK_PREEMPT, AK_START);

	if (pTrans->recvtask < 0)
	{
		Fwl_Print(C2, M_NETWORK, "pTrans->recvtask create failed %d!", pTrans->recvtask);
		pTrans->recvtask = AK_INVALID_TASK;
		pTrans->pRecvStackAddr = Fwl_Free(pTrans->pRecvStackAddr);
		return AK_FALSE;
	}

	return AK_TRUE;
}


/**
* @brief create send task
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
static T_BOOL NetTrans_CreateSendTask(T_NET_TRANS* pTrans)
{
	AK_ASSERT_PTR(pTrans, "NetTrans_CreateSendTask(): pTrans null", AK_FALSE);
	AK_ASSERT_PTR(pTrans->pNetConn, "NetTrans_CreateSendTask(): pTrans->pNetConn null", AK_FALSE);
	
	pTrans->pSendStackAddr = Fwl_Malloc(NT_STACK_SIZE);
	AK_ASSERT_PTR(pTrans->pSendStackAddr, "pTrans->pSendStackAddr malloc error", AK_FALSE);
	memset(pTrans->pSendStackAddr, 0, NT_STACK_SIZE);

	pTrans->sendtask = AK_Create_Task((T_VOID*)NetTrans_SendThread, "send", 1, 
		pTrans, pTrans->pSendStackAddr, NT_STACK_SIZE, 110, 5, 
		AK_PREEMPT, AK_START);

	if (pTrans->sendtask < 0)
	{
		Fwl_Print(C2, M_NETWORK, "pTrans->sendtask create failed %d!", pTrans->sendtask);
		pTrans->sendtask = AK_INVALID_TASK;
		pTrans->pSendStackAddr = Fwl_Free(pTrans->pSendStackAddr);
		return AK_FALSE;
	}

	return AK_TRUE;
}

/**
* @brief try to send
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_NET_TRANS* pTrans:network transmit handle
* @return T_BOOL
* @retval 
*/
T_BOOL NetTrans_TrySend(T_NET_TRANS* pTrans)
{
	AK_ASSERT_PTR(pTrans, "NetTrans_TrySend(): pTrans null", AK_FALSE);

	pTrans->bSending = AK_TRUE;

	if (AK_INVALID_TASK != pTrans->sendtask)
	{
		return AK_TRUE;
	}

	return NetTrans_CreateSendTask(pTrans);
}


#endif
