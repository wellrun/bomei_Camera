/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: AKFrameStream.c
 * Function:this is a simple stream component,define normal stream operation 
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/

#include "AKFrameStream.h"
#include "eng_debug.h"
#include "akerror.h"
#include "fwl_osmalloc.h"
#include "akcomponent.h"
#include "Fwl_display.h"

#include "fwl_oscom.h"
#include "Akos_api.h"



//#define STRM_DEBUG_TRACE
#ifdef OS_ANYKA
#define STRM_DEBUG_OUTPUT  AkDebugOutput
#else
#define STRM_DEBUG_OUTPUT  printf
#endif

extern T_U8* Dev_GetFrameBuf (T_RECT* pstRectLay, T_U8* ColorSpace);

/*=======================DEFINE MACRO   ==========================*/
//#define STRM_READ_PROTECT
//#define STRM_WRITE_PROTECT

//#################################################################################
typedef T_hSemaphore     T_STRM_SEM;
typedef T_hSemaphore     T_STRM_MUTEX;
typedef struct tagCFrmStrm
{
   T_U32      m_Ref;
   IFrmStrm   m_myIFrmStrm;
   //Add your data here...
   //=====================
   T_STRM_INIT_PARAM  initParam;   // constructor info
   _VOLATILE T_U32    len;         // ring buffer valid length
   _VOLATILE T_S32    head;        // ring buffer's  head
   _VOLATILE T_S32    tail;        // ring buffer's tail
   T_STRM_SEM         nonEmptySem; // sem    for read
   T_STRM_SEM         nonFullSem;  // sem    for write
   T_STRM_MUTEX       strmMutex;   // mutex for update Member
   T_FRM             *frameArray; // all frames store space
#ifdef STRM_READ_PROTECT
   T_STRM_MUTEX       readMutex;  //   mutex for wait read success         
#endif
#ifdef STRM_WRITE_PROTECT
   T_STRM_MUTEX       writeMutex;  //   mutex for wait read success             
#endif  
   //======================
}CFrmStrm;

/*=======================DEFINE MACRO   ==========================*/
#define STRM_MUTEX_INIT(mutex)        ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define STRM_MUTEX_IS_ERR(mutex)      (((mutex) <= 0) && ((mutex) > -100))
#define STRM_MUTEX_IS_LOCKED(mutex)   (0 == AK_Get_SemVal(mutex))
#define STRM_MUTEX_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define STRM_MUTEX_TRY_LOCK(mutex)    (0 < (AK_Try_Obtain_Semaphore((mutex), AK_SUSPEND)))
#define STRM_MUTEX_UNLOCK(mutex)      AK_Release_Semaphore(mutex)
#define STRM_MUTEX_DEINIT(mutex)      {\
	if (!(STRM_MUTEX_IS_ERR(mutex)))\
	{\
		AK_Delete_Semaphore(mutex);\
		(mutex) = 0;\
	}\
}

#define STRM_SEM_INIT(sem,cnt)       ((sem) = AK_Create_Semaphore((cnt), AK_PRIORITY))
#define STRM_SEM_IS_ERR(sem)         (((sem) <= 0) && ((sem) > -100))
#define STRM_SEM_IS_EMPTY(sem)     (0 == AK_Get_SemVal(sem))
#define STRM_SEM_WAIT(sem)            AK_Obtain_Semaphore((sem), AK_SUSPEND)
#define STRM_SEM_TRY_WAIT(sem)        (0 < (AK_Try_Obtain_Semaphore((sem), AK_SUSPEND)))
#define STRM_SEM_POST(sem)            AK_Release_Semaphore((sem))
#define STRM_SEM_DEINIT(sem)      {\
	if (!(STRM_SEM_IS_ERR(sem)))\
	{\
		AK_Delete_Semaphore(sem);\
		(sem) = 0;\
	}\
}


//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================
/*
 * @brief   release  one  stream 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream instance count
 */
static T_U32 CFrmStrm_AddRef(IFrmStrm *pIFrmStrm);

/*
 * @brief   release  one  stream 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @return	resulst AK_SUCCESS--release Ok, else  fail
 */
static T_U32 CFrmStrm_Release(IFrmStrm *pIFrmStrm);


//Add your function declaration here...
static T_S32   CFrmStrm_Init(IFrmStrm *pIFrmStrm,T_STRM_INIT_PARAM InitParam);

/*
 * @brief   put one frame to  stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] putProcFun: user's callback function for process frame 
 * @param[in] srcFrmData: source data (put to stream) 
 * @param[in] param: user's callback param for process 
 * @return	resulst AK_SUCCESS--process Ok, else  fail
 */
static T_S32   CFrmStrm_PutFrameByProc(IFrmStrm *pIFrmStrm, T_STEM_PUT_PROC putProcFun, T_FRM_DATA *srcFrmData, T_pVOID param);

/*
 * @brief   get one frame from  stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] getProcFun: user's callback function for process frame 
 * @param[in] param: user's callback param for process 
 * @return	resulst AK_SUCCESS--process Ok, else  fail
 */
static T_S32   CFrmStrm_GetFrameByProc(IFrmStrm *pIFrmStrm, T_STEM_GET_PROC procFun, T_pVOID param);


