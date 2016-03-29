/**
 * @FILENAME: hal_send_pcm.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-29
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
#define SENDPCM_MESSAGE 1

typedef struct
{
    T_S16 *pBufAddr;            ///< the address of buffer
    T_U32 dataLen;              ///< buffer length   
}T_SENDPCM_BUF;

static T_BOOL  m_bTaskInit = AK_FALSE;

typedef struct
{
    T_SENDPCM_BUF  Buf[BUF_MAX_NUM];///< data buffer
    T_U8    rpIndex;            ///< write index
    T_U8    wpIndex;            ///< read index     
    T_U32   BufNum;             ///< buffer number
    T_U32   OneBufSize;         ///< buffer size
    T_U8    L2Buf_id;
    T_BOOL  bL2Start;
    T_fSOUND SendFinish;
    T_S16   *pNullBuf;       //for send null data while not data to send
}T_SENDPCM_BUF_MSG;

static T_SENDPCM_BUF_MSG m_sendpcm_msg;
static T_VOID sendpcm_l2_callback(T_VOID);
static T_BOOL sendpcm_l2_init( T_VOID );

T_BOOL sendpcm_free ( T_VOID );
T_BOOL sendpcm_setcallback (T_fSOUND setcallback);

 
/**
 * @brief  L2 callback,it will be call while one buffer send finished
 * @author LianGenhui
 * @date   2010-07-29
*/
static T_VOID sendpcm_l2_callback(T_VOID)
{
    T_U32 read_index;

    if(BUF_NULL == m_sendpcm_msg.L2Buf_id)
        return;

    DrvModule_Send_Message(DRV_MODULE_DA, SENDPCM_MESSAGE, AK_NULL);

    //send data to pcm 
    if(m_sendpcm_msg.rpIndex == m_sendpcm_msg.wpIndex )
    {        
        m_sendpcm_msg.bL2Start = AK_FALSE;
    }
    else
    {
        //send next data buffer
        read_index = m_sendpcm_msg.rpIndex;
        l2_combuf_dma((T_U32)m_sendpcm_msg.Buf[read_index].pBufAddr, m_sendpcm_msg.L2Buf_id, m_sendpcm_msg.Buf[read_index].dataLen, MEM2BUF, AK_TRUE);
        m_sendpcm_msg.rpIndex = (m_sendpcm_msg.rpIndex + 1) % m_sendpcm_msg.BufNum;
    }

}

static T_VOID sendpcm_handler(T_U32 *param, T_U32 len)
{
    if(m_sendpcm_msg.SendFinish != AK_NULL)
        m_sendpcm_msg.SendFinish ();//callback,send one buffer data finished 
}

/**
 * @brief  initial l2,alloc L2 buffer,set l2 callback
 * @author LianGenhui
 * @date   2010-07-29
 */
