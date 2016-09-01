#include "collection/order_list.h"

#include "assert.h"

#include <string.h>

#define ORDER_LIST_DEFAULT_ALLOCATE_SIZE (64)

struct OrderList {
    SortCompareFunc* compare;
    int size;
    int allocate_size;
    void** array;
};

OrderList* orderlist_new(SortCompareFunc* compare) {
    ASSERT(compare != NULL, "Compare function can not be NULL.");
    OrderList* self = malloc(sizeof(OrderList));
    bzero(self, sizeof(OrderList));

    self->compare = compare;
    self->size = 0;
    self->allocate_size = ORDER_LIST_DEFAULT_ALLOCATE_SIZE;

    self->array = calloc(self->allocate_size, sizeof(void*));
    bzero(self->array, self->allocate_size * sizeof(void*));
    return self;
}

void orderlist_free(OrderList* self) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    free(self->array);
    free(self);
}

bool orderlist_contain(OrderList* self, void* value) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    ASSERT(value != NULL, "Input value can not be NULL.");
    int index = orderlist_binary_search(self, 0, self->size - 1, value);
    return index >= 0 && index < self->size ? true : false;
}

void* orderlist_get(OrderList* self, int index) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    ASSERT(index < self->size, "Input index out of range.");
    return self->array [index];
}

void orderlist_resize(OrderList* self, int allocate_size)
{
    ASSERT(self != NULL, "Input order list can not be NULL.");
    ASSERT(allocate_size > self->size, "Input allocate_size <= size");

    void* t = realloc(self->array, (allocate_size * sizeof(void*)));

    ASSERT(t != NULL, "realloc failed.");

    self->array = (void**) t;

    bzero((void*) (self->array + self->allocate_size), ((allocate_size - self->allocate_size) * sizeof(void*))); // 多出来的部分

    self->allocate_size = allocate_size;
}

void orderlist_add(OrderList* self, void* value) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    ASSERT(value != NULL, "Input value can not be NULL.");

    if (self->size == self->allocate_size) {
        orderlist_resize(self, self->allocate_size * 1.75);
    }


    for (int i = self->size; i > 0; i--) {
        if (self->compare(self->array[i - 1], value) < 0) {
            self->array[i] = value;
            self->size++;
            return;
        } else if (self->compare(self->array[i - 1], value) == 0) {
            self->array[i-1] = value;
            self->size++;
            return;
        } else {
            self->array[i] = self->array[i - 1]; // move
        }
    }
    self->array[0] = value;
    self->size++;
}

// 二分查找算法
// 返回值为-1，表示没有找到
// 返回值为size，表示sort list中的所有元素都小于value
// 返回值为<-1，表示找到一个第一次大于等于value值的insert point : (-pos - 1)
int orderlist_binary_search(OrderList* self, int begin_index, int end_index, void* value) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    ASSERT(value != NULL, "Input value can not be NULL.");
    ASSERT(begin_index <= end_index, "Input begin_index must be <= end_index");
    ASSERT(begin_index >= 0 && end_index >= 0, "Input begin_index and end_index must be >= 0");
    ASSERT(end_index < self->size, "Out Of Range");

    int Low, Mid = 0, High;
    Low = begin_index;
    High = end_index;
    while (Low <= High) {
        Mid = (Low + High) / 2;
        if (self->compare(self->array[Mid], value) < 0) {
            Low = Mid + 1;
        } else if (self->compare(self->array[Mid], value) > 0) {
            High = Mid - 1;
        } else {
            return Mid; /*Found*/
        }
    }
    // not found
    if (Low > end_index) {
        return end_index + 1;
    } else {
        return -Mid - 1;
    }
}

int orderlist_size(OrderList* self) {
    ASSERT(self != NULL, "Input order list can not be NULL.");
    return self->size;
}