/*
 * @brief   get  one  stream cureent size(valid node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_U16   CFrmStrm_Size(IFrmStrm *pIFrmStrm);

/*
 * @brief   get  one  stream capcity(all node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_U16   CFrmStrm_GetCapcity(IFrmStrm *pIFrmStrm);

/*
 * @brief   dump  one  stream info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	AK_SUCCESS: dump Ok, else fail
 */
static T_S32   CFrmStrm_Dump(IFrmStrm *pIFrmStrm);

/*
 * @brief   get  one  stream translate type(all node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	frame trans type, data or address
 */
static T_U16   CFrmStrm_GetTransType(IFrmStrm *pIFrmStrm);

/*
 * @brief   touch(update status) one frame from  stream whitch cound not be write
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] pFrmBufferAddr: frame's data buffer address
 * @return	resulst AK_SUCCESS--touch Ok, else  fail
 */
static T_S32   CFrmStrm_TouchFrmByAddr(IFrmStrm *pIFrmStrm, T_U8 *pFrmBufferAddr);

/*
 * @brief   set  one  stream all frame info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @param[in] pInfo: new frame info
 * @return	resulst AK_SUCCESS--touch Ok, else  fail
 */
static T_S32   CFrmStrm_SetFrmInfo(IFrmStrm *pIFrmStrm,T_FRM_INFO *pInfo);



/*
 * @brief   start  one  stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_S32   CFrmStrm_Start(IFrmStrm *pIFrmStrm);

/*
 * @brief   stop  one  stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_S32   CFrmStrm_Stop(IFrmStrm *pIFrmStrm);


//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IFrmStrm) g_IFrmStrmFuncs =
{
    CFrmStrm_AddRef,
    CFrmStrm_Release,
    CFrmStrm_Init,
    CFrmStrm_PutFrameByProc,
    CFrmStrm_GetFrameByProc,
    CFrmStrm_Size,
    CFrmStrm_GetCapcity,
    CFrmStrm_Dump,
    CFrmStrm_GetTransType,
    CFrmStrm_TouchFrmByAddr,
    CFrmStrm_SetFrmInfo,
    CFrmStrm_Start,
    CFrmStrm_Stop
};

//////////////////////////////////////////////////////////////////////////

/*
 * @brief   check frame status is writeable
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @param[in] status: frame status
 * @return	AK_TRUE--enbale write, else  disable
 */
static __inline T_BOOL CFrmStrm_FrmIsWriteEnable(CFrmStrm *me, T_FRM_STATUS status);


/*
 * @brief   check frame status is readable
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @param[in] status: frame status
 * @return	AK_TRUE--enbale read, else  disable
 */
static __inline T_BOOL CFrmStrm_FrmIsReadEnable(CFrmStrm *me, T_FRM_STATUS status);

/*
 * @brief   check frame status is writeable
 * @author WangXi
 * @date 2011-10-25
 * @param[in/out] pInitParam: stream handle
 * @param[in] status: frame status
 * @return	resulst AK_SUCCESS--paramete is valid, else invalid parameter
 */
static __inline T_S32  CFrmStrm_ParamCheck(T_STRM_INIT_PARAM *pInitParam);

/*
 * @brief   enable stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
static __inline T_S32  CFrmStrm_Enable(CFrmStrm *me, T_BOOL isEnable);

//////////////////////////////////////////////////////////////////////////

/*
 * @brief   check frame info valid 
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pFrmInfo: frame info
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
static T_S32 CFrmStrm_FrmInfoCheck(T_FRM_INFO *pFrmInfo)
{
	if (AK_NULL != pFrmInfo)
	{
		if (0 == pFrmInfo->size) // buffer size is invalid
		{
			return AK_EBADPARAM;
		}
	}
	else
	{
		return AK_EBADPARAM;
	}

	return AK_SUCCESS;
}


/*
 * @brief   check frame status is readable
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @param[in] status: frame status
 * @return	AK_TRUE--enbale read, else  disable
 */
static __inline T_BOOL CFrmStrm_FrmIsReadEnable(CFrmStrm *me, T_FRM_STATUS status)
{
	if (me->initParam.enableReadFromOld)
	{
	    // if enable read old frame, if not new frame ,will use preframe instead
		return ((eFRM_STATUS_WRITE_OK == (status))\
			   || (eFRM_STATUS_READ_OK == (status)));
	}
	else
	{
         // if disable read old frame, if not new frame, will use empty frame instead
        return (eFRM_STATUS_WRITE_OK == (status));
	}
}



/*
 * @brief   check frame status is writeable
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @param[in] status: frame status
 * @return	AK_TRUE--enbale write, else  disable
 */
static __inline T_BOOL CFrmStrm_FrmIsWriteEnable(CFrmStrm *me, T_FRM_STATUS status)
{
	if (me->initParam.enableWriteToOld)
	{
	    // if enable write old frame, if not new frame ,will use preframe instead
		return ((eFRM_STATUS_NULL == (status))\
			  || (eFRM_STATUS_WRITE_DIRTY == (status))\
			  || (eFRM_STATUS_READ_OK == (status))\
			  || (eFRM_STATUS_WRITE_OK == (status)));
	}
	
	else
	{  
        // if disable write old frame, if not new frame, will use empty frame instead
        return ((eFRM_STATUS_NULL == (status))\
			  || (eFRM_STATUS_WRITE_DIRTY == (status))\
			  || (eFRM_STATUS_READ_OK == (status)));
	}
}



