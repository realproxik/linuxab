/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/af_netlink.h
 * Netlink socket definitions
 */

#ifndef _LINUXAB_AF_NETLINK_H
#define _LINUXAB_AF_NETLINK_H

#include "socket.h"

#define NETLINK_ROUTE       0
#define NETLINK_UNUSED      1
#define NETLINK_USERSOCK    2
#define NETLINK_FIREWALL    3
#define NETLINK_SOCK_DIAG   4
#define NETLINK_NFLOG       5
#define NETLINK_XFRM        6
#define NETLINK_SELINUX     7
#define NETLINK_ISCSI       8
#define NETLINK_AUDIT       9
#define NETLINK_FIB_LOOKUP  10
#define NETLINK_CONNECTOR   11
#define NETLINK_NETFILTER   12
#define NETLINK_IP6_FW      13
#define NETLINK_DNRTMSG     14
#define NETLINK_KOBJECT_UEVENT 15
#define NETLINK_GENERIC     16
#define NETLINK_SCSITRANSPORT 18
#define NETLINK_ECRYPTFS    19
#define NETLINK_RDMA        20
#define NETLINK_CRYPTO      21
#define NETLINK_SMC         22

#define NLMSG_ALIGNTO       4
#define NLMSG_ALIGN(len)    (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN        ((int)NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len)   ((len) + NLMSG_ALIGN(NLMSG_HDRLEN))
#define NLMSG_SPACE(len)    NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)     ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len) ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                             (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len)   ((len) >= (int)sizeof(struct nlmsghdr) && \
                             (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
                             (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP          0x1
#define NLMSG_ERROR         0x2
#define NLMSG_DONE          0x3
#define NLMSG_OVERRUN       0x4

#define NLM_F_REQUEST       0x001
#define NLM_F_MULTI         0x002
#define NLM_F_ACK           0x004
#define NLM_F_ECHO          0x008
#define NLM_F_DUMP_INTR     0x010
#define NLM_F_DUMP_FILTERED 0x020

#define NLM_F_ROOT      0x100
#define NLM_F_MATCH     0x200
#define NLM_F_ATOMIC    0x400
#define NLM_F_DUMP      (NLM_F_ROOT|NLM_F_MATCH)

#define NLM_F_REPLACE   0x100
#define NLM_F_EXCL      0x200
#define NLM_F_CREATE    0x400
#define NLM_F_APPEND    0x800

struct nlmsghdr {
    uint32_t nlmsg_len;
    uint16_t nlmsg_type;
    uint16_t nlmsg_flags;
    uint32_t nlmsg_seq;
    uint32_t nlmsg_pid;
};

struct nlmsgerr {
    int      error;
    struct nlmsghdr msg;
};

struct sockaddr_nl {
    uint16_t nl_family;
    uint16_t nl_pad;
    uint32_t nl_pid;
    uint32_t nl_groups;
};

int netlink_create(struct socket *sock, int protocol);
int netlink_sendmsg(struct socket *sock, struct msghdr *msg, size_t len);
int netlink_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags);

#endif /* _LINUXAB_AF_NETLINK_H */