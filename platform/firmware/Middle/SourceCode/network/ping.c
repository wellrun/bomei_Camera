/**
  * @Copyrights (C)  ANYKA
  * @All rights reserved.
  * @File name: ping.c
  * @Function:  ping interface
  * @Author:    
  * @Date:      
  * @Version:   1.0
  */

#include <stddef.h>

#ifdef SUPPORT_NETWORK
#include "ping.h"
#include "fwl_net.h"
#include "eng_debug.h"
#include "gbl_macrodef.h"


#include "../../Library/lwip/include/opt.h"
#include "../../Library/lwip/include/mem.h"
#include "../../Library/lwip/include/raw.h"
#include "../../Library/lwip/include/icmp.h"
#include "../../Library/lwip/include/netif.h"
#include "../../Library/lwip/include/sys.h"
#include "../../Library/lwip/include/timers.h"
#include "../../Library/lwip/include/inet_chksum.h"


/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif


/* ping variables */
static T_U16 ping_seq_num;
static T_U32 ping_time;
static ping_recv_fn recv_func;


static struct raw_pcb *ping_pcb;


/** Prepare a echo ICMP request */
static T_VOID ping_prepare_echo( struct icmp_echo_hdr *iecho, T_U16 len)
{
	size_t i;
	size_t data_len = len - sizeof(struct icmp_echo_hdr);

	ICMPH_TYPE_SET(iecho, ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id     = PING_ID;
	iecho->seqno  = htons(++ping_seq_num);

	/* fill the additional data buffer with some data */
	for(i = 0; i < data_len; i++) 
	{
		((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
	}

	iecho->chksum = inet_chksum(iecho, len);
}



/* Ping using the raw ip */
static T_U8 ping_recv(T_VOID *arg, struct raw_pcb *pcb, struct pbuf *p, ip_addr_t *addr)
{
	struct icmp_echo_hdr *iecho;
	T_U32 time = 0;
	
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(pcb);
	LWIP_UNUSED_ARG(addr);
	AK_ASSERT_PTR(p, "ping_recv(): p error", 0);

	if (ICMP_ER != *((T_U8 *)p->payload + PBUF_IP_HLEN))
	{
		return 0; /* don't eat the packet */
	}

	if ((p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr))) &&
		pbuf_header( p, -PBUF_IP_HLEN) == 0) 
	{
		iecho = (struct icmp_echo_hdr *)p->payload;

		if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) 
		{
			time = sys_now()-ping_time;

			/* do some ping result processing */
			if (AK_NULL != recv_func)
			{
				recv_func(addr->addr, PING_DATA_SIZE, time);
			}
			
			pbuf_free(p);
			return 1; /* eat the packet */
		}
	}

	return 0; /* don't eat the packet */
}

static T_VOID ping_send(struct raw_pcb *raw, ip_addr_t *addr)
{
	struct pbuf *p;
	struct icmp_echo_hdr *iecho;
	size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
	
	AK_ASSERT_VAL_VOID(ping_size <= 0xffff, "ping_send(): ping_size error");

	p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
	
	if (!p) 
	{
		return;
	}
	
	if ((p->len == p->tot_len) && (p->next == NULL)) 
	{
		iecho = (struct icmp_echo_hdr *)p->payload;

		ping_prepare_echo(iecho, (u16_t)ping_size);

		raw_sendto(raw, p, addr);
		ping_time = sys_now();
	}
	
	pbuf_free(p);
}

static T_VOID ping_timeout(T_VOID *arg)
{

}

static T_VOID ping_raw_init(T_VOID)
{
	ping_pcb = raw_new(IP_PROTO_ICMP);
	AK_ASSERT_PTR_VOID(ping_pcb, "ping_raw_init(): ping_pcb error");

	raw_recv(ping_pcb, ping_recv, NULL);
	raw_bind(ping_pcb, IP_ADDR_ANY);
	sys_timeout(PING_DELAY, ping_timeout, ping_pcb);
}


/**
* @brief ping init
*
* @author Songmengxing
* @date 2014-6-19
* @param in ping_recv_fn func:ping recv function
* @return T_VOID
* @retval 
*/    
T_VOID ping_init(ping_recv_fn func)
{
	ping_raw_init();
	recv_func = func;
}

/**
* @brief ping free
*
* @author Songmengxing
* @date 2014-6-19
* @param T_VOID
* @return T_VOID
* @retval 
*/    
T_VOID ping_free(T_VOID)
{
	raw_remove(ping_pcb);
	ping_pcb = AK_NULL;
}


/**
* @brief ping send
*
* @author Songmengxing
* @date 2014-6-19
* @param in T_U32 ipaddr:remote ip addr
* @return T_VOID
* @retval 
*/ 
T_VOID ping_send_now(T_U32 ipaddr)
{
	ip_addr_t ping_target;
	
	ping_target.addr = ipaddr;
	
	AK_ASSERT_PTR_VOID(ping_pcb, "ping_send_now(): ping_pcb error");
	
	ping_send(ping_pcb, &ping_target);
}


#endif