/*
 * @brief   enable stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
static __inline T_S32 CFrmStrm_Enable(CFrmStrm *me, T_BOOL isEnable)
{
	T_S32 i = 0;
	T_FRM_STATUS status = eFRM_STATUS_NULL;
	
    // destory non empty semaphore for write
    STRM_SEM_DEINIT(me->nonEmptySem);
    
    // destory  non full semaphore as empty cnt for read
    STRM_SEM_DEINIT(me->nonFullSem);

#ifdef STRM_WRITE_PROTECT
    // destory write protect lock
    STRM_MUTEX_DEINIT(me->writeMutex);
#endif

#ifdef STRM_READ_PROTECT
    // destory read protect lock
    STRM_MUTEX_DEINIT(me->readMutex);
#endif	

    //clear stream node status
    if (AK_NULL != me->frameArray)
    {
        if (isEnable)
        {
            status = eFRM_STATUS_NULL;
        }
        else
        {
            status = eFRM_STATUS_WRITE_DIRTY;
        }
        
        for (i=0;i<(T_S32)me->initParam.len;i++)
        {
            me->frameArray[i].status = status;
        }
    }
    
    // clear  stream  w/r pos and len
    me->head = 0;
    me->tail = 0;
    me->len  = 0;

    //rebulid stream process protect info
    if (isEnable)
    {
        // create non empty semaphore for write
        STRM_SEM_INIT(me->nonEmptySem, 0);
        if (STRM_SEM_IS_ERR(me->nonEmptySem))
        {
            Fwl_Print(C1, M_AKFRAME,  "STRM:Err sem\n");
            return AK_INVALID_SEMAPHORE;
        }
        
        // create  non full semaphore as empty cnt for read
        STRM_SEM_INIT(me->nonFullSem, me->initParam.len);
        if (STRM_SEM_IS_ERR(me->nonFullSem))
        {
            Fwl_Print(C1, M_AKFRAME,  "STRM:Err sem\n");
            return AK_INVALID_SEMAPHORE;
        }

#ifdef STRM_READ_PROTECT
        // create read protect lock
        STRM_MUTEX_INIT(me->readMutex);
        if (STRM_MUTEX_IS_ERR(me->readMutex))
        {
            Fwl_Print(C1, M_AKFRAME,  "STRM:Err Mutex\n");
            return AK_INVALID_SEMAPHORE;
        }
#endif

#ifdef STRM_WRITE_PROTECT
        // create write protect lock
        STRM_MUTEX_INIT(me->writeMutex);
        if (STRM_MUTEX_IS_ERR(me->writeMutex))
        {
            Fwl_Print(C1, M_AKFRAME,  "STRM:Err Mutex\n");
            return AK_INVALID_SEMAPHORE;
        }
#endif	
    }
	
    return AK_SUCCESS;   
}

/*
 * @brief   check frame status is writeable
 * @author WangXi
 * @date 2011-10-25
 * @param[in/out] pInitParam: stream handle
 * @param[in] status: frame status
 * @return	resulst AK_SUCCESS--paramete is valid, else invalid parameter
 */
static __inline T_S32 CFrmStrm_ParamCheck(T_STRM_INIT_PARAM *pInitParam)
{
	if (AK_NULL == pInitParam)
	{
        Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
		return AK_EBADPARAM;
	}

	if (0 == pInitParam->len) // stream length is invalid 
	{
        Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
		return AK_EBADPARAM;
	}
	
    // stream proc mode is invalid
	if ((eSTRM_BLK_TYPE_BLK   != pInitParam->getProcMode) &&\
		(eSTRM_BLK_TYPE_NOBLK != pInitParam->getProcMode)) 
	{
        Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
		return AK_EBADPARAM;
	}

    // stream proc mode is invalid
	if ((eSTRM_BLK_TYPE_BLK   != pInitParam->putProcMode) &&\
	    (eSTRM_BLK_TYPE_NOBLK != pInitParam->putProcMode)) 
	{
        Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
		return AK_EBADPARAM;
	}


	if (eSTRM_TYPE_DATA == pInitParam->transMode
		|| eSTRM_TYPE_DISP == pInitParam->transMode) // operator data  is Inner Malloc
	{
		if (AK_IS_FAILURE(CFrmStrm_FrmInfoCheck(&(pInitParam->frameInfo))))
		{
            Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
			return AK_EBADPARAM;
		}
	}
	else if (eSTRM_TYPE_ADDR == pInitParam->transMode)
	{
			pInitParam->len = 1;// if trans address  quenue only need one buffer
	}
	else
	{
        Fwl_Print(C2, M_AKFRAME,  "STRM:Err param @%d\n", __LINE__);
		return AK_EBADPARAM;
	}
	
	return AK_SUCCESS;
}


