#include "collection/array_list.h"

#include "assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAYLIST_DEFAULT_SIZE (16)

// arraylist

struct ArrayList{
    int size;
    int allocated_size;
    void** array;
};

ArrayList* arraylist_new(int size){
    ArrayList* self = malloc(sizeof(ArrayList));
    ASSERT_NOT_NULL(self);

    if (size > 0) {
        self->allocated_size = size;
    } else {
        self->allocated_size = ARRAYLIST_DEFAULT_SIZE;
    }

    self->size = 0;
    self->array =  calloc(sizeof(void*), self->allocated_size);
    bzero(self->array, sizeof(void*) * self->allocated_size);

    return self;
}

void arraylist_free(ArrayList* self){
    ASSERT_NOT_NULL(self);
    free(self->array);
    free(self);
}

void arraylist_reset(ArrayList* self, int size) {
    ASSERT(size > 0, "input size <= 0");
    ASSERT(size > self->size, "input size <= size of array list.");
    void* t = realloc(self->array, sizeof(void*) * size);
    ASSERT_NOT_NULL(t);
    self->array = t;
    self->allocated_size = size;
}

void arraylist_add(ArrayList* self, void* data){
    ASSERT_NOT_NULL(self);
    if ((self->allocated_size - self->size) < 1) {
        arraylist_reset(self, self->allocated_size * 2);
    }
    self->array[self->size] = data;
    self->size++;
}

void* arraylist_get(ArrayList* self, int index){
    ASSERT_NOT_NULL(self);
    ASSERT(index >= 0 && index < self->size, "Index out of range. index(%d), size(%d).", index, self->size);
    return self->array[index];
}

void* arraylist_remove(ArrayList* self, int index) {
    ASSERT_NOT_NULL(self);
    ASSERT(index >= 0 && index < self->size, "Index out of range. index(%d), size(%d).", index, self->size);
    void* target = self->array[index];
    self->array[index] = NULL;
    for (int i = index; i < self->size - 1; i++) {
        self->array[i] = self->array[i + 1];
    }
    self->size--;
    return target;
}

int arraylist_size(ArrayList* self) {
    return self->size;
}

void arraylist_clear(ArrayList* self)
{
    ASSERT_NOT_NULL(self);
    int i = 0;
    int size = self->size;
    while (i < size)
    {
        self->array[i] = NULL;
        i++;
    }
    self->size = 0;
}

// iterator

struct arraylistiterator_t{
    struct ArrayList* array;
    int size;
    int cursor;
};

ArrayListIterator* arraylist_iterator(ArrayList* self){
    ArrayListIterator* iter = malloc(sizeof(ArrayListIterator));
    ASSERT_NOT_NULL(iter);
    iter->array = self;
    iter->size = self->size;
    iter->cursor = -1;
    return iter;
}

void arraylist_iterator_free(ArrayListIterator* self){
    free(self);
}

bool arraylist_iterator_hasnext(ArrayListIterator* self){
    ASSERT(self->size == self->array->size, "The src array list has been modified.");
    if ((self->cursor + 1) < self->size) {
        return true;
    } else {
        return false;
    }
}

void* arraylist_iterator_next(ArrayListIterator* self){
    ASSERT(self->size == self->array->size, "The src array list has been modified.");
    return self->array->array[(self->cursor++)];
}

void arraylist_iterator_remove(ArrayListIterator* self){
    arraylist_remove(self->array, self->cursor);
    self->size--;
}