static T_BOOL sendpcm_l2_init( T_VOID )
{
    T_S32 status;

    //alloc l2 buffer
    m_sendpcm_msg.L2Buf_id = l2_alloc(ADDR_DAC);

    if(BUF_NULL == m_sendpcm_msg.L2Buf_id)
    {
        akprintf(C2, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n");
        return AK_FALSE;
    }    
    
    //set L2 callback function
    l2_set_dma_callback (m_sendpcm_msg.L2Buf_id, sendpcm_l2_callback);

    return AK_TRUE;
}

static T_VOID parameter_init(T_VOID)
{
    m_sendpcm_msg.L2Buf_id = BUF_NULL;
    m_sendpcm_msg.SendFinish = AK_NULL;
    m_sendpcm_msg.bL2Start = AK_FALSE;
    m_sendpcm_msg.pNullBuf = AK_NULL; 
}

/**
 * @brief  malloc sendpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return T_BOOL
 */
T_BOOL sendpcm_malloc (T_U32 OneBufSize, T_U32 BufNum)
{
    T_U32 i;
    T_U32 j;

    if(((OneBufSize % 512) != 0) || (BufNum < 4))
    {
        akprintf(C1, M_DRVSYS, "OneBufSize %% 512 != 0 or BufNum < 4\r\n");
        return AK_FALSE;
    }

    if(m_bTaskInit)
    {
        akprintf(C1, M_DRVSYS, "can't malloc again,please free!\n");
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_DA); 
    
    parameter_init();
    
    //create send pcm task
    if(!DrvModule_Create_Task(DRV_MODULE_DA))
    {
       DrvModule_UnProtect(DRV_MODULE_DA); 
       return AK_FALSE;
    }

    //set massage and handler
    DrvModule_Map_Message(DRV_MODULE_DA, SENDPCM_MESSAGE, sendpcm_handler);

    //malloc all buffer and set to 0
    for (i=0; i<BufNum; i++)
    {
        m_sendpcm_msg.Buf[i].pBufAddr = (T_S16 *)drv_malloc(OneBufSize);

        if (AK_NULL ==  m_sendpcm_msg.Buf[i].pBufAddr)//malloc fail
        {
            for (j=0; j<i; j++)//free the mallocated memories
            {
                drv_free(m_sendpcm_msg.Buf[j].pBufAddr);
                m_sendpcm_msg.Buf[j].pBufAddr = AK_NULL;
            }

            akprintf(C1, M_DRVSYS, "malloc fail!\n");
            DrvModule_UnProtect(DRV_MODULE_DA); 
            return AK_FALSE;
        }
        memset(m_sendpcm_msg.Buf[i].pBufAddr, 0, OneBufSize);
    } 

    //malloc null buffer,for send 0 data to pcm while not data to send
    m_sendpcm_msg.pNullBuf = (T_S16 *)drv_malloc(OneBufSize);
    if(AK_NULL == m_sendpcm_msg.pNullBuf)
    {
        akprintf(C1, M_DRVSYS, "malloc fail!\n");
        m_sendpcm_msg.pNullBuf = AK_NULL;
        for(i=0; i<BufNum; i++)
        {
            drv_free(m_sendpcm_msg.Buf[i].pBufAddr);
            m_sendpcm_msg.Buf[i].pBufAddr = AK_NULL;
        }
        DrvModule_UnProtect(DRV_MODULE_DA); 
        return AK_FALSE;
     }
    memset(m_sendpcm_msg.pNullBuf, 0, OneBufSize);//set to 0 for null buffer

    m_sendpcm_msg.rpIndex = 0; //set default for read point index
    m_sendpcm_msg.wpIndex = 0;
    m_sendpcm_msg.BufNum = BufNum;
    m_sendpcm_msg.OneBufSize = OneBufSize;
    m_sendpcm_msg.bL2Start = AK_FALSE;

    //L2 to PCM initial ,include DAC and IIS send
    sendpcm_l2_init();
    
    m_bTaskInit = AK_TRUE;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}

/**
 * @brief  realloc sendpcm buffer
 * @author liao_zhijun
 * @date   2010-11-02
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return T_BOOL
 */
T_BOOL sendpcm_realloc (T_U32 OneBufSize, T_U32 BufNum, T_fSOUND callback)
{
    T_BOOL ret;

    sendpcm_free();
    ret = sendpcm_malloc(OneBufSize, BufNum);

    sendpcm_setcallback(callback);

    return ret;
}

/**
 * @brief  free sendpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return T_BOOL
 */
T_BOOL sendpcm_free ( T_VOID )
{
    T_U32 i;
    DrvModule_Protect(DRV_MODULE_DA); 
    
    m_sendpcm_msg.L2Buf_id = BUF_NULL;
    l2_free(ADDR_DAC);
    
    DrvModule_Terminate_Task(DRV_MODULE_DA);//release DA task
    
    for(i=0; i<m_sendpcm_msg.BufNum; i++)
    {
        drv_free(m_sendpcm_msg.Buf[i].pBufAddr);
        m_sendpcm_msg.Buf[i].pBufAddr = AK_NULL;
    }
    m_sendpcm_msg.BufNum = 0;
    m_sendpcm_msg.OneBufSize = 0;

    //free null buf
    drv_free(m_sendpcm_msg.pNullBuf);
    m_sendpcm_msg.pNullBuf = AK_NULL;
    m_bTaskInit = AK_FALSE;
    m_sendpcm_msg.bL2Start = AK_FALSE;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}

/**
 * @brief  set callback function
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] setcallback callback function,it will be call when one buffer send finished
 * @return T_BOOL
 */
T_BOOL sendpcm_setcallback (T_fSOUND setcallback)
{
    if(AK_NULL == setcallback)
    return AK_FALSE;

    m_sendpcm_msg.SendFinish = setcallback;
    return AK_TRUE;
}
 
/**
 * @brief  clean  buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return T_VOID
 */
T_VOID sendpcm_cleanbuf ( T_VOID )
{
    T_U32 i;
    T_U32 delay_cnt = 0;

    DrvModule_Protect(DRV_MODULE_DA); 

    while (1)
    {
        if (AK_FALSE == m_sendpcm_msg.bL2Start)
            break;
        else 
        {
            mini_delay(1);
            delay_cnt++;
            if (delay_cnt > 300)
            {
                akprintf(C3, M_DRVSYS, "clean buf timeout\n");
                break;
            }
        }
    }       
    
    for(i=0; i<m_sendpcm_msg.BufNum; i++)
    {
       memset(m_sendpcm_msg.Buf[i].pBufAddr, 0, m_sendpcm_msg.OneBufSize);
    }

    m_sendpcm_msg.rpIndex = 0; //set define for read point index
    m_sendpcm_msg.wpIndex = 0;
    
    DrvModule_UnProtect(DRV_MODULE_DA); 
}
 
/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve dac data
 * @author LianGenhui
 * @date   2010-07-29
 * @param[out] pbuf return buffer address or AK_NULL
 * @param[out] len return buffer len or 0
 * @return T_BOOL
 * @retval AK_TRUE  get buffer successful
 * @retval AK_FALSE get buffer failed
 * @note   if sendpcm_create failed or no buffer to return, it will return failed
 */
T_BOOL sendpcm_getbuf ( T_VOID **pbuf, T_U32 *len )
{
    T_U32 bufHead;
    
    DrvModule_Protect(DRV_MODULE_DA); 

    if(((m_sendpcm_msg.wpIndex +1) % m_sendpcm_msg.BufNum) ==  m_sendpcm_msg.rpIndex )
    {
        *pbuf = AK_NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_DA); 
        return AK_FALSE;
    }

    bufHead = m_sendpcm_msg.wpIndex;

    *pbuf = m_sendpcm_msg.Buf[bufHead].pBufAddr;
    *len = m_sendpcm_msg.OneBufSize;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    return AK_TRUE;
}

