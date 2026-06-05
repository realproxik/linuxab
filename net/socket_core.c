// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/socket.c
 * Socket core implementation
 */

#include "socket.h"
#include "printk.h"

static struct socket *sock_alloc(void)
{
    struct socket *sock = kmalloc_page();
    if (!sock) return NULL;
    
    sock->state = SS_FREE;
    atomic_set(&sock->refcnt, 1);
    sock->sk = NULL;
    return sock;
}

static void sock_free(struct socket *sock)
{
    if (sock->sk) {
        /* TODO: sk_free(sock->sk) */
    }
    kfree_page(sock);
}

int sock_create(int family, int type, int protocol, struct socket **res)
{
    struct socket *sock;
    int err = -1;
    
    printk(KERN_DEBUG "sock_create: family=%d type=%d proto=%d\n", family, type, protocol);
    
    if (family < 0 || family >= AF_MAX)
        return -1; /* EAFNOSUPPORT */
    
    sock = sock_alloc();
    if (!sock)
        return -1; /* ENOMEM */
    
    sock->family = family;
    sock->type = type;
    
    switch (family) {
    case AF_INET:
        err = inet_create(sock, protocol);
        break;
    case AF_NETLINK:
        err = netlink_create(sock, protocol);
        break;
    default:
        err = -1; /* EAFNOSUPPORT */
        break;
    }
    
    if (err < 0) {
        sock_free(sock);
        return err;
    }
    
    sock->state = SS_UNCONNECTED;
    *res = sock;
    return 0;
}

int sock_release(struct socket *sock)
{
    if (!sock) return 0;
    
    if (sock->ops && sock->ops->shutdown)
        sock->ops->shutdown(sock, SHUT_RDWR);
    
    sock->state = SS_DISCONNECTING;
    
    if (atomic_dec_and_test(&sock->refcnt)) {
        sock_free(sock);
    }
    return 0;
}

int sock_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
    if (!sock->ops || !sock->ops->sendmsg)
        return -1; /* EOPNOTSUPP */
    return sock->ops->sendmsg(sock, msg, len);
}

int sock_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags)
{
    if (!sock->ops || !sock->ops->recvmsg)
        return -1; /* EOPNOTSUPP */
    return sock->ops->recvmsg(sock, msg, len, flags);
}

int sys_socket(int family, int type, int protocol)
{
    struct socket *sock;
    int err = sock_create(family, type, protocol, &sock);
    if (err < 0) return err;
    
    /* TODO: Allocate fd and return it */
    return 0;
}

int sys_bind(int fd, struct sockaddr *addr, int addrlen)
{
    /* TODO: Look up socket by fd */
    struct socket *sock = NULL;
    if (!sock || !sock->ops || !sock->ops->bind)
        return -1;
    return sock->ops->bind(sock, addr, addrlen);
}

int sys_listen(int fd, int backlog)
{
    struct socket *sock = NULL;
    if (!sock || !sock->ops || !sock->ops->listen)
        return -1;
    return sock->ops->listen(sock, backlog);
}

int sys_accept(int fd, struct sockaddr *addr, int *addrlen)
{
    struct socket *sock = NULL;
    struct socket *newsock;
    int err;
    
    if (!sock || !sock->ops || !sock->ops->accept)
        return -1;
    
    err = sock_create(sock->family, sock->type, 0, &newsock);
    if (err) return err;
    
    err = sock->ops->accept(sock, newsock, 0);
    if (err) {
        sock_release(newsock);
        return err;
    }
    
    /* TODO: Return new fd */
    return 0;
}

int sys_connect(int fd, struct sockaddr *addr, int addrlen)
{
    struct socket *sock = NULL;
    if (!sock || !sock->ops || !sock->ops->connect)
        return -1;
    return sock->ops->connect(sock, addr, addrlen, 0);
}

ssize_t sys_sendto(int fd, void *buf, size_t len, int flags,
                   struct sockaddr *addr, int addrlen)
{
    struct socket *sock = NULL;
    struct msghdr msg = {0};
    struct iovec iov;
    
    if (!sock) return -1;
    
    iov.iov_base = buf;
    iov.iov_len = len;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = addr;
    msg.msg_namelen = addrlen;
    
    return sock_sendmsg(sock, &msg, len);
}

ssize_t sys_recvfrom(int fd, void *buf, size_t len, int flags,
                     struct sockaddr *addr, int *addrlen)
{
    struct socket *sock = NULL;
    struct msghdr msg = {0};
    struct iovec iov;
    int err;
    
    if (!sock) return -1;
    
    iov.iov_base = buf;
    iov.iov_len = len;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = addr;
    msg.msg_namelen = addrlen ? *addrlen : 0;
    
    err = sock_recvmsg(sock, &msg, len, flags);
    if (addrlen) *addrlen = msg.msg_namelen;
    return err;
}