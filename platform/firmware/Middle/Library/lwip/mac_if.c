/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */

#include <string.h>
#ifdef SUPPORT_NETWORK

#include "include/opt.h"

#include "include/debug.h"

#include "include/def.h"
#include "include/mem.h"
#include "include/pbuf.h"
#include "include/stats.h"
#include "include/sys.h"
#include "include/ip.h"
#include "include/tcpip.h"
#include "include/timers.h"
#include "include/etharp.h"


#include "anyka_types.h"
#include "eng_debug.h"
#include "arch_mac.h"

typedef struct S_MAC_IF
{
    T_U8 mac_addr[ETHARP_HWADDR_LEN];

    T_U8 link_state;
}
T_MAC_IF;

T_MAC_IF m_mac_if;

/** Low-level initialization: find the correct adapter and initialize it.
 */
static void
macif_low_level_init(struct netif *netif)
{	
	AK_ASSERT_PTR_VOID(netif, "macif_low_level_init(): netif error");
	
    netif->state = (void *)&m_mac_if;

    //set mac address
    memcpy(&netif->hwaddr, m_mac_if.mac_addr, ETHARP_HWADDR_LEN);

    //set link state
    if(m_mac_if.link_state)
    {
        netif_set_link_up(netif);
    }
    else
    {
        netif_set_link_down(netif);
    }

    //other
}

/** low_level_output():
 * Transmit a packet. The packet is contained in the pbuf that is passed to
 * the function. This pbuf might be chained.
 */
static err_t
macif_low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    unsigned char buffer[1520];
    unsigned char *buf = buffer;
    unsigned char *ptr;
    T_U16 tot_len = p->tot_len - ETH_PAD_SIZE;

	AK_ASSERT_PTR(netif, "macif_low_level_output(): netif error", ERR_VAL);
	AK_ASSERT_PTR(p, "macif_low_level_output(): p error", ERR_VAL);

    if (p->len == p->tot_len)
    {
        buf = &((unsigned char*)p->payload)[ETH_PAD_SIZE];
    }

    ptr = buffer;
	
    for(q = p; q != NULL; q = q->next)
    {
        if (q == p)
        {
            memcpy(ptr, &((char*)q->payload)[ETH_PAD_SIZE], q->len - ETH_PAD_SIZE);
            ptr += q->len - ETH_PAD_SIZE;
        }
        else
        {
            memcpy(ptr, q->payload, q->len);
            ptr += q->len;
        }
    }

    //send data
    if(mac_send(buf, tot_len) != tot_len)
    {
		Fwl_Print(C2, M_NETWORK, "mac_send  ERR_BUF");
        return ERR_BUF;
    }

    return ERR_OK;
}

/** low_level_input(): Allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 */
struct pbuf *
macif_low_level_input(struct netif *netif, const void *packet, int packet_len)
{
    struct pbuf *p, *q;

    int start;
    int length = packet_len;
    
    struct eth_addr *dest = (struct eth_addr*)packet;
    struct eth_addr *src = dest + 1;
    
    int unicast;
    const u8_t bcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    const u8_t ipv4mcast[] = {0x01, 0x00, 0x5e};
    const u8_t ipv6mcast[] = {0x33, 0x33};

	AK_ASSERT_PTR(netif, "macif_low_level_input(): netif error", AK_NULL);
	AK_ASSERT_PTR(packet, "macif_low_level_input(): packet error", AK_NULL);
    
    /* Don't let feedback packets through */
    if(!memcmp(src, netif->hwaddr, ETHARP_HWADDR_LEN))
    {
        return AK_NULL;
    }

    /* MAC filter: only let my MAC or non-unicast through*/
    unicast = ((dest->addr[0] & 0x01) == 0);
    if (memcmp(dest, &netif->hwaddr, ETHARP_HWADDR_LEN) &&
        (memcmp(dest, ipv4mcast, 3) || ((dest->addr[3] & 0x80) != 0)) &&
        memcmp(dest, ipv6mcast, 2) &&
        memcmp(dest, bcast, 6))
    {
        return AK_NULL;
    }

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, (u16_t)length + ETH_PAD_SIZE, PBUF_POOL);
    if (p != NULL)
    {
        start=0;
        for (q = p; q != AK_NULL; q = q->next)
        {
            u16_t copy_len = q->len;
            if (q == p)
            {
                memcpy(&((char*)q->payload)[ETH_PAD_SIZE], &((char*)packet)[start], copy_len);
            }
            else
            {
                memcpy(q->payload, &((char*)packet)[start], copy_len);
            }

            start += copy_len;
            length -= copy_len;

            if (length <= 0)
                break;
        }
    }
    else
    {
    }

    return p;
}


err_t macif_init(struct netif *netif)
{
	AK_ASSERT_PTR(netif, "macif_init(): netif error", ERR_VAL);
	
	netif->linkoutput = macif_low_level_output;

	netif->output = etharp_output;

	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* sets link up or down based on current status */
	macif_low_level_init(netif);

	return ERR_OK;
}

void macif_set_linkstate(T_U8 link_state)
{
	m_mac_if.link_state = link_state;
}

T_BOOL macif_set_macaddr(T_U8 *pmacaddr)
{
	AK_ASSERT_PTR(pmacaddr, "macif_set_macaddr(): pmac_addr error", AK_FALSE);
	
	memcpy(m_mac_if.mac_addr, pmacaddr, ETHARP_HWADDR_LEN);

	return AK_TRUE;
}

#endif