/**
 * @brief set one buffer end\n
 * after call sendpcm_getbuf and finish the operation of dac data,call this function
 * @author  LianGenhui
 * @date    2010-07-29 
 * @param[in] len   buffer len(use for write)
 * @return T_BOOL
 * @retval AK_TRUE  successful
 * @retval AK_FALSE longer than one buffer's len
 */
T_BOOL sendpcm_endbuf ( T_U32 len )
{
    T_U32 idx;
    
    if(len > m_sendpcm_msg.OneBufSize)
        return AK_FALSE;

    DrvModule_Protect(DRV_MODULE_DA); 

    //save this buffer len
    m_sendpcm_msg.Buf[m_sendpcm_msg.wpIndex].dataLen = len;

    //point to next buffer
    m_sendpcm_msg.wpIndex = (m_sendpcm_msg.wpIndex + 1) % m_sendpcm_msg.BufNum;

    if(!m_sendpcm_msg.bL2Start)
    { 
        idx = m_sendpcm_msg.rpIndex;
        m_sendpcm_msg.rpIndex = (m_sendpcm_msg.rpIndex + 1) % m_sendpcm_msg.BufNum;
        m_sendpcm_msg.bL2Start = AK_TRUE;
        
        l2_combuf_dma((T_U32)m_sendpcm_msg.Buf[idx].pBufAddr, m_sendpcm_msg.L2Buf_id, 
            m_sendpcm_msg.Buf[idx].dataLen, MEM2BUF, AK_TRUE);
    }

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}

/**
 * @brief get the number of buffers which have been filled or retrieved dac data 
 * @author LianGenhui
 * @date   2010-07-29
 * @return T_U32
 * @retval value the value will from 0 to the number when create a dac set 
  */
T_U32 sendpcm_getnum_fullbuf( T_VOID )
{
    T_U32 num_fullbuf;
    DrvModule_Protect(DRV_MODULE_DA); 
    
    if( m_sendpcm_msg.wpIndex < m_sendpcm_msg.rpIndex )
    {
        num_fullbuf =  m_sendpcm_msg.wpIndex + m_sendpcm_msg.BufNum -  m_sendpcm_msg.rpIndex;
    }
    else
    {
         num_fullbuf =  m_sendpcm_msg.wpIndex -  m_sendpcm_msg.rpIndex;
    }
    
    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return num_fullbuf;
}


