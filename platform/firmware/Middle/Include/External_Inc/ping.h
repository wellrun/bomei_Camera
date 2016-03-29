/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: ping.h
  * @Function:  ping interface
  * @Author:    
  * @Date:      
  * @Version:   1.0
  */


#ifndef __PING_H__
#define __PING_H__

#ifdef SUPPORT_NETWORK

#include "anyka_types.h"

//ping应用接口
typedef T_VOID (*ping_recv_fn)(T_U32 addr, T_U32 size, T_U32 time);

/**
* @brief ping init
*
* @author Songmengxing
* @date 2014-6-19
* @param in ping_recv_fn func:ping recv function
* @return T_VOID
* @retval 
*/ 
T_VOID ping_init(ping_recv_fn func);

/**
* @brief ping free
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_VOID
* @retval 
*/    
T_VOID ping_free(T_VOID);

/**
* @brief ping send
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 ipaddr:remote ip addr
* @return T_VOID
* @retval 
*/ 
T_VOID ping_send_now(T_U32 ipaddr);

#endif
#endif


