// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/tcp.c
 * TCP protocol implementation (simplified)
 */

#include "af_inet.h"
#include "skbuff.h"
#include "printk.h"

#define TCP_SYN_SENT    1
#define TCP_SYN_RECV    2
#define TCP_ESTABLISHED 3
#define TCP_FIN_WAIT1   4
#define TCP_FIN_WAIT2   5
#define TCP_CLOSE_WAIT  6
#define TCP_CLOSING     7
#define TCP_LAST_ACK    8
#define TCP_TIME_WAIT   9
#define TCP_CLOSE       10

struct tcp_sock {
    struct inet_sock inet;
    
    uint32_t snd_nxt;
    uint32_t snd_una;
    uint32_t rcv_nxt;
    uint32_t rcv_wnd;
    
    uint16_t state;
    uint16_t mss;
    
    struct sk_buff_head write_queue;
    struct sk_buff_head receive_queue;
};

int tcp_sendmsg(struct inet_sock *inet, struct msghdr *msg, size_t size)
{
    struct sk_buff *skb;
    struct iovec *iov = msg->msg_iov;
    
    skb = alloc_skb(size + 128, 0);
    if (!skb) return -1;
    
    skb_reserve(skb, 128);
    
    /* Copy data */
    memcpy(skb_put(skb, iov->iov_len), iov->iov_base, iov->iov_len);
    
    /* Build TCP header */
    skb_push(skb, sizeof(struct tcphdr));
    skb_reset_transport_header(skb);
    
    struct tcphdr *th = (struct tcphdr *)skb->data;
    memset(th, 0, sizeof(*th));
    th->source = inet->inet_sport;
    th->dest = inet->inet_dport;
    th->seq = 0; /* TODO */
    th->ack_seq = 0;
    th->doff = sizeof(*th) / 4;
    th->psh = 1;
    th->ack = 1;
    th->window = htons(65535);
    
    /* Pseudo-header checksum */
    th->check = 0;
    /* TODO: Compute TCP checksum */
    
    return ip_build_and_send_pkt(skb, (void *)inet,
                                  inet->inet_saddr,
                                  inet->inet_daddr,
                                  NULL);
}

int tcp_recvmsg(struct inet_sock *inet, struct msghdr *msg, size_t size, int flags)
{
    /* TODO: Dequeue from receive queue */
    return 0;
}

int tcp_rcv(struct sk_buff *skb)
{
    struct tcphdr *th = (struct tcphdr *)skb->data;
    int thlen = th->doff * 4;
    
    printk(KERN_DEBUG "tcp_rcv: sport=%d dport=%d seq=%u ack=%u flags=%c%c%c\n",
           ntohs(th->source), ntohs(th->dest),
           ntohl(th->seq), ntohl(th->ack_seq),
           th->syn ? 'S' : '.',
           th->ack ? 'A' : '.',
           th->fin ? 'F' : '.');
    
    skb_pull(skb, thlen);
    
    /* TODO: State machine, queue to socket */
    kfree_skb(skb);
    return 0;
}