/*
 * 一个简单的内存池，便于统一分配中和销毁内存
 *
 * 设计和思想来自于Nginx，主要区别是simple pool当前只是管理内存
 *
 */
#ifndef LEMON_MEMORY_POOL_H
#define LEMON_MEMORY_POOL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LEMON_ALIGN_PTR(p, a) \
        (unsigned char *) (((uint64_t ) (p) + ((uint64_t ) a - 1)) & ~((uint64_t ) a - 1))

typedef struct SimplePool SimplePool;

// 创建内存池
SimplePool* simple_pool_create(size_t size);

// 从内存池中申请内存 
void* simple_pool_malloc(SimplePool* self, size_t size);

// 将申请的内存释放
void simple_pool_free(SimplePool* self, void* ptr);

// 清空large内存并重置相应的指针
void simple_pool_reset(SimplePool* self);

// 销毁内存池对象
void simple_pool_destory(SimplePool* self);

// 显示内存池的状态，用于调试
void simple_pool_status(SimplePool* self);

#endif
