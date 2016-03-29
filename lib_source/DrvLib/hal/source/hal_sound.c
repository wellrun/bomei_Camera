/**
 * @FILENAME: hal_sound.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-27
 * @VERSION 1.0
 */

#include "l2.h"
#include "drv_api.h"
#include "hal_sound.h"

extern T_BOOL adc_open(T_VOID);
extern T_BOOL adc_close(T_VOID);
extern T_BOOL adc_setinfo(SOUND_INFO *info);
extern T_BOOL recvpcm_malloc (T_U32 OneBufSize, T_U32 BufNum);
extern T_BOOL recvpcm_realloc (T_U32 OneBufSize, T_U32 BufNum, T_fSOUND callback);
extern T_BOOL recvpcm_setcallback (T_fSOUND setcallback);
extern T_BOOL recvpcm_free ( T_VOID );
extern T_VOID recvpcm_cleanbuf ( T_VOID );
extern T_U32  recvpcm_getnum_fullbuf( T_VOID );
extern T_BOOL recvpcm_endbuf ( T_U32 len );
extern T_BOOL recvpcm_getbuf ( T_VOID **pbuf, T_U32 *len );

extern T_BOOL dac_open(T_VOID);
extern T_BOOL dac_close(T_VOID);
extern T_BOOL dac_setinfo(SOUND_INFO *info);
extern T_BOOL sendpcm_malloc (T_U32 OneBufSize, T_U32 BufNum);
extern T_BOOL sendpcm_realloc (T_U32 OneBufSize, T_U32 BufNum, T_fSOUND callback);
extern T_BOOL sendpcm_setcallback (T_fSOUND setcallback);
extern T_BOOL sendpcm_free ( T_VOID );
extern T_VOID sendpcm_cleanbuf ( T_VOID );
extern T_U32  sendpcm_getnum_fullbuf( T_VOID );
extern T_BOOL sendpcm_endbuf ( T_U32 len );
extern T_BOOL sendpcm_getbuf ( T_VOID **pbuf, T_U32 *len );

extern T_BOOL i2s_send_open(T_VOID);
extern T_BOOL i2s_send_close(T_VOID);
extern T_BOOL i2s_recv_open(T_VOID);
extern T_BOOL i2s_recv_close(T_VOID);
extern T_BOOL i2s_send_setinfo(SOUND_INFO *info);
extern T_BOOL i2s_recv_setinfo(SOUND_INFO *info);

static T_SOUND_DRV adc_function_handler = 
{
    adc_open,
    adc_close,
    adc_setinfo,
    recvpcm_malloc,
    recvpcm_realloc,
    recvpcm_setcallback,
    recvpcm_free,
    recvpcm_cleanbuf,
    recvpcm_getbuf,
    recvpcm_endbuf,
    recvpcm_getnum_fullbuf
};

static T_SOUND_DRV dac_function_handler = 
{
    dac_open,
    dac_close,
    dac_setinfo,
    sendpcm_malloc,
    sendpcm_realloc,
    sendpcm_setcallback,
    sendpcm_free,
    sendpcm_cleanbuf,
    sendpcm_getbuf,
    sendpcm_endbuf,
    sendpcm_getnum_fullbuf
};

#if 0
static T_SOUND_DRV i2s_send_function_handler = 
{
    i2s_send_open,
    i2s_send_close,
    i2s_send_setinfo,
    sendpcm_malloc,
    sendpcm_realloc,
    sendpcm_setcallback,
    sendpcm_free,
    sendpcm_cleanbuf,
    sendpcm_getbuf,
    sendpcm_endbuf,
    sendpcm_getnum_fullbuf

};

static T_SOUND_DRV i2s_recv_function_handler = 
{
    i2s_recv_open,
    i2s_recv_close,
    i2s_recv_setinfo,
    recvpcm_malloc,
    recvpcm_realloc,
    recvpcm_setcallback,
    recvpcm_free,
    recvpcm_cleanbuf,
    recvpcm_getbuf,
    recvpcm_endbuf,
    recvpcm_getnum_fullbuf
};
#endif

static T_SOUND_DRV *array_sound_function[] = {
    &adc_function_handler,
    &dac_function_handler,
    //&i2s_send_function_handler,
    //&i2s_recv_function_handler,
    AK_NULL
};

/**
 * @brief   create a sound driver, it will malloc sound buffer and init L2\n
 *          the callback function will be called when a read or write buffer complete, it can be AK_NULL and do nothing.
 * @author  LianGenhui
 * @date    2010-07-27
 * @param[in] driver sound driver, refer to SOUND_DRIVER
 * @param[in] OneBufSize buffer size
 * @param[in] DABufNum buffer number
 * @param[in] callback callback function or AK_NULL 
 * @return  T_SOUND_DRV
 * @retval  AK_NULL created failed
 */
