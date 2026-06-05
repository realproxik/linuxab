// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/core/dev.c
 * Network device core implementation
 */

#include "dev.h"
#include "printk.h"

struct net_device *dev_base = NULL;
static int dev_count = 0;

int register_netdev(struct net_device *dev)
{
    struct net_device *d;
    
    printk(KERN_INFO "Registering netdev: %s\n", dev->name);
    
    if (!dev->name[0]) {
        /* Auto-generate name */
        snprintf(dev->name, IFNAMSIZ, "eth%d", dev_count);
    }
    
    /* Check for duplicate */
    for (d = dev_base; d; d = d->next) {
        if (!strcmp(d->name, dev->name)) {
            printk(KERN_ERR "Device %s already exists\n", dev->name);
            return -1; /* EEXIST */
        }
    }
    
    /* Initialize */
    if (dev->init && dev->init(dev) != 0) {
        printk(KERN_ERR "Device %s init failed\n", dev->name);
        return -1;
    }
    
    /* Add to list */
    dev->next = dev_base;
    dev_base = dev;
    dev_count++;
    
    printk(KERN_INFO "Registered %s, mtu=%d, addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
           dev->name, dev->mtu,
           dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
           dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
    
    return 0;
}

void unregister_netdev(struct net_device *dev)
{
    struct net_device **p;
    
    for (p = &dev_base; *p; p = &(*p)->next) {
        if (*p == dev) {
            *p = dev->next;
            dev_count--;
            break;
        }
    }
    
    if (dev->uninit)
        dev->uninit(dev);
    
    printk(KERN_INFO "Unregistered %s\n", dev->name);
}

struct net_device *dev_get_by_name(const char *name)
{
    struct net_device *dev;
    
    for (dev = dev_base; dev; dev = dev->next) {
        if (!strcmp(dev->name, name))
            return dev;
    }
    return NULL;
}

int dev_queue_xmit(struct sk_buff *skb)
{
    struct net_device *dev = skb->dev;
    
    if (!dev || !netif_running(dev)) {
        kfree_skb(skb);
        return -1; /* ENETDOWN */
    }
    
    if (!dev->hard_start_xmit) {
        kfree_skb(skb);
        return -1;
    }
    
    return dev->hard_start_xmit(skb, dev);
}

void netif_rx(struct sk_buff *skb)
{
    /* TODO: Queue to backlog and schedule softirq */
    /* For now, just process directly */
    if (skb->protocol == htons(0x0800)) {
        ip_rcv(skb);
    } else if (skb->protocol == htons(0x0806)) {
        /* ARP */
        arp_rcv(skb);
    } else {
        kfree_skb(skb);
    }
}

void netif_start_queue(struct net_device *dev)
{
    /* Clear TX stopped flag */
}

void netif_stop_queue(struct net_device *dev)
{
    /* Set TX stopped flag */
}

void netif_wake_queue(struct net_device *dev)
{
    netif_start_queue(dev);
    /* TODO: Wake any waiting transmitters */
}