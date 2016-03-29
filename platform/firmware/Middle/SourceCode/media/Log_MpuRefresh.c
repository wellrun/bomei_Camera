/**
 * @file Log_MpuRefresh.c
 * @brief MPU LCD AsynRefresh Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#if CI37XX_PLATFORM

#include "Anyka_Types.h"
#include "AKOS_Api.h"

#include "AKDefine.h"
#include "AKError.h"
#include "Eng_Debug.h"
#include "Fwl_osMalloc.h"
#include "AKSubThread.h"
#include "AKAppMgr.h"
#include "Lib_Media_Global.h"

#include "Log_MediaStruct.h"

#define M_MPU					"MPU"
#define VIDEO_OUT_BUF_NUM		3
#define YUV_SHOW_THREAD_PRIO	90
#define YUV_SHOW_THREAD_SLIC	2
#define YUV_SHOW_THREAD_STACK	4*1024


typedef struct tag_VidoeOutList{
	T_VIDEO_DECODE_OUT 			videoOut; //!< YUV Data's Buffer Address, Width, Height Etc.
	struct tag_VidoeOutList* 	next;

}T_VIDEO_OUT_LIST, *T_pVIDEO_OUT_LIST;

typedef struct tag_MpuRefresh{
	ISubThread 			*thread;		//!< Refresh Thread Instance
	T_hEventGroup		evtGroup;		//!< Event Group Handler
	
	T_pVIDEO_OUT_LIST	head;			//!< The Header of Refresh Queue
	T_pVIDEO_OUT_LIST	tail;			//!< The Tailer of Refresh Queue
	T_fSHOWFRAME_CB 	pShowFrameCB;	//!< Show A Frame Callback Function

}T_MPU_REFRESH, *T_pMPU_REFRESH;


static T_pMPU_REFRESH s_pMpuRefresh = AK_NULL;

static T_pVOID MpuRefr_FreeList(T_pVIDEO_OUT_LIST head)
{
	T_pVIDEO_OUT_LIST q, p;

	AK_ASSERT_PTR(head, "MPU:	head Is Invalid", AK_NULL);

	p = head->next;

	while(p != head)
	{
		q = p;
		p = p->next;
		Fwl_FreeTrace(q);
	}
	
	return Fwl_FreeTrace(head);
}

static T_pVOID MpuRefr_Destroy(T_pMPU_REFRESH pMpuRefresh)
{
	AK_ASSERT_PTR(pMpuRefresh, "MPU:	pMpuRefresh Is Invalid", AK_NULL);
	
	if (AK_IS_VALIDHANDLE(pMpuRefresh->evtGroup))
	{
		if (AK_SUCCESS != AK_Delete_Event_Group(pMpuRefresh->evtGroup))
			Fwl_Print(C2, M_MPU, "Deleted Event Group Failure");
	}	
		
	pMpuRefresh->head = MpuRefr_FreeList(pMpuRefresh->head);
	pMpuRefresh->tail = AK_NULL;
	pMpuRefresh = Fwl_FreeTrace(pMpuRefresh);
	
	Fwl_Print(C2, M_MPU, "Destroy MPU Refresh Thread");

	return AK_NULL;
}

static T_VOID MpuRefr_Handle(T_pVOID argv)
{
	T_U32 tmpEvt;
	T_VIDEO_DECODE_OUT* pVdo;
	T_pMPU_REFRESH pMpuRefr = argv;

	AK_ASSERT_PTR_VOID(pMpuRefr, "MPU:	pMpuRefr Parameter Is Invalid");

	while(AK_SUCCESS == AK_Retrieve_Events(pMpuRefr->evtGroup, EVT_MPU_REFRESH | EVT_THREAD_EXIT, AK_OR_CONSUME, &tmpEvt, AK_SUSPEND))
	{
		switch (tmpEvt)
		{
		case EVT_MPU_REFRESH:
			if (pMpuRefr->pShowFrameCB 
				&& pMpuRefr->head != pMpuRefr->tail)
			{	
				pVdo = &pMpuRefr->head->videoOut;
				pMpuRefr->pShowFrameCB(pVdo->m_pBuffer, pVdo->m_pBuffer_u, pVdo->m_pBuffer_v, 
								pVdo->m_uDispWidth, pVdo->m_uDispHeight, pVdo->m_uOriWidth, pVdo->m_uOriHeight);

				pMpuRefr->head->videoOut.m_pBuffer = AK_NULL;
				pMpuRefr->head = pMpuRefr->head->next;
			}
			break;

		case EVT_THREAD_EXIT:
			Fwl_Print(C3, M_MPU, "MPU Refresh Thread END");
			pMpuRefr->pShowFrameCB = AK_NULL;
			return;
			break;

		default:
			break;
		}
	}	
}


static T_BOOL MpuRefr_InitList(T_pVIDEO_OUT_LIST *head)
{
	T_U8 i;
	T_pVIDEO_OUT_LIST p;
	
	*head = p = Fwl_Malloc(sizeof(T_VIDEO_OUT_LIST));
	AK_ASSERT_PTR(p, "MPU:	Malloc List Head Failure", AK_FALSE);
	
	memset(p, 0, sizeof(T_VIDEO_OUT_LIST));
	
	for(i = 0; i < VIDEO_OUT_BUF_NUM-1; ++i)
	{
		p->next =  Fwl_Malloc(sizeof(T_VIDEO_OUT_LIST));
		AK_ASSERT_PTR(p->next, "MPU:	Malloc List p->next Failure", AK_FALSE);
		
		memset(p->next, 0, sizeof(T_VIDEO_OUT_LIST));
		
		p = p->next;
	}
	p->next = *head;
	return AK_TRUE;
}


static T_BOOL MpuRefr_CreateThread(T_pMPU_REFRESH pMpuRefresh)
{
    T_SUBTHREAD_INITPARAM   param;
    
    param.pcszName        = "YuvShowThread";
    param.byPriority      = YUV_SHOW_THREAD_PRIO;
    param.ulTimeSlice     = YUV_SHOW_THREAD_SLIC;
    param.ulStackSize     = YUV_SHOW_THREAD_STACK;
    param.wMainThreadCls  = AKAPP_CLSID_VIDEO;
    param.pUserData       = pMpuRefresh;
    param.fnEntry         = MpuRefr_Handle;
    param.fnAbort         = AK_NULL;	// MpuRefr_Destroy; Maybe Exec 2 Times, Error!

    if(AK_SUCCESS != CSubThread_New(&pMpuRefresh->thread, &param, AK_TRUE))
    {
		Fwl_Print(C2, M_MPU, "Create YuvShow Sub Thread Failure");
		return AK_FALSE;
    }
	
	Fwl_Print(C3, M_MPU, "Create YuvShow Sub Thread Success");
	
	// AK_RELEASEIF(pMpuRefresh->thread); 
	return AK_TRUE;
}

const T_pVOID MpuRefr_IsInit(T_VOID)
{
	return s_pMpuRefresh;
}

T_VOID MpuRefr_SetShowFrameCB(T_fSHOWFRAME_CB 	ShowCB)
{
	AK_ASSERT_PTR_VOID(s_pMpuRefresh, "MPU:	s_pMpuRefresh Is Invalid");
	s_pMpuRefresh->pShowFrameCB = ShowCB;
}

// Called By Video Decode Thread
T_BOOL MpuRefr_Free(T_VOID)
{
	T_U8 i = 0;
	ISubThread *thread = s_pMpuRefresh->thread;
		
	AK_Set_Events(s_pMpuRefresh->evtGroup, EVT_THREAD_EXIT, AK_OR);
		
	AK_Sleep(2);
		

	Fwl_Print(C3, M_MPU, "Exit Thread Waited Times: %d", i);
	
	s_pMpuRefresh = MpuRefr_Destroy(s_pMpuRefresh);

	ISubThread_Terminate(thread);
	
	return AK_TRUE;
}

T_BOOL MpuRefr_Init(T_VOID)
{
	if (s_pMpuRefresh)
		s_pMpuRefresh = MpuRefr_Destroy(s_pMpuRefresh);

	Fwl_Print(C4, M_MPU, "Init YUV Show Sub Thread ...");
	
	s_pMpuRefresh = Fwl_Malloc(sizeof(T_MPU_REFRESH));
	AK_ASSERT_PTR(s_pMpuRefresh, "MPU:	s_pMpuRefresh Malloc Failure", AK_FALSE);
	memset(s_pMpuRefresh, 0, sizeof(T_MPU_REFRESH));

	// Initialized List
	if (!MpuRefr_InitList(&s_pMpuRefresh->head))
	{
		Fwl_Print(C2, M_MPU, "MpuRefr_InitList() Failure");
		s_pMpuRefresh = Fwl_FreeTrace(s_pMpuRefresh);
		return AK_FALSE;
	}
	s_pMpuRefresh->tail = s_pMpuRefresh->head;

	s_pMpuRefresh->evtGroup = AK_Create_Event_Group();
	if (AK_IS_INVALIDHANDLE(s_pMpuRefresh->evtGroup))
		Fwl_Print(C2, M_MPU, "AK_Create_Event_Group() Failure");
	
	return MpuRefr_CreateThread(s_pMpuRefresh);
#if 0
	// Initialize Thread Parameter
	s_pMpuRefresh->thread = Fwl_Malloc(sizeof(T_GENRL_THREAD));
	if (!s_pMpuRefresh->thread)
	{
		s_pMpuRefresh->head = MpuRefr_FreeList(s_pMpuRefresh->head);
		s_pMpuRefresh->tail = AK_NULL;
		s_pMpuRefresh = Fwl_FreeTrace(s_pMpuRefresh);
		return AK_FALSE;
	}
	memset(s_pMpuRefresh->thread, 0, sizeof(T_GENRL_THREAD));

	s_pMpuRefresh->thread->parm = GenrlThread_Init(MpuRefr_Handle, "Mpu_Handle", 1, s_pMpuRefresh, 100, 2);
	if (!s_pMpuRefresh->thread->parm)
	{
		AK_DEBUG_OUTPUT("MPU:	Malloc Audio Thread Parameter Failure.\n");
		s_pMpuRefresh->thread = Fwl_FreeTrace(s_pMpuRefresh->thread);
		s_pMpuRefresh->head = MpuRefr_FreeList(s_pMpuRefresh->head);
		s_pMpuRefresh->tail = AK_NULL;
		s_pMpuRefresh = Fwl_FreeTrace(s_pMpuRefresh);
		return AK_FALSE;
	}

	// Create Thread
	if (!GenrlThread_Create(s_pMpuRefresh->thread))
	{
		// Mpu Refresh Thread Failure
		AK_DEBUG_OUTPUT("MPU:	Create Audio Thread Failure.\n");
	
		Fwl_FreeTrace(s_pMpuRefresh->thread->parm->stackAddr);
		Fwl_FreeTrace(s_pMpuRefresh->thread->parm);
		
		s_pMpuRefresh->thread = Fwl_FreeTrace(s_pMpuRefresh->thread);
		s_pMpuRefresh->head = MpuRefr_FreeList(s_pMpuRefresh->head);
		s_pMpuRefresh->tail = AK_NULL;
		s_pMpuRefresh = Fwl_FreeTrace(s_pMpuRefresh);
		return AK_FALSE;
	}
	
	// AK_DEBUG_OUTPUT("MPU:	Initialized MPU Refresh Success.\n");
	// return AK_TRUE;
#endif
}


T_VOID MpuRefr_Refresh(T_VIDEO_DECODE_OUT *decOut)
{
	AK_ASSERT_PTR_VOID(s_pMpuRefresh, "MPU:	s_pMpuRefresh Is NOT Initialized");
	
	if (s_pMpuRefresh->tail->next != s_pMpuRefresh->head)
	{
		memcpy(&s_pMpuRefresh->tail->videoOut, decOut, sizeof(T_VIDEO_DECODE_OUT));	
		s_pMpuRefresh->tail = s_pMpuRefresh->tail->next;
	}
	else
	{
		AK_Sleep(1);
	}
	
	AK_Set_Events(s_pMpuRefresh->evtGroup, EVT_MPU_REFRESH, AK_OR);
}

T_VOID MpuRefr_Stop(T_VOID)
{
	T_pVIDEO_OUT_LIST p;
	
	if (AK_NULL == s_pMpuRefresh)
		return;
	
	s_pMpuRefresh->pShowFrameCB = AK_NULL;
	p = s_pMpuRefresh->head;
	
	do
	{
		memset(s_pMpuRefresh->head, 0, sizeof(T_VIDEO_DECODE_OUT));
		s_pMpuRefresh->head = s_pMpuRefresh->head->next;
	}while (s_pMpuRefresh->head->next != p);

	s_pMpuRefresh->tail = s_pMpuRefresh->head;
}

T_BOOL MpuRefr_CheckFrameFinish(const T_pDATA pBuf)
{
	T_pVIDEO_OUT_LIST p;
	
	if (!s_pMpuRefresh)
		return AK_TRUE;
	
	p = s_pMpuRefresh->head;
	
	while(p != s_pMpuRefresh->tail)
	{
		if (p->videoOut.m_pBuffer == pBuf)
			return AK_FALSE;
		
		p = p->next;
	}

	return AK_TRUE;
}

#endif	// CI37XX_PLATFORM
/*
 * End of File
 */
