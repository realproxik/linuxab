// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/ipv4/af_inet.c
 * IPv4 protocol family implementation
 */

#include "af_inet.h"
#include "printk.h"

static struct proto_ops inet_stream_ops = {
    .family     = AF_INET,
    .create     = inet_create,
    .bind       = inet_bind,
    .listen     = inet_listen,
    .accept     = inet_accept,
    .connect    = inet_connect,
    .sendmsg    = inet_sendmsg,
    .recvmsg    = inet_recvmsg,
    .shutdown   = NULL,
    .setsockopt = NULL,
    .getsockopt = NULL,
};

static struct proto_ops inet_dgram_ops = {
    .family     = AF_INET,
    .create     = inet_create,
    .bind       = inet_bind,
    .connect    = inet_connect,
    .sendmsg    = inet_sendmsg,
    .recvmsg    = inet_recvmsg,
};

int inet_create(struct socket *sock, int protocol)
{
    struct inet_sock *inet;
    struct proto *answer;
    
    printk(KERN_DEBUG "inet_create: type=%d protocol=%d\n", sock->type, protocol);
    
    inet = kmalloc_page();
    if (!inet) return -1;
    
    memset(inet, 0, sizeof(*inet));
    
    switch (sock->type) {
    case SOCK_STREAM:
        if (protocol && protocol != IPPROTO_TCP)
            return -1;
        sock->ops = &inet_stream_ops;
        break;
    case SOCK_DGRAM:
        if (protocol && protocol != IPPROTO_UDP)
            return -1;
        sock->ops = &inet_dgram_ops;
        break;
    default:
        return -1;
    }
    
    inet->uc_ttl = 64;
    inet->sk_socket = sock;
    sock->sk = (void *)inet;
    
    return 0;
}

int inet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
    struct inet_sock *inet = (struct inet_sock *)sock->sk;
    struct sockaddr_in *addr = (struct sockaddr_in *)uaddr;
    
    if (addr_len < sizeof(*addr))
        return -1;
    
    if (addr->sin_family != AF_INET)
        return -1;
    
    inet->inet_rcv_saddr = addr->sin_addr.s_addr;
    inet->inet_saddr = addr->sin_addr.s_addr;
    inet->inet_sport = addr->sin_port;
    
    printk(KERN_DEBUG "inet_bind: %pI4:%d\n", &addr->sin_addr.s_addr, ntohs(addr->sin_port));
    return 0;
}

int inet_listen(struct socket *sock, int backlog)
{
    sock->state = SS_CONNECTING;
    return 0;
}

int inet_accept(struct socket *sock, struct socket *newsock, int flags)
{
    /* TODO: Accept connection from listen queue */
    return 0;
}

int inet_connect(struct socket *sock, struct sockaddr *uaddr,
                 int addr_len, int flags)
{
    struct inet_sock *inet = (struct inet_sock *)sock->sk;
    struct sockaddr_in *addr = (struct sockaddr_in *)uaddr;
    
    inet->inet_daddr = addr->sin_addr.s_addr;
    inet->inet_dport = addr->sin_port;
    sock->state = SS_CONNECTED;
    
    printk(KERN_DEBUG "inet_connect: -> %pI4:%d\n",
           &addr->sin_addr.s_addr, ntohs(addr->sin_port));
    return 0;
}

int inet_sendmsg(struct socket *sock, struct msghdr *msg, size_t size)
{
    struct inet_sock *inet = (struct inet_sock *)sock->sk;
    struct sk_buff *skb;
    struct sockaddr_in *addr = (struct sockaddr_in *)msg->msg_name;
    
    if (sock->type == SOCK_DGRAM) {
        /* UDP */
        return udp_sendmsg(inet, msg, size);
    } else if (sock->type == SOCK_STREAM) {
        /* TCP */
        return tcp_sendmsg(inet, msg, size);
    }
    
    return -1;
}

int inet_recvmsg(struct socket *sock, struct msghdr *msg, size_t size, int flags)
{
    struct inet_sock *inet = (struct inet_sock *)sock->sk;
    
    if (sock->type == SOCK_DGRAM)
        return udp_recvmsg(inet, msg, size, flags);
    else if (sock->type == SOCK_STREAM)
        return tcp_recvmsg(inet, msg, size, flags);
    
    return -1;
}

/* IP checksum */
uint16_t ip_fast_csum(const void *iph, unsigned int ihl)
{
    const uint32_t *ptr = iph;
    uint32_t sum = 0;
    
    for (unsigned int i = 0; i < ihl; i++)
        sum += ptr[i];
    
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    
    return ~sum;
}

uint16_t ip_compute_csum(const void *buff, int len)
{
    const uint16_t *ptr = buff;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len == 1)
        sum += *(const uint8_t *)ptr;
    
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    
    return ~sum;
}