/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/skbuff.h
 * Socket buffer (skb) definitions
 */

#ifndef _LINUXAB_SKBUFF_H
#define _LINUXAB_SKBUFF_H

#include "types.h"
#include "atomic.h"

#define SKB_DATA_ALIGN(X)   ALIGN(X, 64)
#define SKB_MAX_HEAD        256

struct sk_buff_head {
    struct sk_buff *next;
    struct sk_buff *prev;
    uint32_t        qlen;
};

struct sk_buff {
    struct sk_buff *next;
    struct sk_buff *prev;
    
    struct sk_buff_head *list;
    
    uint64_t        tstamp;
    struct sock    *sk;
    struct net_device *dev;
    
    char           *cb;         /* Control buffer */
    uint32_t        len;
    uint32_t        data_len;
    uint16_t        mac_len;
    uint16_t        hdr_len;
    
    uint16_t        queue_mapping;
    uint8_t         cloned:1,
                    ip_summed:2,
                    pkt_type:3;
    
    uint32_t        priority;
    atomic_t        users;
    
    uint32_t        truesize;
    uint8_t        *head;       /* Start of buffer */
    uint8_t        *data;       /* Start of data */
    uint8_t        *tail;       /* End of data */
    uint8_t        *end;        /* End of buffer */
    
    /* Layer headers */
    union {
        struct ethhdr   *eth_hdr;
        struct iphdr    *ip_hdr;
        struct tcphdr   *tcp_hdr;
        struct udphdr   *udp_hdr;
    } nh;
    
    void (*destructor)(struct sk_buff *skb);
};

/* Packet types */
#define PACKET_HOST         0
#define PACKET_BROADCAST    1
#define PACKET_MULTICAST    2
#define PACKET_OTHERHOST    3
#define PACKET_OUTGOING     4
#define PACKET_LOOPBACK     5

/* IP checksum states */
#define CHECKSUM_NONE       0
#define CHECKSUM_UNNECESSARY 1
#define CHECKSUM_COMPLETE   2
#define CHECKSUM_PARTIAL    3

struct sk_buff *alloc_skb(unsigned int size, int priority);
struct sk_buff *dev_alloc_skb(unsigned int length);
void kfree_skb(struct sk_buff *skb);
struct sk_buff *skb_clone(struct sk_buff *skb, int priority);
struct sk_buff *skb_copy(const struct sk_buff *skb, int priority);

static inline unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
    unsigned char *tmp = skb->tail;
    skb->tail += len;
    skb->len += len;
    return tmp;
}

static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
    skb->data -= len;
    skb->len += len;
    return skb->data;
}

static inline unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
    skb->len -= len;
    return skb->data += len;
}

static inline void skb_reserve(struct sk_buff *skb, int len)
{
    skb->data += len;
    skb->tail += len;
}

static inline unsigned int skb_headroom(const struct sk_buff *skb)
{
    return skb->data - skb->head;
}

static inline unsigned int skb_tailroom(const struct sk_buff *skb)
{
    return skb->end - skb->tail;
}

static inline void skb_reset_mac_header(struct sk_buff *skb)
{
    skb->nh.eth_hdr = (struct ethhdr *)skb->data;
}

static inline void skb_set_mac_header(struct sk_buff *skb, const int offset)
{
    skb->nh.eth_hdr = (struct ethhdr *)(skb->data + offset);
}

static inline void skb_reset_network_header(struct sk_buff *skb)
{
    skb->nh.ip_hdr = (struct iphdr *)skb->data;
}

static inline unsigned char *skb_network_header(const struct sk_buff *skb)
{
    return (unsigned char *)skb->nh.ip_hdr;
}

static inline void skb_reset_transport_header(struct sk_buff *skb)
{
    skb->nh.tcp_hdr = (struct tcphdr *)skb->data;
}

static inline unsigned char *skb_transport_header(const struct sk_buff *skb)
{
    return (unsigned char *)skb->nh.tcp_hdr;
}

void skb_queue_head_init(struct sk_buff_head *list);
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);
struct sk_buff *skb_dequeue(struct sk_buff_head *list);
void skb_queue_purge(struct sk_buff_head *list);
unsigned int skb_queue_len(const struct sk_buff_head *list_);

#endif /* _LINUXAB_SKBUFF_H */