//==========================================================
/*
 * @brief   create one  stream by init parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @param[in] initParam:  init parameter 
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
static T_S32 CFrmStrm_Constructor(CFrmStrm *pMe, T_STRM_INIT_PARAM initParam)
{
    //Simple Constructor, Add your code here...
    T_S32 i = 0;
	
    memcpy(&(pMe->initParam), &initParam, sizeof(T_STRM_INIT_PARAM)); // save  init parm

	// check param if valid
	if (AK_IS_FAILURE(CFrmStrm_ParamCheck(&(pMe->initParam))))
	{
        Fwl_Print(C1, M_AKFRAME,  "STRM:Err param\n");
		return AK_EBADPARAM;
	}

	// create mutex for update member var
    STRM_MUTEX_INIT(pMe->strmMutex);
    if (STRM_MUTEX_IS_ERR(pMe->strmMutex))
    {
        Fwl_Print(C1, M_AKFRAME,  "STRM:Err mutex\n");
		return AK_INVALID_SEMAPHORE;
	}

    //create array  for store the  frame data
	pMe->frameArray = (T_FRM *)Fwl_Malloc((pMe->initParam.len) * sizeof(T_FRM));
    if (AK_NULL == pMe->frameArray)
    {
       Fwl_Print(C1, M_AKFRAME,  "STRM:No Enough Mem\n");
       return AK_ENOMEMORY;
    }

	// init array for default data 
	for (i=0; i<(T_S32)pMe->initParam.len;i++)
	{
    	// if trans by addr, this pointer need put byout side 
		pMe->frameArray[i].pData  = AK_NULL;
        pMe->frameArray[i].status = eFRM_STATUS_NULL;
	}

	//if use  inner space for store data
	if (eSTRM_TYPE_DATA == pMe->initParam.transMode
		|| eSTRM_TYPE_DISP == pMe->initParam.transMode) 
	{
		T_RECT 	rect;
		T_U8 	colorSpace;
		
	    // operator data  is Inner Malloc
		for (i = (T_S32)pMe->initParam.len-1; i >= 0; --i)
		{
			//create data info
			pMe->frameArray[i].pData= (T_FRM_DATA *)Fwl_Malloc(sizeof(T_FRM_DATA));
			if (AK_NULL == pMe->frameArray[i].pData)
			{
                Fwl_Print(C1, M_AKFRAME,  "STRM:Malloc err\n");
				return AK_ENOMEMORY;
			}
			
			//copy  frame init info to buffer self's space
			memcpy(&(pMe->frameArray[i].pData->info),\
				   &(pMe->initParam.frameInfo), sizeof(T_FRM_INFO));

			if (eSTRM_TYPE_DATA == pMe->initParam.transMode)
			{				
				//use buffer info to create the frame buffer
				pMe->frameArray[i].pData->pBuffer = (T_U8*)Fwl_Malloc(pMe->frameArray[i].pData->info.size);
			}	
			else //eSTRM_TYPE_DISP
			{
				if (0 == i && DISPLAY_BUF_SIZE >= pMe->frameArray[0].pData->info.size)
				{
					pMe->frameArray[i].pData->pBuffer = Dev_GetFrameBuf(&rect, &colorSpace);
				}
				else
				{
					//use buffer info to create the frame buffer
					pMe->frameArray[i].pData->pBuffer = (T_U8*)Fwl_Malloc(pMe->frameArray[i].pData->info.size);
				}
			}
			
            // not enough memory, return error
			if (AK_NULL == pMe->frameArray[i].pData->pBuffer)
			{
                Fwl_Print(C1, M_AKFRAME,  "STRM[%d/%d]:No Enough Mem(Need:%d,Remain:%d)\n", 
                    i+1, pMe->initParam.len, pMe->frameArray[i].pData->info.size,
                    Fwl_GetRemainRamSize());
				return AK_ENOMEMORY;
			}

			memset(pMe->frameArray[i].pData->pBuffer, 0, pMe->frameArray[i].pData->info.size);
		}
	}

    return CFrmStrm_Enable(pMe, AK_TRUE);
    
}


/*
 * @brief   destory one  stream 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @return	resulst AK_SUCCESS--destory Ok, else  fail
 */
static T_S32 CFrmStrm_Destructor(CFrmStrm *pMe)
{
	T_S32 i = 0;
	

    STRM_MUTEX_LOCK(pMe->strmMutex);  // entry protect zone
	
	//if use  inner space for store data
	if (eSTRM_TYPE_DATA == pMe->initParam.transMode
		|| eSTRM_TYPE_DISP == pMe->initParam.transMode) // operator data  is Inner Malloc
	{
		for (i=0; i<(T_S32)pMe->initParam.len; i++)
		{
			if (eSTRM_TYPE_DATA == pMe->initParam.transMode)
			{
				//free the frame buffer
				if (AK_NULL != pMe->frameArray[i].pData->pBuffer)
				{					
					Fwl_Free(pMe->frameArray[i].pData->pBuffer);					
					pMe->frameArray[i].pData->pBuffer = AK_NULL;
				}
			}
			else // eSTRM_TYPE_DISP
			{
				//free the frame buffer
				if (AK_NULL != pMe->frameArray[i].pData->pBuffer)
				{
					if (0 == i && DISPLAY_BUF_SIZE >= pMe->frameArray[0].pData->info.size)
					{
						;	// TV-OUT BUFFER, NO FREE
					}
					else
					{
						Fwl_Free(pMe->frameArray[i].pData->pBuffer);
					}
					
					pMe->frameArray[i].pData->pBuffer = AK_NULL;
				}
			}

			//free  data info
			if (AK_NULL != pMe->frameArray[i].pData)
			{
				Fwl_Free(pMe->frameArray[i].pData);
				pMe->frameArray[i].pData = AK_NULL;
			}
		}
	}
	
    CFrmStrm_Enable(pMe, AK_FALSE);
    
	//free array	for store the  frame data
	if (AK_NULL != pMe->frameArray)
	{
		Fwl_Free(pMe->frameArray);
		pMe->frameArray = AK_NULL;
	}
	
	STRM_MUTEX_UNLOCK(pMe->strmMutex);// exit  protect zone

	// destory mutex for update member var
	STRM_MUTEX_DEINIT(pMe->strmMutex);
	
	
	return AK_SUCCESS;
}


