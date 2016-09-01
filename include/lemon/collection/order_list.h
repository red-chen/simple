// list内部是根据compare函数排好序的，主要是解决logging的mapping继承问题

#ifndef LEMON_ORDER_LIST_H_
#define LEMON_ORDER_LIST_H_

#include <stdbool.h>

typedef struct OrderList OrderList;

// 返回值小于0，表示v1 < v2
// 返回值等于0，表示v1 == v2
// 返回值大于0，表示v1 > v2
typedef int SortCompareFunc(void* v1, void* v2);

OrderList* orderlist_new(SortCompareFunc* compare);

void orderlist_free(OrderList* self);

bool orderlist_contain(OrderList* self, void* value);

// 二分查找算法
// 返回值为0 <= pos < size，表示 找到元素，并且返回元素在容器中的地址
// 返回值为size，表示sort list中的所有元素都小于value
// 返回值为 < 0，表示找到一个第一次大于等于value值的insert point : (-pos - 1)
int orderlist_binary_search(OrderList* self, int begin_index, int end_index, void* value);

void* orderlist_get(OrderList* self, int index);

void orderlist_resize(OrderList* self, int allocate_size);

void orderlist_add(OrderList* self, void* value);

int orderlist_size(OrderList* self);

#endif /* ORDER_LIST_H_ */
