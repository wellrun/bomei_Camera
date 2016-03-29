/**@file  hal_sound.h
 * @brief sound operation interface
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author  Liangenhui 
 * @date  2010-06-30
 * @version 1.0
 */

#ifndef         __HAL_SOUND_H
#define         __HAL_SOUND_H

/** @defgroup SOUND sound group
 *  @ingroup Drv_Lib
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief sound callback function
 */
typedef T_VOID (*T_fSOUND)( T_VOID );

/**
 * @brief sound driver module
 */
typedef enum
{
    SOUND_ADC = 0,        ///< ADC driver
    SOUND_DAC,            ///< DAC driver
    SOUND_IIS_SEND,       ///< IIS send driver
    SOUND_IIS_RECEIVE,     ///< IIS receive driver
    SOUND_DRIVER_MAX
}SOUND_DRIVER;

/**
 * @brief sound info
 */
typedef struct
{
    T_U32    nSampleRate;    ///< SampleRate
    T_U16    nChannel;       ///< dimensional sound(2), single channel(1)
    T_U16    BitsPerSample;  ///< 8bit or 16 bit
}SOUND_INFO;

/**
 * @brief handler of the sound device
 */
typedef struct
{
    T_BOOL (*sound_open_func)( T_VOID ); 
    T_BOOL (*sound_close_func)( T_VOID );   
    T_BOOL (*sound_setinfo_func)( SOUND_INFO *info );
    T_BOOL (*sound_malloc_func) (T_U32 OneBufSize, T_U32 DABufNum);
    T_BOOL (*sound_realloc_func) (T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback);
    T_BOOL (*sound_setcallback_func) (T_fSOUND setcallback);
    T_BOOL (*sound_free_func)( T_VOID );
    T_VOID (*sound_cleanbuf_func)( T_VOID );
    T_BOOL (*sound_getbuf_func)( T_VOID **pbuf, T_U32 *len);
    T_BOOL (*sound_endbuf_func)( T_U32 len );
    T_U32  (*sound_getnum_fullbuf_func)( T_VOID );
}T_SOUND_DRV;

/**
 * @brief   create a sound driver, it will malloc sound buffer and init L2\n
 *          the callback function will be called when a read or write buffer complete, it can be AK_NULL and do nothing.
 * @author    LianGenhui
 * @date    2010-06-30
 * @param[in]  driver sound driver, refer to SOUND_DRIVER
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or AK_NULL 
 * @return  T_SOUND_DRV
 * @retval  AK_NULL created failed
 */
T_SOUND_DRV *sound_create (SOUND_DRIVER driver,T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback);


/**
 * @brief  realloc buffer for giving sound driver
 * @author    liao_zhijun
 * @date    2010-11-02
 * @param[in] handler handler of the sound device
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or AK_NULL 
 * @return  T_BOOL
 * @retval  AK_TRUE  realloc successful
 * @retval  AK_FALSE realloc failed
 */
T_BOOL sound_realloc (T_SOUND_DRV *handler,T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback);

/**
 * @brief  delete sound driver and Free sound buffer
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device
 */
T_VOID sound_delete (T_SOUND_DRV *handler);

/**
 * @brief  open a sound device and it can be used
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_BOOL
 * @retval  AK_TRUE  open successful
 * @retval  AK_FALSE open failed
 */
T_BOOL sound_open (T_SOUND_DRV *handler);

/**
 * @brief   Close a sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_BOOL
 * @retval  AK_TRUE close successful
 * @retval  AK_FALSE close failed
 */
T_BOOL sound_close (T_SOUND_DRV *handler);

/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[in] info     refer to SOUND_INFO
 * @return  T_BOOL
 * @retval  AK_TRUE set successful
 * @retval  AK_FALSE set failed
 */
T_BOOL sound_setinfo (T_SOUND_DRV *handler, SOUND_INFO *info);

/**
 * @brief   clean sound buffer
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_VOID
 */
T_VOID sound_cleanbuf (T_SOUND_DRV *handler);

/**
 * @brief   get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[out] pbuf return buffer address or AK_NULL
 * @param[out] len return buffer len or 0
 * @return  T_BOOL
 * @retval  AK_TRUE  get buffer successful
 * @retval  AK_FALSE get buffer failed
 * @note    if sound_create failed or no buffer to return, it will return failed
 */
T_BOOL sound_getbuf (T_SOUND_DRV *handler, T_VOID **pbuf, T_U32 *len);

/**
 * @brief set one buffer end\n
 * after call sound_getbuf and finish the operation of sound data,call this function
 * @author    LianGenhui
 * @date     2010-06-30 
 * @param[in] handler handler of the sound device  
 * @param[in] len     buffer len(use for write)
 * @return  T_BOOL
 * @retval  AK_TRUE  successful
 * @retval  AK_FALSE longer than one buffer's len
 */
T_BOOL sound_endbuf(T_SOUND_DRV *handler, T_U32 len);

/**
 * @brief get the number of buffers which have been filled or retrieved sound data 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_U32
 * @retval  value the value will from 0 to the number when create a sound set 
  */
T_U32 sound_getnum_fullbuf(T_SOUND_DRV *handler);

#ifdef __cplusplus
}
#endif

/*@}*/

#endif //end  #ifndef         __HAL_SOUND_H


