/**
 * @FILENAME: hal_receive_pcm.c
 * @BRIEF the adc code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-27
 * @VERSION 1.0
 */
#include "l2.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "anyka_cpu.h"
#include "drv_module.h"
#include "interrupt.h"

#define BUF_MAX_NUM     100
#define RECVPCM_MESSAGE 1

typedef struct
{
    T_S16 *pBufAddr;            ///< the address of buffer
    T_U32 dataLen;              ///< buffer length   
}T_RECVPCM_BUF;

typedef struct
{
    T_RECVPCM_BUF  Buf[BUF_MAX_NUM];///< data buffer
    T_U8    rpIndex;            ///< write index
    T_U8    wpIndex;            ///< read index     
    T_U32   BufNum;             ///< buffer number
    T_U32   OneBufSize;         ///< buffer size
    T_U8    L2Buf_id;           ///< l2 buffer id
    T_BOOL  bL2Start;           ///< flag of L2 start
    T_BOOL  bBufAllFull;        ///< flag of all buffer are full
    T_fSOUND RecvFinish;        ///< receive finish ,and call back
    T_S16   *pNullBuf;          ///< for recv null data while not data to recv
}T_RECVPCM_BUF_MSG;

static T_BOOL m_bTaskInit = AK_FALSE;
static T_RECVPCM_BUF_MSG m_recvpcm_msg;

static T_VOID recvpcm_l2_callback(T_VOID);
static T_BOOL recvpcm_l2_init( T_VOID );

T_BOOL recvpcm_free ( T_VOID );
T_BOOL recvpcm_setcallback (T_fSOUND setcallback);

/**
 * @brief  L2 callback,it will be call while one buffer recv finished
 * @author LianGenhui
 * @date   2010-07-29
*/
static T_VOID recvpcm_l2_callback(T_VOID)
{
    T_U32 write_index;

    if(BUF_NULL == m_recvpcm_msg.L2Buf_id)
        return;

    DrvModule_Send_Message(DRV_MODULE_AD, RECVPCM_MESSAGE, AK_NULL);

    if(((m_recvpcm_msg.wpIndex + 1) % m_recvpcm_msg.BufNum) ==  m_recvpcm_msg.rpIndex )
    {
        l2_combuf_dma((T_U32)m_recvpcm_msg.pNullBuf, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, AK_TRUE);

        //set all buffer full flag
        m_recvpcm_msg.bBufAllFull = AK_TRUE;

    }
    else
    {
        //receive next data buffer
        m_recvpcm_msg.wpIndex = (m_recvpcm_msg.wpIndex + 1) % m_recvpcm_msg.BufNum;

        write_index = m_recvpcm_msg.wpIndex;
        l2_combuf_dma((T_U32)m_recvpcm_msg.Buf[write_index].pBufAddr, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, AK_TRUE);
   }
}

static T_VOID recvpcm_handler(T_U32 *param, T_U32 len)
{
    //callback,receive one buffer data finished 
    if(m_recvpcm_msg.RecvFinish != AK_NULL)
        m_recvpcm_msg.RecvFinish ();
}

/**
 * @brief  initial l2,alloc L2 buffer,set l2 callback
 * @author LianGenhui
 * @date   2010-07-29
 */
