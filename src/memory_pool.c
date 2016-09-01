#include "memory_pool.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SIMPLE_POOL_ALINMENT 16

size_t simple_pool_pagesize = 0;

#define SIMPLE_MAX_ALLOC_FROM_POOL \
    (simple_pool_pagesize == 0 ? (simple_pool_pagesize = getpagesize() - 1) : (simple_pool_pagesize))

typedef struct SimplePoolLargeSegment SimplePoolLargeSegment;

struct SimplePoolLargeSegment {
    SimplePoolLargeSegment* next;
    void* alloc;
};

typedef struct {
    void* last;
    void* end;
    SimplePool* next;
    unsigned int failed;
} SimplePoolHeader;

struct SimplePool {
    SimplePoolHeader header;
    size_t max;
    SimplePool* current;
    SimplePoolLargeSegment* large;
};

static void* simple_pool_memalign(int alignment, int size) {
    void* p = NULL;
    posix_memalign(&p, alignment, size);
    return p;
}

SimplePool* simple_pool_create(size_t size) {
    SimplePool* self = simple_pool_memalign(SIMPLE_POOL_ALINMENT, size);
    if (self == NULL) {
        return NULL;
    }

    self->header.last = (unsigned char *) self + sizeof(SimplePool);
    self->header.end = (unsigned char *) self + size;
    self->header.failed = 0;

    size = size - sizeof(SimplePool);
    self->max = (size < SIMPLE_MAX_ALLOC_FROM_POOL) ? size : SIMPLE_MAX_ALLOC_FROM_POOL;
    self->current = self;
    self->large = NULL;

    return self;
}

void simple_pool_destory(SimplePool* self) {
    SimplePoolLargeSegment* large = NULL;
    for (large = self->large; large; large = large->next) {
        if (large->alloc) {
            free(large->alloc);
        }
    }

    SimplePool* p = NULL;
    SimplePool* n = NULL;
    for (p = self, n = self->header.next; ; p = n, n = n->header.next) {
        free(p);

        if (n == NULL) {
            break;
        }
    }
}

static void* simple_pool_alloc_block(SimplePool* self, size_t size) {
    unsigned char      *m;
    size_t       psize;
    SimplePool   *p, *new;

    psize = (size_t) ((void*)self->header.end - (void*)self);

    m = simple_pool_memalign(SIMPLE_POOL_ALINMENT, psize);
    if (m == NULL) {
        return NULL;
    }

    new = (SimplePool *) m;

    new->header.end = m + psize;
    new->header.next = NULL;
    new->header.failed = 0;

    m += sizeof(SimplePoolHeader);
    m = LEMON_ALIGN_PTR(m, SIMPLE_POOL_ALINMENT);
    new->header.last = m + size;

    for (p = self->current; p->header.next; p = p->header.next) {
        if (p->header.failed++ > 4) {
            self->current = p->header.next;
        }
    }

    p->header.next = new;

    return m; 
}

static void* simple_pool_alloc_large(SimplePool *self, size_t size) {
    void              *p;
    int              n;
    SimplePoolLargeSegment  *large;

    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = self->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = simple_pool_malloc(self, sizeof(SimplePoolLargeSegment));
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = self->large;
    self->large = large;

    return p;
}

void* simple_pool_malloc(SimplePool* self, size_t size) {
    if (size < self->max) {
        SimplePool* p = self->current;
        do {
            void* mem = LEMON_ALIGN_PTR(p->header.last, SIMPLE_POOL_ALINMENT);
            if ((size_t) (p->header.end - mem) >= size) {
                p->header.last = mem + size;
                return mem;
            }
            p = p->header.next;
        } while (p);

        return simple_pool_alloc_block(self, size);
    } else {
        return simple_pool_alloc_large(self, size);
    }
}

void simple_pool_reset(SimplePool* self) {
    SimplePool        *p;
    SimplePoolLargeSegment   *l;

    for (l = self->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = self; p; p = p->header.next) {
        p->header.last = (unsigned char *) p + sizeof(SimplePool);
        p->header.failed = 0;
    }

    self->current = self;
    self->large = NULL; 
}
