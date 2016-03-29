
#ifndef __MAC_H__
#define __MAC_H__

//number 0  is not allowed to define udisk msg
#define MAC_RECV_MSG            1
#define MAC_STATUS_MSG          2

//mac event
#define MAC_EVENT_LINK_UP       0x1
#define MAC_EVENT_LINK_DOWN     0x2

typedef struct tagMAC_MSG
{
    T_U8 event;
    T_U8 reserve1;
    T_U16 reserve2;
}
T_MAC_MSG;

/* rrd format */
typedef struct _RrdDescr_s {

	unsigned short  xsum;           /*  */

	unsigned short  nor     :4  ;   /* number of RFD */
	unsigned short  si      :12 ;   /* start index of rfd-ring */

	unsigned short  hash;           /* rss(MSFT) hash value */

	unsigned short  hash1;  

	unsigned short  vidh    :4  ;   /* vlan-id high part */
	unsigned short  cfi     :1  ;   /* vlan-cfi */ 
	unsigned short  pri     :3  ;   /* vlan-priority */
	unsigned short  vidl    :8  ;   /* vlan-id low part */
	unsigned char   hdr_len;        /* Header Length of Header-Data Split. unsigned short unit */
	unsigned char   hds_typ :2  ;   /* Header-Data Split Type, 
									00:no split, 
									01:split at upper layer protocol header
									10:split at upper layer payload */
	unsigned char   rss_cpu :2  ;   /* CPU number used by RSS */
	unsigned char   hash_t6 :1  ;   /* TCP(IPv6) flag for RSS hash algrithm */
	unsigned char   hash_i6 :1  ;   /* IPv6 flag for RSS hash algrithm */
	unsigned char   hash_t4 :1  ;   /* TCP(IPv4)  flag for RSS hash algrithm */
	unsigned char   hash_i4 :1  ;   /* IPv4 flag for RSS hash algrithm */

	unsigned short  frm_len :14 ;   /* frame length of the packet */        
	unsigned short  l4f     :1  ;   /* L4(TCP/UDP) checksum failed */
	unsigned short  ipf     :1  ;   /* IP checksum failed */
	unsigned short  vtag    :1  ;   /* vlan tag */
	unsigned short  pid     :3  ;   /* protocol id,
						  000: non-ip packet
						  001: ipv4(only)
						  011: tcp/ipv4
						  101: udp/ipv4
						  010: tcp/ipv6
						  100: udp/ipv6
						  110: ipv6(only) */
	unsigned short  res     :1  ;   /* received error summary */
	unsigned short  crc     :1  ;   /* crc error */
	unsigned short  fae     :1  ;   /* frame alignment error */
	unsigned short  trunc   :1  ;   /* truncated packet, larger than MTU */
	unsigned short  runt    :1  ;   /* runt packet */
	unsigned short  icmp    :1  ;   /* incomplete packet, due to insufficient rx-descriptor */
	unsigned short  bar     :1  ;   /* broadcast address received */
	unsigned short  mar     :1  ;   /* multicast address received */
	unsigned short  typ     :1  ;   /* type of packet (ethernet_ii(1) or snap(0)) */
	unsigned short  resv1   :2  ;   /* reserved, must be 0 */
	unsigned short  updt    :1  ;   /* update by hardware. after hw fulfill the buffer, this bit 
						  should be 1 */
} RrdDescr_t, *PRrdDescr_t;


/* Structure/enum declaration ------------------------------- */
typedef struct tagMac_Info
{
	T_U16		tx_pkt_cnt;
	T_U16		queue_pkt_len;
	T_U16		queue_start_addr;
	T_U16		queue_ip_summed;
	T_U16		dbug_cnt;
	T_U8		io_mode;		/* 0:word, 2:byte */
	T_U8		phy_addr;
	T_U8		imr_all;

	T_U32	flags;

	T_U32		rx_csum;
	T_U32		can_csum;
	T_U32		ip_summed;
	T_U32		phy_id;

	T_U8        dev_addr[6];
}
T_MAC_INFO;

/** 
    tpd: transmit packet descritor
    trd: transmit return descritor
    rfd: receive free descriptor
    rrd: receive return descriptor
 */
typedef struct tagMAC_DMA
{
    T_U8 *tpd_ring_base;
    T_U8 *tpd_fifo_base;

    T_U32 tpd_producer_index;
    T_U32 tpd_consumer_index;
    
    T_U8 *rfd_ring_base;
    T_U8 *rrd_ring_base;

    T_U8 *rfd_fifo_base;

    T_U32 rfd_producer_index;
    T_U32 rfd_consumer_index;

    T_U32 rrd_producer_index;
    T_U32 rrd_consumer_index;
}
T_MAC_DMA;

#endif
