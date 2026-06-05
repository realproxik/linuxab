/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/socket.h
 * Socket definitions
 */

#ifndef _LINUXAB_SOCKET_H
#define _LINUXAB_SOCKET_H

#include "types.h"
#include "atomic.h"

#define AF_UNSPEC       0
#define AF_UNIX         1
#define AF_INET         2
#define AF_INET6        10
#define AF_NETLINK      16
#define AF_PACKET       17
#define AF_MAX          32

#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#define SOCK_RDM        4
#define SOCK_SEQPACKET  5
#define SOCK_DCCP       6
#define SOCK_PACKET     10

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_IPV6    41

#define SOL_SOCKET      1
#define SO_DEBUG        1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_DONTROUTE    5
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_KEEPALIVE    9
#define SO_OOBINLINE    10
#define SO_LINGER       13
#define SO_REUSEPORT    15

#define MSG_OOB         1
#define MSG_PEEK        2
#define MSG_DONTROUTE   4
#define MSG_TRUNC       0x20
#define MSG_DONTWAIT    0x40
#define MSG_EOR         0x80
#define MSG_CONFIRM     0x800
#define MSG_ERRQUEUE    0x2000
#define MSG_NOSIGNAL    0x4000

#define SHUT_RD         0
#define SHUT_WR         1
#define SHUT_RDWR       2

struct sockaddr {
    uint16_t sa_family;
    char     sa_data[14];
};

struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    char     sin_zero[8];
};

struct in_addr {
    uint32_t s_addr;
};

#define INADDR_ANY      ((uint32_t)0x00000000)
#define INADDR_LOOPBACK ((uint32_t)0x7F000001)
#define INADDR_BROADCAST ((uint32_t)0xFFFFFFFF)

#define ntohs(x)        __builtin_bswap16(x)
#define htons(x)        __builtin_bswap16(x)
#define ntohl(x)        __builtin_bswap32(x)
#define htonl(x)        __builtin_bswap32(x)

struct msghdr {
    void         *msg_name;
    uint32_t      msg_namelen;
    struct iovec *msg_iov;
    uint64_t      msg_iovlen;
    void         *msg_control;
    uint64_t      msg_controllen;
    uint32_t      msg_flags;
};

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

struct socket {
    int           state;
    uint16_t      type;
    uint16_t      family;
    atomic_t      refcnt;
    void         *sk;           /* struct sock pointer */
    const struct proto_ops *ops;
};

struct proto_ops {
    int   family;
    int (*create)(struct socket *sock, int protocol);
    int (*bind)(struct socket *sock, struct sockaddr *addr, int addrlen);
    int (*connect)(struct socket *sock, struct sockaddr *addr, int addrlen, int flags);
    int (*listen)(struct socket *sock, int backlog);
    int (*accept)(struct socket *sock, struct socket *newsock, int flags);
    int (*getname)(struct socket *sock, struct sockaddr *addr, int *addrlen, int peer);
    int (*sendmsg)(struct socket *sock, struct msghdr *msg, size_t len);
    int (*recvmsg)(struct socket *sock, struct msghdr *msg, size_t len, int flags);
    int (*shutdown)(struct socket *sock, int how);
    int (*setsockopt)(struct socket *sock, int level, int optname, void *optval, int optlen);
    int (*getsockopt)(struct socket *sock, int level, int optname, void *optval, int *optlen);
    int (*ioctl)(struct socket *sock, unsigned int cmd, unsigned long arg);
    int (*mmap)(struct socket *sock, void *vma);
    int (*sendpage)(struct socket *sock, void *page, int offset, size_t size, int flags);
};

/* Socket states */
#define SS_FREE         0
#define SS_UNCONNECTED  1
#define SS_CONNECTING   2
#define SS_CONNECTED    3
#define SS_DISCONNECTING 4

int sock_create(int family, int type, int protocol, struct socket **res);
int sock_release(struct socket *sock);
int sock_sendmsg(struct socket *sock, struct msghdr *msg, size_t len);
int sock_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags);

#endif /* _LINUXAB_SOCKET_H */