/*
 * @brief   create one  stream instance by init parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[out] ppi: stream handle
 * @param[in] initParam:  init parameter 
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
T_S32 CFrmStrm_New(IFrmStrm **ppi, T_STRM_INIT_PARAM initParam)
{
    T_S32 nErr = AK_SUCCESS;
    CFrmStrm *pNew = AK_NULL;

    if (AK_NULL == ppi)
    {
        Fwl_Print(C1, M_AKFRAME,  "STRM:Err param\n");
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;
  
    do 
    {
        // create stream handle
        pNew = AK_MALLOCRECORD(CFrmStrm);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        memset(pNew, 0, sizeof(CFrmStrm));
		
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_myIFrmStrm), &g_IFrmStrmFuncs);
        pNew->m_myIFrmStrm.pData = (T_VOID*)pNew;

        nErr = CFrmStrm_Constructor(pNew, initParam);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CFrmStrm_Release(&pNew->m_myIFrmStrm);
            pNew = AK_NULL;
            break;
        }

       *ppi = (IFrmStrm *)&pNew->m_myIFrmStrm;  
    } while(AK_FALSE);

    return nErr;
}

/*
 * @brief   release  one  stream 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream instance count
 */
static T_U32 CFrmStrm_AddRef(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;

    return ++pMe->m_Ref;
}


/*
 * @brief   release  one  stream 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] me: stream handle
 * @return	resulst AK_SUCCESS--release Ok, else  fail
 */
static T_U32 CFrmStrm_Release(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CFrmStrm_Destructor(pMe);
        Fwl_Free(pMe);        
		return 0;
    }

    return pMe->m_Ref;
}


//=============================================================================================
static T_S32   CFrmStrm_Init(IFrmStrm *pIFrmStrm,T_STRM_INIT_PARAM InitParam)
{
    //CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    

    return AK_SUCCESS;   
}


/*
 * @brief   put one frame to  stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] putProcFun: user's callback function for process frame 
 * @param[in] srcFrmData: source data (put to stream) 
 * @param[in] param: user's callback param for process 
 * @return	resulst AK_SUCCESS--process Ok, else  fail
 */
static T_S32   CFrmStrm_PutFrameByProc(IFrmStrm *pIFrmStrm, T_STEM_PUT_PROC putProcFun, T_FRM_DATA *srcFrmData, T_pVOID param)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    T_S32 lRet = AK_EFAILED;
	T_FRM *pCurFrm = AK_NULL;
	T_FRM_DATA *pFrmData = AK_NULL;
	T_S32  writePos = -1;
	T_BOOL everyBufIsOnlyWrited = AK_FALSE;
	

	//if put process is not block
	if (eSTRM_BLK_TYPE_NOBLK == pMe->initParam.putProcMode)
	{
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "<Pa_%d>\n",pMe->tail);
        }
#endif
        if (!STRM_SEM_TRY_WAIT(pMe->nonFullSem)) //not any resource to write
        {
            if (pMe->initParam.enableWriteToOld)
            {
                everyBufIsOnlyWrited = AK_TRUE;
            }
            else
            {
                return AK_EFAILED;
            }
		}
	}
	else
	{
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "<pa_%d>\n",pMe->tail);
        }
#endif
		STRM_SEM_WAIT(pMe->nonFullSem); // wait until exist nonfull resource
	}
	
    STRM_MUTEX_LOCK(pMe->strmMutex);  // entry protect zone

    if (AK_NULL != pMe->frameArray)
    {
    	// get whitch postion will be write
		if (pMe->len < pMe->initParam.len)
		{
			writePos = pMe->tail;// use new frame
		}
		else if (everyBufIsOnlyWrited)
		{
			if (pMe->initParam.enableWriteToOld)
			{
				//use  old frame
				writePos = (pMe->tail + pMe->initParam.len - 1) % pMe->initParam.len;
			}
		
			else
			{
				//use  empty  frame
				//writePos = -1;//(defalut writePos is -1)
			}
		}
		else
		{
			//use  empty  frame
			//writePos = -1;//(defalut writePos is -1)
		}

#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "<Pb_%d>\n",writePos);
        }
