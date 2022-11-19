
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


// 创建新链表
ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    ngx_list_t  *list;

    // 从内存池中创建新链表的 ngx_list_t 结构
    list = ngx_palloc(pool, sizeof(ngx_list_t));
    if (list == NULL) {
        return NULL;
    }

    // 初始化内嵌在 ngx_list_t 结构里的首个区块，大小为 n * size
    // 以及填入 ngx_list_t 的其他字段
    if (ngx_list_init(list, pool, n, size) != NGX_OK) {
        return NULL;
    }

    return list;
}


// 在链表中追加一个新元素
// 这里有点反直觉，新元素不是作为参数传进来的，而是作为返回值移交出去，需要由调用者自行填充新元素的取值
// 给这个函数取个别名 ngx_alloc_in_list 可能更好理解
void *
ngx_list_push(ngx_list_t *l)
{
    void             *elt;
    ngx_list_part_t  *last;

    last = l->last;

    if (last->nelts == l->nalloc) {

        /* the last part is full, allocate a new list part */
        // 满了，建新区块
        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t));
        if (last == NULL) {
            return NULL;
        }

        // 新区块里的数组，注意大小是 nalloc * size
        last->elts = ngx_palloc(l->pool, l->nalloc * l->size);
        if (last->elts == NULL) {
            return NULL;
        }

        last->nelts = 0;  // 下边退出循环后做了自增，所以最后 nelts=1
        last->next = NULL;

        // 把之前的最后一个区块连上当前的新区块，并修改last区块字段
        l->last->next = last;
        l->last = last;
    }

    // 直接对 elts 指针做偏移，得到对应元素的地址
    elt = (char *) last->elts + l->size * last->nelts;
    last->nelts++;

    return elt;
}

// 值得注意的是，没有 ngx_list_pop
// OpenResty 好像也有类似设计，内存在同一请求内不会太主动释放
// 等到请求结束才一起随着 ngx_pool_t 内存池一块释放
// 所以对象池的概念在这里很有用：虽然请求结束后会释放，但请求结束之前不注意这些零碎内存的话，就会造成“临时的内存泄露”
