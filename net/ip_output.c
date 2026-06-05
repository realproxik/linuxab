// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/ip_output.c
 * IP packet transmission
 */

#include "af_inet.h"
#include "dev.h"
#include "printk.h"

int ip_build_and_send_pkt(struct sk_buff *skb, struct sock *sk,
                          uint32_t saddr, uint32_t daddr,
                          struct ip_options *opt)
{
    struct iphdr *iph;
    
    skb_push(skb, sizeof(struct iphdr));
    skb_reset_network_header(skb);
    iph = (struct iphdr *)skb->data;
    
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(skb->len);
    iph->id = 0; /* TODO: Allocate ID */
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = skb->sk ? ((struct inet_sock *)skb->sk)->inet_num : 0;
    iph->check = 0;
    iph->saddr = saddr;
    iph->daddr = daddr;
    
    iph->check = ip_fast_csum(iph, iph->ihl);
    
    return ip_output(skb);
}

int ip_output(struct sk_buff *skb)
{
    struct net_device *dev;
    struct iphdr *iph = (struct iphdr *)skb_network_header(skb);
    
    /* TODO: Route lookup */
    dev = dev_get_by_name("eth0");
    if (!dev) {
        kfree_skb(skb);
        return -1;
    }
    
    skb->dev = dev;
    
    /* Decrement TTL */
    iph->ttl--;
    if (iph->ttl <= 0) {
        /* Send ICMP time exceeded */
        kfree_skb(skb);
        return -1;
    }
    
    /* Recompute checksum if TTL changed */
    iph->check = 0;
    iph->check = ip_fast_csum(iph, iph->ihl);
    
    /* Ethernet header */
    skb_push(skb, 14);
    skb->protocol = htons(0x0800);
    
    return dev_queue_xmit(skb);
}

int ip_queue_xmit(struct sk_buff *skb)
{
    return ip_output(skb);
}