#endif
		//get which frame will be  write
		if ((writePos >= 0) && (writePos < (T_S32)pMe->initParam.len))
		{
			pCurFrm = &(pMe->frameArray[writePos]);//use  new or old  frame

			// if frame status is enable write 
			if (CFrmStrm_FrmIsWriteEnable(pMe, pCurFrm->status))
			{
				// if tans mode is addres just assign the source data's pointer
				if (eSTRM_TYPE_ADDR == pMe->initParam.transMode)
				{
				   pCurFrm->pData = srcFrmData;
				}

				if (!everyBufIsOnlyWrited)// not old frame
				{
					pMe->tail = (pMe->tail + 1) % pMe->initParam.len;
					pMe->len++;
				}
			
				pCurFrm->status = eFRM_STATUS_WRITE_ING;
			}
			else
			{
			    //use  empty  frame
                pCurFrm = AK_NULL;
			}
		}
		else
		{
		    //use  empty  frame
			//pCurFrm = AK_NULL;//(defalut pCurFrm is  NULL)
		}

		lRet = AK_SUCCESS;
	}

	STRM_MUTEX_UNLOCK(pMe->strmMutex); // exit protect zone

    //decide how to  call user fun  
	if (AK_IS_SUCCESS(lRet))
	{
	    //decide  the user  fun's  param
		if (AK_NULL != pCurFrm)
		{
			pFrmData = pCurFrm->pData;//use new  or new  frame
		}
		else
		{
			if (eSTRM_TYPE_ADDR == pMe->initParam.transMode)
			{
			   pFrmData = srcFrmData;//use  src frame
			}
			else
			{
               pFrmData = AK_NULL;// use empty  frame
			}
		}
		
	   // call user 's callback
	   if (AK_NULL != putProcFun)
	   {
		   lRet = putProcFun(pFrmData, srcFrmData,param);
	   }
	   else
	   {
		   lRet = AK_SUCCESS;
	   }

        //update the  status  (new or old  frame)
		if (AK_NULL != pCurFrm)
		{
			if (AK_IS_SUCCESS(lRet)) 
			{
			     // this frame can be process again
				pCurFrm->status = eFRM_STATUS_WRITE_OK;
			}
			else // set the write  failed frame status is dirty
			{
				pCurFrm->status = eFRM_STATUS_WRITE_DIRTY;
			}  

			if (!everyBufIsOnlyWrited)
			{
#ifdef STRM_DEBUG_TRACE
                if (pMe->initParam.enableDebugMode)
                {
                    Fwl_Print(C3, M_AKFRAME,  "<Pc_%d>\n", writePos);
                }
#endif
				STRM_SEM_POST(pMe->nonEmptySem);// create a nonEmpty semphore for get
			}
		}
	}

	
    return (AK_NULL != pCurFrm) ? AK_SUCCESS : AK_EFAILED;   
}



/*
 * @brief   get one frame from  stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] getProcFun: user's callback function for process frame 
 * @param[in] param: user's callback param for process 
 * @return	resulst AK_SUCCESS--process Ok, else  fail
 */
static T_S32 CFrmStrm_GetFrameByProc(IFrmStrm *pIFrmStrm, T_STEM_GET_PROC getProcFun,T_pVOID param)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
	T_FRM *pCurFrm = AK_NULL;
	T_FRM_DATA *pFrmData = AK_NULL;
	T_S32  readPos = -1;
    T_S32  lRet = AK_EFAILED;
	T_BOOL everyBufIsOnlyReaded = AK_FALSE;

	if (eSTRM_BLK_TYPE_NOBLK == pMe->initParam.getProcMode)
	{
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "[Ga_%d]\n",pMe->head);
        }
#endif
        if (!STRM_SEM_TRY_WAIT(pMe->nonEmptySem)) //not any  resource for  read 
        {
            if (pMe->initParam.enableReadFromOld)
            {
                everyBufIsOnlyReaded = AK_TRUE;
            }
            else
            {
                return AK_EFAILED;
            }
		}
	}
	else
	{
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "[ga_%d]\n",pMe->head);
        }
#endif
		STRM_SEM_WAIT(pMe->nonEmptySem);  // wait  until  resource  can be read 
	}

#ifdef STRM_DEBUG_TRACE
    if (pMe->initParam.enableDebugMode)
    {
        Fwl_Print(C3, M_AKFRAME,  "[Gb_%d]\n",pMe->head);
    }
#endif

    STRM_MUTEX_LOCK(pMe->strmMutex);  // entry protect zone


	if (AK_NULL != pMe->frameArray)
	{
    	// get whitch postion will be read
		if (pMe->len > 0)
		{
			readPos = pMe->head;// use new frame
		}
		else if (everyBufIsOnlyReaded)
		{
			if (pMe->initParam.enableReadFromOld)
			{
				//use  old frame
				readPos = (pMe->head + pMe->initParam.len - 1) % pMe->initParam.len;
			}
			else
			{
				//use  empty  frame
				//readPos = -1;//(defalut readPos is -1)
			}
		}
		else
		{
			//use  empty  frame
			//readPos = -1;//(defalut readPos is -1)
		}
		
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "[Gc_%d]\n",readPos);
        }
