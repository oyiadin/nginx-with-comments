
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;

// 链表的组成部分，暂称为“区块”
struct ngx_list_part_s {
    void             *elts;     // 数组
                                // 数组占用空间 = size * nalloc
    ngx_uint_t        nelts;    // 已使用的元素数量，nelts <= nalloc
    ngx_list_part_t  *next;     // 下一个数组
};


// ngx链表，链表的元素是区块，区块里的数组才是存放最终数据的地方
typedef struct {
    ngx_list_part_t  *last;     // 链表尾部，方便 ngx_list_push 操作
    ngx_list_part_t   part;     // 链表首个区块
    size_t            size;     // 区块里元素的大小
    ngx_uint_t        nalloc;   // 每个区块存储元素数量的上限
    ngx_pool_t       *pool;     // 内存池对象
} ngx_list_t;


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

// 初始化链表，ngx_list_create 会调用
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    // 每个区块大小为 n * size
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;     // 已使用（已拥有）的元素数量
    list->part.next = NULL;   // 下一区块，置空
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  ngx_list_t 典型的迭代过程
 *  the iteration through the list:
 *
 *  拿到链表中的首个区块以及区块中的数组
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          如果当前数组迭代完毕，而且没有更多区块了，退出迭代过程
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          否则，切到下一个区块，继续循环处理区块中的 nelts 个元素
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      对每个元素的处理
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
