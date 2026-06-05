// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/ip_input.c
 * IP packet reception
 */

#include "af_inet.h"
#include "dev.h"
#include "printk.h"

int ip_rcv(struct sk_buff *skb)
{
    struct iphdr *iph;
    uint32_t len;
    
    if (!skb) return -1;
    
    iph = (struct iphdr *)skb->data;
    
    /* Version check */
    if (iph->version != 4) {
        kfree_skb(skb);
        return -1;
    }
    
    /* Header length */
    if (iph->ihl < 5) {
        kfree_skb(skb);
        return -1;
    }
    
    /* Length check */
    len = ntohs(iph->tot_len);
    if (skb->len < len) {
        kfree_skb(skb);
        return -1;
    }
    
    /* Checksum verification (optional for performance) */
    if (ip_compute_csum(iph, iph->ihl * 4) != 0) {
        printk(KERN_WARNING "IP checksum failed from %pI4\n", &iph->saddr);
        /* Some NICs do checksum offload, so we might skip */
    }
    
    printk(KERN_DEBUG "ip_rcv: %pI4 -> %pI4, proto=%d, len=%d\n",
           &iph->saddr, &iph->daddr, iph->protocol, len);
    
    /* Pull IP header */
    skb_pull(skb, iph->ihl * 4);
    
    /* Fragment reassembly would go here */
    
    /* Deliver to upper layer */
    return ip_local_deliver(skb);
}

int ip_local_deliver(struct sk_buff *skb)
{
    struct iphdr *iph = (struct iphdr *)(skb->data - (iph->ihl * 4));
    
    switch (iph->protocol) {
    case IPPROTO_TCP:
        return tcp_rcv(skb);
    case IPPROTO_UDP:
        return udp_rcv(skb);
    case IPPROTO_ICMP:
        return icmp_rcv(skb);
    default:
        printk(KERN_DEBUG "IP: unknown protocol %d\n", iph->protocol);
        kfree_skb(skb);
        return -1;
    }
}