#endif
		//get which frame will be  read
		if ((readPos >= 0) && (readPos < (T_S32)pMe->initParam.len))
		{
			pCurFrm = &(pMe->frameArray[readPos]);//use	new or old	frame
		
			// if frame status is enable write 
			if (CFrmStrm_FrmIsReadEnable(pMe, pCurFrm->status))
			{
			    //If all buffer is readed but not write new, use old
				if (!everyBufIsOnlyReaded) 
				{
					pMe->head = (pMe->head + 1) % pMe->initParam.len;
					pMe->len--;
				}
				
				pCurFrm->status = eFRM_STATUS_READ_ING;
			}
			else
			{
				//use  empty  frame
				pCurFrm = AK_NULL;
			}
		}
		else
		{
			//use  empty  frame
			//pCurFrm = AK_NULL;//(defalut pCurFrm is  NULL)
		}
		
		lRet = AK_SUCCESS;
	}

	STRM_MUTEX_UNLOCK(pMe->strmMutex); // exit  protect zone
	
    //decide how to  call user fun  
	if (AK_IS_SUCCESS(lRet))
	{
	    //decide  the user  fun's  param
		if (AK_NULL != pCurFrm)
		{
			pFrmData = pCurFrm->pData;//use new  or new  frame
		}
		
		else
		{
			pFrmData = AK_NULL;// use empty  frame
		}
		
#ifdef STRM_DEBUG_TRACE
        if (pMe->initParam.enableDebugMode)
        {
            Fwl_Print(C3, M_AKFRAME,  "[Gd_%d]\n",readPos);
        }
#endif

#ifdef STRM_READ_PROTECT
		STRM_MUTEX_LOCK(pMe->readMutex);
#endif
	   // call user 's callback
	   if (AK_NULL != getProcFun)
	   {
		   lRet = getProcFun(pFrmData, param);//	call user 's callback
	   }
	   
	   else
	   {
		   lRet = AK_SUCCESS;
	   }

        if  (AK_NULL != pCurFrm)
        {
            //update the  status  (new or old  frame)
#ifdef STRM_READ_PROTECT
        	if (AK_IS_SUCCESS(lRet))
        	{
        		STRM_MUTEX_UNLOCK(pMe->readMutex);
                pCurFrm->status = eFRM_STATUS_READ_OK; // this data can dymatic opt proc
        	}
#else      
            pCurFrm->status = eFRM_STATUS_READ_OK; // this data can dymatic opt proc
#endif
            if (!everyBufIsOnlyReaded)
            {
#ifdef STRM_DEBUG_TRACE
                if (pMe->initParam.enableDebugMode)
                {
                    Fwl_Print(C3, M_AKFRAME, "[Ge_%d]\n",readPos);
                }
#endif
                STRM_SEM_POST(pMe->nonFullSem);// create a nonfull semphore for put 
            }
        }
	}

    return (AK_NULL != pCurFrm) ? AK_SUCCESS : AK_EFAILED;   
}



/*
 * @brief   touch(update status) one frame from  stream whitch cound not be write
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] IFrmStrm: stream handle
 * @param[in] pFrmBufferAddr: frame's data buffer address
 * @return	resulst AK_SUCCESS--touch Ok, else  fail
 */
static T_S32 CFrmStrm_TouchFrmByAddr(IFrmStrm *pIFrmStrm, T_U8 *pFrmBufferAddr)
{
#ifdef STRM_READ_PROTECT
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
	T_S32 i = 0;

	STRM_MUTEX_LOCK(pMe->strmMutex); // entry protect zone
	if (AK_NULL != pMe->frameArray)
	{
		for (i=0; i< (T_S32)pMe->initParam.len; i++)
		{
			if ((eFRM_STATUS_READ_ING == pMe->frameArray[i].status)\
				&& (AK_NULL != pMe->frameArray[i].pData))
			{

				 // add this logic for force refresh one frame status
				 //(such as ui refresh used extern address,just use this mutex logic but addr)
				if (0 == pFrmBufferAddr)
				{
					pMe->frameArray[i].status = eFRM_STATUS_READ_OK;
				}
				
				else if (pFrmBufferAddr == pMe->frameArray[i].pData->pBuffer)
				{
				   pMe->frameArray[i].status = eFRM_STATUS_READ_OK;
				}

				if (eFRM_STATUS_READ_OK == pMe->frameArray[i].status)
				{
#ifdef STRM_DEBUG_TRACE
                    if (pMe->initParam.enableDebugMode)
                    {
                        Fwl_Print(C3, M_AKFRAME,  "\n~G_%d~\n",pMe->head);
                    }
#endif
					STRM_SEM_POST(pMe->nonFullSem);// create a nonfull semphore for write
					if (STRM_MUTEX_IS_LOCKED(pMe->readMutex))
					{
						STRM_MUTEX_UNLOCK(pMe->readMutex);
					}
					break;
				}
			}
		}
	}
	STRM_MUTEX_UNLOCK(pMe->strmMutex);// exit protect zone
#endif

	return AK_SUCCESS;
}


/*
 * @brief   get  one  stream cureent size(valid node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_U16   CFrmStrm_Size(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    
    return (T_U16)pMe->len;   
}


/*
 * @brief   get  one  stream capcity(all node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_U16   CFrmStrm_GetCapcity(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    
    return (T_U16)pMe->initParam.len;
}


/*
 * @brief   get  one  stream translate type(all node count)
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	frame trans type, data or address
 */
