// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/icmp.c
 * ICMP protocol implementation
 */

#include "af_inet.h"
#include "skbuff.h"
#include "printk.h"

int icmp_rcv(struct sk_buff *skb)
{
    struct icmphdr *icmph = (struct icmphdr *)skb->data;
    
    printk(KERN_DEBUG "icmp_rcv: type=%d code=%d\n", icmph->type, icmph->code);
    
    switch (icmph->type) {
    case ICMP_ECHO:
        /* Send echo reply */
        {
            uint32_t tmp;
            struct iphdr *iph = (struct iphdr *)(skb->data - sizeof(struct iphdr) - (skb->nh.ip_hdr->ihl * 4));
            
            /* Swap addresses */
            tmp = iph->daddr;
            iph->daddr = iph->saddr;
            iph->saddr = tmp;
            
            icmph->type = ICMP_ECHOREPLY;
            icmph->checksum = 0;
            icmph->checksum = ip_compute_csum(icmph, skb->len);
            
            /* Send back */
            skb->pkt_type = PACKET_OUTGOING;
            ip_output(skb);
        }
        break;
        
    default:
        kfree_skb(skb);
        break;
    }
    
    return 0;
}