/***********************************************************
    >File Name: list.h
 > Author:    lujiefeng
 > Mail: jiefeng.lu@sirun.net
 > Create Time:2016年03月02日 星期三 12时56分11秒
**************************************************/
#ifndef COMMON_INC_LIST_H_
#define COMMON_INC_LIST_H_

#define _INLINE_ static inline

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)} 

//定义并初始化头结点head
#define LIST_HEAD(name)    \
    struct list_head name = LIST_HEAD_INIT(name)

//初始化头结点ptr,因此需要首先定义ptr
#define INIT_LIST_HEAD(ptr) do {\
    (ptr)->next = (ptr); (ptr)->prev = (ptr);\
} while (0)

_INLINE_ void __list_add(struct list_head *add,
        struct list_head *prev,
        struct list_head *next)
{
    next->prev = add;
    add->next = next;
    add->prev = prev;
    prev->next = add;
}

//每次添加节点到head之后，始终都是添加到头结点之后
_INLINE_ void list_add(struct list_head *add, struct list_head *head)
{
    __list_add(add, head, head->next);
}

//每次添加节点都是头结点之前，由于是循环链表，就是说添加到链表尾部
_INLINE_ void list_add_tail(struct list_head *add, struct list_head *head)
{
    __list_add(add, head->prev, head);
}

_INLINE_ void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

//删除节点
_INLINE_ void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}


//删除节点，并初始化被删除的结点（也就是使被删除的结点的prev和next都指向自己）
_INLINE_ void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

//判断链表是否为空
_INLINE_ int list_empty(struct list_head *head)
{
    return head->next == head;
}

//通过两个链表的head，进行连接
_INLINE_ void list_splice(struct list_head *list, struct list_head *head)
{
    struct list_head *first = list->next;

    if (first != list) {
        struct list_head *last = list->prev;
        struct list_head *at = head->next;

        first->prev = head;
        head->next = first;

        last->next = at;
        at->prev = last;
    }
}

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

//遍历链表，此时删除节点的操作可能会出错
#define list_for_each(pos, head)    \
    for (pos = (head)->next; pos != (head); pos = pos->next) 

 //遍历链表，可以同时有删除节点的操作
#define list_for_each_safe(pos, pnext, head)    \
    for (pos = (head)->next, pnext = pos->next; pos != (head); \
        pos = pnext, pnext = pos->next)

#undef _INLINE_
#endif