static T_U16   CFrmStrm_GetTransType(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    
    return pMe->initParam.transMode;
}

/*
 * @brief   stop  one  stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_S32   CFrmStrm_Stop(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    
	return CFrmStrm_Enable(pMe, AK_FALSE);
}

/*
 * @brief   start  one  stream process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	stream's frame  node count
 */
static T_S32   CFrmStrm_Start(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    
	return CFrmStrm_Enable(pMe, AK_TRUE);
}

/*
 * @brief   set  one  stream all frame info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @param[in] pInfo: new frame info
 * @return	resulst AK_SUCCESS--touch Ok, else  fail
 */
static T_S32   CFrmStrm_SetFrmInfo(IFrmStrm *pIFrmStrm,T_FRM_INFO *pInfo)
{
	CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
	T_S32 i = 0;
	T_FRM_DATA *data = AK_NULL;
	T_BOOL isNeedClearBuf = AK_FALSE;

	STRM_MUTEX_LOCK(pMe->strmMutex); // entry protect zone

    // disable stream process
	CFrmStrm_Enable(pMe, AK_FALSE);

	if ((AK_NULL != pInfo) && (AK_NULL != pMe->frameArray))
	{
	    if (eSTRM_TYPE_DATA == pMe->initParam.transMode
			|| eSTRM_TYPE_DISP == pMe->initParam.transMode)
	    {
            isNeedClearBuf = AK_TRUE;

	    }
	    else
	    {
            isNeedClearBuf = AK_FALSE;
	    }
	    
		for (i=0;i<(T_S32)pMe->initParam.len; i++)
		{
			data = pMe->frameArray[i].pData;
			if (AK_NULL != data)
			{
                memcpy(&data->info, pInfo, sizeof(T_FRM_INFO));
                if ((isNeedClearBuf) && (AK_NULL != data->pBuffer))
                {
                    Fwl_Print(C3, M_AKFRAME, "Clear Frm Buf.");
                    // clear frame buffer 
                    memset(data->pBuffer, 0, data->info.size);
                }
                Fwl_Print(C3, M_AKFRAME,"\tFrm[%d]_0x%x:size=%d(%dx%d)\n",i,data->pBuffer,data->info.size, data->info.width, data->info.height);
			}
			else
			{
				Fwl_Print(C3, M_AKFRAME,"\tFrm[%d]_0x%x\n",i,data);
			}
		}
	}
	
    // enable stream process
	CFrmStrm_Enable(pMe, AK_TRUE);
	STRM_MUTEX_UNLOCK(pMe->strmMutex);// exit  protect zone
	
	return AK_SUCCESS;	 
}

/*
 * @brief   dump  one  stream info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pIFrmStrm: stream handle
 * @return	AK_SUCCESS: dump Ok, else fail
 */
static T_S32   CFrmStrm_Dump(IFrmStrm *pIFrmStrm)
{
    CFrmStrm *pMe = (CFrmStrm *)pIFrmStrm->pData;
    T_S32 i = 0;
	T_FRM_DATA *data = AK_NULL;


	STRM_MUTEX_LOCK(pMe->strmMutex); // entry protect zone
	AK_DEBUG_OUTPUT("\n\t== FrmStrm(%08x) Begin ==\n",pIFrmStrm);

	// print stream information
	if (pMe->initParam.len > 0)
	{
		AK_DEBUG_OUTPUT("\tinitLen=%d(trans=0x%x) size=%d(%dx%d)\n"\
			"\tput/get(0x%x/0x%x_%d/%d) CurLen=%d @[%d<-%d]~\n\t::\n",
			pMe->initParam.len, pMe->initParam.transMode,
			pMe->initParam.frameInfo.size, pMe->initParam.frameInfo.width,pMe->initParam.frameInfo.height,
			pMe->initParam.putProcMode, pMe->initParam.getProcMode,
			pMe->initParam.enableReadFromOld, pMe->initParam.enableWriteToOld,
			pMe->len,pMe->head,pMe->tail);
	}

	// print all frame information
	if (AK_NULL != pMe->frameArray)
	{
		for (i=0;i<(T_S32)pMe->initParam.len; i++)
		{
			data = pMe->frameArray[i].pData;
			if (AK_NULL != data)
			{
			   AK_DEBUG_OUTPUT("\tFrm[%d]_0x%x:size=%d<[%dx%d]/(%d,%d)[%dx%d]>\n",i,data->pBuffer,
                data->info.size, data->info.width, data->info.height,
                data->info.rect.left, data->info.rect.top,
                data->info.rect.width, data->info.rect.height);
			}
			else
			{
				AK_DEBUG_OUTPUT("\tFrm[%d]_0x%x\n",i,data);
			}
		}
	}

	if (pMe->initParam.len > 0)
	{
		AK_DEBUG_OUTPUT("\t== FrmStrm(%08x)  End  ==\n",pIFrmStrm);
	}
	
	STRM_MUTEX_UNLOCK(pMe->strmMutex);// exit  protect zone
	
    return AK_SUCCESS;   
}


