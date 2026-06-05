/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/dev.h
 * Network device definitions
 */

#ifndef _LINUXAB_NETDEV_H
#define _LINUXAB_NETDEV_H

#include "types.h"
#include "skbuff.h"

#define IFNAMSIZ        16
#define NETDEV_ALIGN    32

#define NET_NAME_UNKNOWN    0
#define NET_NAME_ENUM       1
#define NET_NAME_PREDICTABLE 2
#define NET_NAME_USER       3
#define NET_NAME_RENAMED    4

/* Device flags */
#define IFF_UP          0x1
#define IFF_BROADCAST   0x2
#define IFF_DEBUG       0x4
#define IFF_LOOPBACK    0x8
#define IFF_POINTOPOINT 0x10
#define IFF_NOTRAILERS  0x20
#define IFF_RUNNING     0x40
#define IFF_NOARP       0x80
#define IFF_PROMISC     0x100
#define IFF_ALLMULTI    0x200
#define IFF_MASTER      0x400
#define IFF_SLAVE       0x800
#define IFF_MULTICAST   0x1000
#define IFF_PORTSEL     0x2000
#define IFF_AUTOMEDIA   0x4000
#define IFF_DYNAMIC     0x8000

/* Feature flags */
#define NETIF_F_SG              1
#define NETIF_F_IP_CSUM         2
#define NETIF_F_HW_CSUM         4
#define NETIF_F_IPV6_CSUM       8
#define NETIF_F_HIGHDMA         16
#define NETIF_F_FRAGLIST        32
#define NETIF_F_HW_VLAN_CTAG_TX 64
#define NETIF_F_HW_VLAN_CTAG_RX 128
#define NETIF_F_RXCSUM          256
#define NETIF_F_HW_TCP_LRO      512

enum netdev_tx {
    __NETDEV_TX_MIN = -100,
    NETDEV_TX_OK = 0,
    NETDEV_TX_BUSY = 16,
    NETDEV_TX_LOCKED = 32,
};

typedef enum netdev_tx netdev_tx_t;

struct net_device_stats {
    unsigned long rx_packets;
    unsigned long tx_packets;
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    unsigned long rx_errors;
    unsigned long tx_errors;
    unsigned long rx_dropped;
    unsigned long tx_dropped;
    unsigned long multicast;
    unsigned long collisions;
};

struct net_device {
    char                    name[IFNAMSIZ];
    char                    *ifalias;
    
    uint64_t                mem_end;
    uint64_t                mem_start;
    uint64_t                base_addr;
    uint32_t                irq;
    
    uint16_t                flags;
    uint64_t                features;
    
    uint32_t                mtu;
    uint16_t                type;
    uint16_t                hard_header_len;
    uint8_t                 dev_addr[6];
    uint8_t                 broadcast[6];
    
    struct net_device_stats stats;
    
    /* Device ops */
    int  (*init)(struct net_device *dev);
    void (*uninit)(struct net_device *dev);
    int  (*open)(struct net_device *dev);
    int  (*stop)(struct net_device *dev);
    netdev_tx_t (*hard_start_xmit)(struct sk_buff *skb,
                                    struct net_device *dev);
    int  (*set_config)(struct net_device *dev, void *map);
    void (*set_multicast_list)(struct net_device *dev);
    int  (*set_mac_address)(struct net_device *dev, void *addr);
    int  (*validate_addr)(struct net_device *dev);
    int  (*do_ioctl)(struct net_device *dev, void *ifr, int cmd);
    int  (*set_features)(struct net_device *dev, uint64_t features);
    
    struct net_device *next;
};

extern struct net_device *dev_base;

int register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);
struct net_device *dev_get_by_name(const char *name);
int dev_queue_xmit(struct sk_buff *skb);
void netif_rx(struct sk_buff *skb);
void netif_start_queue(struct net_device *dev);
void netif_stop_queue(struct net_device *dev);
void netif_wake_queue(struct net_device *dev);

static inline bool netif_running(const struct net_device *dev)
{
    return dev->flags & IFF_RUNNING;
}

static inline bool netif_device_present(const struct net_device *dev)
{
    return dev->flags & IFF_UP;
}

#endif /* _LINUXAB_NETDEV_H */