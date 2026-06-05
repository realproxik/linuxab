/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/af_inet.h
 * IPv4 protocol family
 */

#ifndef _LINUXAB_AF_INET_H
#define _LINUXAB_AF_INET_H

#include "socket.h"
#include "skbuff.h"

/* IP protocols */
#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_IGMP    2
#define IPPROTO_IPIP    4
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_IPV6    41
#define IPPROTO_ESP     50
#define IPPROTO_AH      51
#define IPPROTO_ICMPV6  58
#define IPPROTO_RAW     255

/* IP flags */
#define IP_CE           0x8000
#define IP_DF           0x4000
#define IP_MF           0x2000
#define IP_OFFSET       0x1FFF

/* IP options */
#define IPOPT_COPY      0x80
#define IPOPT_CLASS_MASK 0x60
#define IPOPT_NUMBER_MASK 0x1f

struct iphdr {
    uint8_t  ihl:4,
             version:4;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

struct tcphdr {
    uint16_t source;
    uint16_t dest;
    uint32_t seq;
    uint32_t ack_seq;
    uint16_t res1:4,
             doff:4,
             fin:1,
             syn:1,
             rst:1,
             psh:1,
             ack:1,
             urg:1,
             ece:1,
             cwr:1;
    uint16_t window;
    uint16_t check;
    uint16_t urg_ptr;
};

struct udphdr {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
};

struct icmphdr {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
    } un;
};

#define ICMP_ECHOREPLY      0
#define ICMP_DEST_UNREACH   3
#define ICMP_SOURCE_QUENCH  4
#define ICMP_REDIRECT       5
#define ICMP_ECHO           8
#define ICMP_TIME_EXCEEDED  11
#define ICMP_PARAMETERPROB  12
#define ICMP_TIMESTAMP      13
#define ICMP_TIMESTAMPREPLY 14

struct inet_sock {
    struct socket   *sk_socket;
    uint32_t        inet_saddr;
    uint32_t        inet_rcv_saddr;
    uint16_t        inet_sport;
    uint32_t        inet_daddr;
    uint16_t        inet_dport;
    uint16_t        inet_num;
    int             uc_ttl;
    uint16_t        cmsg_flags;
};

struct inet_protosw {
    struct list_head list;
    unsigned short   type;
    unsigned short   protocol;
    struct proto    *prot;
    const struct proto_ops *ops;
    int              capability;
    char             no_check;
    unsigned char    flags;
};

int inet_create(struct socket *sock, int protocol);
int inet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len);
int inet_listen(struct socket *sock, int backlog);
int inet_accept(struct socket *sock, struct socket *newsock, int flags);
int inet_connect(struct socket *sock, struct sockaddr *uaddr, int addr_len, int flags);
int inet_sendmsg(struct socket *sock, struct msghdr *msg, size_t size);
int inet_recvmsg(struct socket *sock, struct msghdr *msg, size_t size, int flags);

int ip_rcv(struct sk_buff *skb);
int ip_local_deliver(struct sk_buff *skb);
int ip_output(struct sk_buff *skb);
int ip_queue_xmit(struct sk_buff *skb);

uint16_t ip_fast_csum(const void *iph, unsigned int ihl);
uint16_t ip_compute_csum(const void *buff, int len);

#endif /* _LINUXAB_AF_INET_H */