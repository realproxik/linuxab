// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/net/core/skbuff.c
 * Socket buffer implementation
 */

#include "skbuff.h"
#include "printk.h"

struct sk_buff *alloc_skb(unsigned int size, int priority)
{
    struct sk_buff *skb;
    uint8_t *data;
    unsigned int alloc_size;
    
    alloc_size = SKB_DATA_ALIGN(sizeof(struct sk_buff));
    alloc_size += SKB_DATA_ALIGN(size);
    
    skb = kmalloc_page();
    if (!skb) return NULL;
    
    data = (uint8_t *)skb + SKB_DATA_ALIGN(sizeof(struct sk_buff));
    
    memset(skb, 0, sizeof(struct sk_buff));
    
    skb->head = data;
    skb->data = data;
    skb->tail = data;
    skb->end = data + size;
    skb->len = 0;
    skb->data_len = 0;
    skb->truesize = alloc_size;
    atomic_set(&skb->users, 1);
    
    return skb;
}

struct sk_buff *dev_alloc_skb(unsigned int length)
{
    struct sk_buff *skb = alloc_skb(length + 64, 0);
    if (skb) skb_reserve(skb, 64);
    return skb;
}

void kfree_skb(struct sk_buff *skb)
{
    if (!skb) return;
    
    if (!atomic_dec_and_test(&skb->users))
        return;
    
    if (skb->destructor)
        skb->destructor(skb);
    
    kfree_page(skb);
}

struct sk_buff *skb_clone(struct sk_buff *skb, int priority)
{
    struct sk_buff *n;
    
    n = alloc_skb(skb->len, priority);
    if (!n) return NULL;
    
    memcpy(skb_put(n, skb->len), skb->data, skb->len);
    n->dev = skb->dev;
    n->protocol = skb->protocol;
    n->pkt_type = skb->pkt_type;
    n->priority = skb->priority;
    
    return n;
}

struct sk_buff *skb_copy(const struct sk_buff *skb, int priority)
{
    struct sk_buff *n;
    unsigned int headerlen = skb->data - skb->head;
    
    n = alloc_skb(skb->len + headerlen, priority);
    if (!n) return NULL;
    
    skb_reserve(n, headerlen);
    memcpy(skb_put(n, skb->len), skb->data, skb->len);
    
    return n;
}

void skb_queue_head_init(struct sk_buff_head *list)
{
    list->prev = (struct sk_buff *)list;
    list->next = (struct sk_buff *)list;
    list->qlen = 0;
}

void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
    struct sk_buff *prev, *next;
    
    newsk->list = list;
    list->qlen++;
    
    prev = (struct sk_buff *)list;
    next = list->next;
    newsk->next = next;
    newsk->prev = prev;
    next->prev = newsk;
    prev->next = newsk;
}

struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
    struct sk_buff *skb = list->next;
    
    if (skb == (struct sk_buff *)list)
        return NULL;
    
    list->next = skb->next;
    skb->next->prev = (struct sk_buff *)list;
    list->qlen--;
    
    skb->list = NULL;
    return skb;
}

void skb_queue_purge(struct sk_buff_head *list)
{
    struct sk_buff *skb;
    
    while ((skb = skb_dequeue(list)) != NULL)
        kfree_skb(skb);
}

unsigned int skb_queue_len(const struct sk_buff_head *list_)
{
    return list_->qlen;
}