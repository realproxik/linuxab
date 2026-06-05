// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/netlink/af_netlink.c
 * Netlink socket implementation
 */

#include "af_netlink.h"
#include "printk.h"

static struct proto_ops netlink_ops = {
    .family     = AF_NETLINK,
    .create     = netlink_create,
    .sendmsg    = netlink_sendmsg,
    .recvmsg    = netlink_recvmsg,
};

struct netlink_sock {
    struct socket *sk_socket;
    uint32_t portid;
    uint32_t dst_portid;
    uint32_t dst_group;
    uint32_t groups;
    uint32_t protocol;
};

int netlink_create(struct socket *sock, int protocol)
{
    struct netlink_sock *nlk;
    
    nlk = kmalloc_page();
    if (!nlk) return -1;
    
    memset(nlk, 0, sizeof(*nlk));
    nlk->protocol = protocol;
    nlk->portid = 0; /* TODO: Allocate unique portid */
    
    sock->ops = &netlink_ops;
    sock->sk = (void *)nlk;
    
    printk(KERN_DEBUG "netlink_create: protocol=%d\n", protocol);
    return 0;
}

int netlink_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
    struct netlink_sock *nlk = (struct netlink_sock *)sock->sk;
    struct nlmsghdr *nlh = (struct nlmsghdr *)msg->msg_iov->iov_base;
    
    printk(KERN_DEBUG "netlink_sendmsg: type=%d flags=%x seq=%d\n",
           nlh->nlmsg_type, nlh->nlmsg_flags, nlh->nlmsg_seq);
    
    /* TODO: Route to kernel or other netlink sockets */
    return len;
}

int netlink_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags)
{
    /* TODO: Dequeue from receive queue */
    return 0;
}