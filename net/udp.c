// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/udp.c
 * UDP protocol implementation
 */

#include "af_inet.h"
#include "skbuff.h"
#include "printk.h"

struct udp_sock {
    struct inet_sock inet;
};

int udp_sendmsg(struct inet_sock *inet, struct msghdr *msg, size_t size)
{
    struct sk_buff *skb;
    struct udphdr *uh;
    struct iovec *iov = msg->msg_iov;
    
    skb = alloc_skb(size + 128, 0);
    if (!skb) return -1;
    
    skb_reserve(skb, 128);
    
    /* Copy data */
    memcpy(skb_put(skb, iov->iov_len), iov->iov_base, iov->iov_len);
    
    /* UDP header */
    skb_push(skb, sizeof(struct udphdr));
    uh = (struct udphdr *)skb->data;
    
    uh->source = inet->inet_sport;
    uh->dest = inet->inet_dport;
    uh->len = htons(skb->len);
    uh->check = 0; /* Optional in IPv4 */
    
    return ip_build_and_send_pkt(skb, (void *)inet,
                                  inet->inet_saddr,
                                  inet->inet_daddr,
                                  NULL);
}

int udp_recvmsg(struct inet_sock *inet, struct msghdr *msg, size_t size, int flags)
{
    /* TODO: Dequeue from receive queue */
    return 0;
}

int udp_rcv(struct sk_buff *skb)
{
    struct udphdr *uh = (struct udphdr *)skb->data;
    int len = ntohs(uh->len);
    
    printk(KERN_DEBUG "udp_rcv: sport=%d dport=%d len=%d\n",
           ntohs(uh->source), ntohs(uh->dest), len);
    
    skb_pull(skb, sizeof(*uh));
    
    /* TODO: Lookup socket and deliver */
    kfree_skb(skb);
    return 0;
}