static T_BOOL recvpcm_l2_init( T_VOID )
{
    T_S32 status;

    //alloc l2 buffer
    m_recvpcm_msg.L2Buf_id = l2_alloc(ADDR_ADC);

    if(BUF_NULL == m_recvpcm_msg.L2Buf_id)
    {
        akprintf(C2, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n");
        return AK_FALSE;
    }    
    
    //set L2 callback function
    l2_set_dma_callback (m_recvpcm_msg.L2Buf_id, recvpcm_l2_callback);

    return AK_TRUE;
}

static T_VOID parameter_init(T_VOID)
{
    m_recvpcm_msg.L2Buf_id = BUF_NULL;
    m_recvpcm_msg.RecvFinish = AK_NULL;
    m_recvpcm_msg.bL2Start = AK_FALSE;
    m_recvpcm_msg.pNullBuf = AK_NULL; 
    m_recvpcm_msg.bBufAllFull = AK_FALSE;
}

/**
 * @brief  malloc recvpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return T_BOOL
 */
T_BOOL recvpcm_malloc (T_U32 OneBufSize, T_U32 BufNum)
{
    T_U32 i;
    T_U32 j;

    if(((OneBufSize % 512) != 0) || (BufNum < 4))
    {
        akprintf(C2, M_DRVSYS, "OneBufSize %% 512 != 0 or BufNum < 4\r\n");
        return AK_FALSE;
    }

    if(m_bTaskInit)
    {
        akprintf(C2, M_DRVSYS, "can't malloc again,please free!\n");
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_AD); 
    
    parameter_init();
    
    //create receive pcm task
    if(!DrvModule_Create_Task(DRV_MODULE_AD))
    {
       DrvModule_UnProtect(DRV_MODULE_AD); 
       return AK_FALSE;
    }

    //set massage and handler
    DrvModule_Map_Message(DRV_MODULE_AD, RECVPCM_MESSAGE, recvpcm_handler);

    //malloc all buffer and set to 0
    for (i=0; i<BufNum; i++)
    {
        m_recvpcm_msg.Buf[i].pBufAddr = (T_S16 *)drv_malloc(OneBufSize);

        if (AK_NULL ==  m_recvpcm_msg.Buf[i].pBufAddr)//malloc fail
        {
            for (j=0; j<i; j++)//free the mallocated memories
            {
                drv_free(m_recvpcm_msg.Buf[j].pBufAddr);
                m_recvpcm_msg.Buf[j].pBufAddr = AK_NULL;
            }

            akprintf(C2, M_DRVSYS, "malloc fail!\n");
            DrvModule_UnProtect(DRV_MODULE_AD); 
            return AK_FALSE;
        }
        memset(m_recvpcm_msg.Buf[i].pBufAddr, 0, OneBufSize);
    } 

    m_recvpcm_msg.pNullBuf = (T_S16 *)drv_malloc(OneBufSize);
    if(AK_NULL == m_recvpcm_msg.pNullBuf)
    {
        akprintf(C1, M_DRVSYS, "malloc fail!\n");
        
        m_recvpcm_msg.pNullBuf = AK_NULL;
        for(i=0; i<BufNum; i++)
        {
            drv_free(m_recvpcm_msg.Buf[i].pBufAddr);
            m_recvpcm_msg.Buf[i].pBufAddr = AK_NULL;
        }
        
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return AK_FALSE;
    }
    memset(m_recvpcm_msg.pNullBuf, 0, OneBufSize);//set to 0 for null buffer

    m_recvpcm_msg.rpIndex = 0; //set define for read point index
    m_recvpcm_msg.wpIndex = 0;
    m_recvpcm_msg.BufNum = BufNum;
    m_recvpcm_msg.OneBufSize = OneBufSize;
    m_recvpcm_msg.bL2Start = AK_FALSE;

    //PCM to L2 initial ,include adc and IIS receive
    recvpcm_l2_init();
    
    m_bTaskInit = AK_TRUE;

    DrvModule_UnProtect(DRV_MODULE_AD); 
    
    return AK_TRUE;
}

/**
 * @brief  realloc recvpcm buffer
 * @author liao_zhijun
 * @date   2010-11-02
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return T_BOOL
 */
T_BOOL recvpcm_realloc (T_U32 OneBufSize, T_U32 BufNum, T_fSOUND callback)
{
    T_BOOL ret;
    
    recvpcm_free();
    ret = recvpcm_malloc(OneBufSize, BufNum);

    recvpcm_setcallback (callback);

    return ret;    
}

/**
 * @brief  free recvpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return T_BOOL
 */
T_BOOL recvpcm_free ( T_VOID )
{
    T_U32 i;
    DrvModule_Protect(DRV_MODULE_AD); 

    m_recvpcm_msg.L2Buf_id = BUF_NULL;
    l2_free(ADDR_ADC);

    DrvModule_Terminate_Task(DRV_MODULE_AD);//release ad task

    drv_free(m_recvpcm_msg.pNullBuf);
    m_recvpcm_msg.pNullBuf = AK_NULL;

    for(i=0; i<m_recvpcm_msg.BufNum; i++)
    {
        drv_free(m_recvpcm_msg.Buf[i].pBufAddr);
        m_recvpcm_msg.Buf[i].pBufAddr = AK_NULL;
    }
    m_recvpcm_msg.BufNum = 0;
    m_recvpcm_msg.OneBufSize = 0;

    //free null buf
    m_bTaskInit = AK_FALSE;
    m_recvpcm_msg.bL2Start = AK_FALSE;

    DrvModule_UnProtect(DRV_MODULE_AD); 

    return AK_TRUE;
}

/**
 * @brief  set callback function
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] setcallback callback function,it will be call when one buffer receive finished
 * @return T_BOOL
 */
T_BOOL recvpcm_setcallback (T_fSOUND setcallback)
{
    if(AK_NULL == setcallback)
    return AK_FALSE;

    m_recvpcm_msg.RecvFinish = setcallback;
    return AK_TRUE;
}

/**
 * @brief  clean adc buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @return T_VOID
 */
T_VOID recvpcm_cleanbuf ( T_VOID )
{
    T_U32 i;
    DrvModule_Protect(DRV_MODULE_AD); 

    for(i=0; i<m_recvpcm_msg.BufNum; i++)
    {
       memset(m_recvpcm_msg.Buf[i].pBufAddr, 0, m_recvpcm_msg.OneBufSize);
    }

    m_recvpcm_msg.rpIndex = 0; //set define for read point index
    m_recvpcm_msg.wpIndex = 0;
    
    DrvModule_UnProtect(DRV_MODULE_AD); 
}

/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve adc data
 * @author LianGenhui
 * @date   2010-07-27
 * @param[out] pbuf return buffer address or AK_NULL
 * @param[out] len return buffer len or 0
 * @return T_BOOL
 * @retval AK_TRUE  get buffer successful
 * @retval AK_FALSE get buffer failed
 * @note   if adbuf_create failed or no buffer to return, it will return failed
 */
T_BOOL recvpcm_getbuf ( T_VOID **pbuf, T_U32 *len )
{
    T_U32 bufHead;
    
    DrvModule_Protect(DRV_MODULE_AD); 

    //if not start,receive first buffer
    if(!m_recvpcm_msg.bL2Start)
    {
        l2_combuf_dma((T_U32)m_recvpcm_msg.Buf[m_recvpcm_msg.rpIndex].pBufAddr, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, AK_TRUE);
        m_recvpcm_msg.bL2Start = AK_TRUE;

        *pbuf = AK_NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return AK_FALSE;
    }

    //not buffer to read
    if(m_recvpcm_msg.rpIndex == m_recvpcm_msg.wpIndex )
    {
        *pbuf = AK_NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return AK_FALSE;
    }

    //get read buffer index and buffer size
    bufHead = m_recvpcm_msg.rpIndex;
    *pbuf = m_recvpcm_msg.Buf[bufHead].pBufAddr;
    *len = m_recvpcm_msg.OneBufSize;

    DrvModule_UnProtect(DRV_MODULE_AD); 
    return AK_TRUE;
}

/**
 * @brief set one buffer end\n
 * after call adbuf_getbuf and finish the operation of adc data,call this function
 * @author  LianGenhui
 * @date    2010-07-27 
 * @param[in] len     buffer len(use for write)
 * @return T_BOOL
 * @retval AK_TRUE  successful
 * @retval AK_FALSE longer than one buffer's len
 */
T_BOOL recvpcm_endbuf ( T_U32 len )
{
    DrvModule_Protect(DRV_MODULE_AD); 
    
    //point to next buffer
    m_recvpcm_msg.rpIndex = (m_recvpcm_msg.rpIndex + 1) % m_recvpcm_msg.BufNum;

    //receive AD again, it is closed when all buffer fill data  
    if(AK_TRUE == m_recvpcm_msg.bBufAllFull)
    {
        m_recvpcm_msg.bBufAllFull = AK_FALSE;
    }

    DrvModule_UnProtect(DRV_MODULE_AD); 
    return AK_TRUE;
}

/**
 * @brief get the number of buffers which have been filled or retrieved adc data 
 * @author LianGenhui
 * @date   2010-07-27
 * @return T_U32
 * @retval value the value will from 0 to the number when create a adc set 
  */
T_U32 recvpcm_getnum_fullbuf( T_VOID )
{
    T_U32 num_fullbuf;
    DrvModule_Protect(DRV_MODULE_AD); 
    
    if( m_recvpcm_msg.wpIndex < m_recvpcm_msg.rpIndex )
    {
        num_fullbuf =  m_recvpcm_msg.wpIndex + m_recvpcm_msg.BufNum -  m_recvpcm_msg.rpIndex;
    }
    else
    {
         num_fullbuf =  m_recvpcm_msg.wpIndex -  m_recvpcm_msg.rpIndex;
    }
    
    DrvModule_UnProtect(DRV_MODULE_AD); 
    
    return num_fullbuf;
}


