// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ethernet/eth.c
 * Ethernet protocol layer
 */

#include "types.h"
#include "skbuff.h"
#include "dev.h"
#include "printk.h"

struct ethhdr {
    uint8_t  h_dest[6];
    uint8_t  h_source[6];
    uint16_t h_proto;
} __attribute__((packed));

/* Protocol handlers */
static void (*eth_protocol_handlers[16])(struct sk_buff *) = {0};

int eth_header(struct sk_buff *skb, struct net_device *dev,
               uint16_t type, const void *daddr,
               const void *saddr, unsigned len)
{
    struct ethhdr *eth = (struct ethhdr *)skb_push(skb, sizeof(*eth));
    
    eth->h_proto = htons(type);
    
    if (daddr)
        memcpy(eth->h_dest, daddr, 6);
    else
        memset(eth->h_dest, 0xFF, 6); /* Broadcast */
    
    if (saddr)
        memcpy(eth->h_source, saddr, 6);
    else
        memcpy(eth->h_source, dev->dev_addr, 6);
    
    return sizeof(*eth);
}

int eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    struct ethhdr *eth = (struct ethhdr *)skb->data;
    
    skb->dev = dev;
    skb->protocol = eth->h_proto;
    
    if (memcmp(eth->h_dest, dev->dev_addr, 6) == 0)
        skb->pkt_type = PACKET_HOST;
    else if (memcmp(eth->h_dest, "\xff\xff\xff\xff\xff\xff", 6) == 0)
        skb->pkt_type = PACKET_BROADCAST;
    else
        skb->pkt_type = PACKET_OTHERHOST;
    
    skb_pull(skb, sizeof(*eth));
    
    return skb->protocol;
}

void eth_rcv(struct sk_buff *skb)
{
    uint16_t proto = eth_type_trans(skb, skb->dev);
    
    switch (ntohs(proto)) {
    case 0x0800:
        ip_rcv(skb);
        break;
    case 0x0806:
        /* ARP */
        printk(KERN_DEBUG "eth_rcv: ARP packet\n");
        kfree_skb(skb);
        break;
    default:
        printk(KERN_DEBUG "eth_rcv: unknown proto 0x%04x\n", ntohs(proto));
        kfree_skb(skb);
        break;
    }
}