T_SOUND_DRV *sound_create (SOUND_DRIVER driver,T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback)
{
    if((driver >= SOUND_DRIVER_MAX) || (AK_NULL == array_sound_function[driver]))
    {
        return AK_NULL;
    }

    if((AK_NULL == array_sound_function[driver]->sound_malloc_func) ||
        (AK_NULL == array_sound_function[driver]->sound_setcallback_func))
    {
        return AK_NULL;
    }

    if(AK_FALSE == array_sound_function[driver]->sound_malloc_func(OneBufSize, DABufNum))
    {
        return AK_NULL;
    }
    
    array_sound_function[driver]->sound_setcallback_func(callback);

    return array_sound_function[driver];//adc:0,dac:1,2:IIS send,3:IIS receive
}


/**
 * @brief  realloc buffer for giving sound driver
 * @author    liao_zhijun
 * @date    2010-11-02
 * @param[in] handler handler of the sound device
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @return  T_BOOL
 * @retval  AK_TRUE  realloc successful
 * @retval  AK_FALSE realloc failed
 */
T_BOOL sound_realloc (T_SOUND_DRV *handler,T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback)
{
    if((AK_NULL == handler) || (AK_NULL == handler->sound_realloc_func))
        return AK_FALSE;

    if(handler->sound_realloc_func != AK_NULL)
        return(handler->sound_realloc_func(OneBufSize, DABufNum, callback));
    else
        return AK_FALSE;    
}


/**
 * @brief  delete sound driver and Free sound buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device
 */
T_VOID sound_delete (T_SOUND_DRV *handler)
{
    if((AK_NULL == handler) || (AK_NULL == handler->sound_free_func))
        return;
    
    handler->sound_free_func();
}

/**
 * @brief  open a sound device and it can be used
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return T_BOOL
 * @retval AK_TRUE  open successful
 * @retval AK_FALSE open failed
 */
T_BOOL sound_open (T_SOUND_DRV *handler)
{
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }

    if(handler->sound_open_func != AK_NULL)
        return(handler->sound_open_func());
    else
        return AK_FALSE;
}

/**
 * @brief  Close a sound device
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return T_BOOL
 * @retval AK_TRUE close successful
 * @retval AK_FALSE close failed
 */
T_BOOL sound_close (T_SOUND_DRV *handler)
{
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }

    if(handler->sound_close_func != AK_NULL)
        return(handler->sound_close_func());
    else
        return AK_FALSE;
}

/**
 * @brief  Set sound sample rate, channel, bits per sample of the sound device
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @param[in] info     refer to SOUND_INFO
 * @return T_BOOL
 * @retval AK_TRUE set successful
 * @retval AK_FALSE set failed
 */
T_BOOL sound_setinfo (T_SOUND_DRV *handler, SOUND_INFO *info)
{
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }
    
    if(handler->sound_setinfo_func != AK_NULL)
        return(handler->sound_setinfo_func(info));
    else
        return AK_FALSE;
}

/**
 * @brief  clean sound buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return T_VOID
 */
T_VOID sound_cleanbuf (T_SOUND_DRV *handler)
{
    if(AK_NULL == handler)
    {
        return;
    }

    if(handler->sound_cleanbuf_func != AK_NULL)
        handler->sound_cleanbuf_func();
}

/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @param[out] pbuf return buffer address or AK_NULL
 * @param[out] len return buffer len or 0
 * @return T_BOOL
 * @retval AK_TRUE  get buffer successful
 * @retval AK_FALSE get buffer failed
 * @note   if sound_create failed or no buffer to return, it will return failed
 */
T_BOOL sound_getbuf (T_SOUND_DRV *handler, T_VOID **pbuf, T_U32 *len)
{
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }

    if(handler->sound_getbuf_func != AK_NULL)
        return(handler->sound_getbuf_func((T_VOID *)((T_S16 *)pbuf), len));
    else
        return AK_FALSE;
}

/**
 * @brief set one buffer end\n
 * after call sound_getbuf and finish the operation of sound data,call this function
 * @author  LianGenhui
 * @date    2010-07-27 
 * @param[in] handler handler of the sound device  
 * @param[in] len     buffer len(use for write)
 * @return T_BOOL
 * @retval AK_TRUE  successful
 * @retval AK_FALSE longer than one buffer's len
 */
T_BOOL sound_endbuf (T_SOUND_DRV *handler, T_U32 len)
{
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }
    
    if(handler->sound_endbuf_func != AK_NULL)
        return(handler->sound_endbuf_func(len));
    else
        return AK_FALSE;
}

/**
 * @brief get the number of buffers which have been filled or retrieved sound data 
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return T_U32
 * @retval value the value will from 0 to the number when create a sound set 
  */
T_U32 sound_getnum_fullbuf (T_SOUND_DRV *handler)
{
    T_U32 num = 0;
    
    if(AK_NULL == handler)
    {
        return AK_FALSE;
    }

    if(handler->sound_getnum_fullbuf_func != AK_NULL)
        num = handler->sound_getnum_fullbuf_func();
 
    return num;
}


