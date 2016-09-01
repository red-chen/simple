//
#ifndef LEMON_ARRAY_LIST_H_
#define LEMON_ARRAY_LIST_H_

#include <stdbool.h>

// arraylist

typedef struct ArrayList ArrayList;

ArrayList* arraylist_new(int size);

void arraylist_free(ArrayList* self);

void arraylist_reset(ArrayList* self, int size);

void arraylist_add(ArrayList* self, void* data);

void* arraylist_get(ArrayList* self, int index);

void* arraylist_remove(ArrayList* self, int index);

int arraylist_size(ArrayList* self);

void arraylist_clear(ArrayList* self);

// iterator

typedef struct arraylistiterator_t ArrayListIterator;

ArrayListIterator* arraylist_iterator(ArrayList* self);

void arraylist_iterator_free(ArrayListIterator* self);

bool arraylist_iterator_hasnext(ArrayListIterator* self);

void* arraylist_iterator_next(ArrayListIterator* self);

void arraylist_iterator_remove(ArrayListIterator* self);

#endif /* ARRAY_LIST_H_ */
