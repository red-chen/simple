#include "collection/blocking_queue.h"

#include "assert.h"

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct BlockingQueue {
    void** queue;
    int max_size;
    int size;
    int head;
    int tail;
    pthread_mutex_t size_mutex;
    pthread_mutex_t push_mutex;
    pthread_mutex_t pop_mutex;
    pthread_cond_t cond_avali; // signal when queue is not full, can push
    pthread_cond_t cond_have;  // signal when queue is not empty, can pop
};

BlockingQueue* blockingqueue_new(int size) {
    ASSERT(size > 0, "Input size(%d) <= 0", size);
    BlockingQueue* self = malloc(sizeof(BlockingQueue));
    ASSERT_NOT_NULL(self);
    bzero(self, sizeof(BlockingQueue));

    self->queue = malloc(size * sizeof(void*));
    ASSERT_NOT_NULL(self->queue);
    bzero(self->queue, size * sizeof(void*));

    self->max_size = size;
    self->size = 0;
    self->head = self->tail = self->size;

    // pthread init
    pthread_mutex_init(&self->size_mutex, NULL);
    pthread_mutex_init(&self->push_mutex, NULL);
    pthread_mutex_init(&self->pop_mutex, NULL);
    pthread_cond_init(&self->cond_avali, NULL);
    pthread_cond_init(&self->cond_have, NULL);

    return self;
}

void blockingqueue_free(BlockingQueue* self) {
    ASSERT_NOT_NULL(self);

    pthread_cond_destroy(&self->cond_avali);
    pthread_cond_destroy(&self->cond_have);
    pthread_mutex_destroy(&self->push_mutex);
    pthread_mutex_destroy(&self->pop_mutex);
    pthread_mutex_destroy(&self->size_mutex);

    free(self->queue);
    free(self);
}

void blockingqueue_push(BlockingQueue* self, void* in) {
    ASSERT_NOT_NULL(self);
    ASSERT_NOT_NULL(in);

    pthread_mutex_lock(&self->push_mutex);

    while(self->size == self->max_size) {
        pthread_cond_wait(&self->cond_avali, &self->push_mutex); // wait for avali
    }

    self->queue[self->tail++] = in;

    if (self->tail == self->max_size) {
        self->tail = 0;
    }

    pthread_mutex_lock(&self->size_mutex);
    self->size++;
    pthread_mutex_unlock(&self->size_mutex);

    if (self->size >= 0) {
        pthread_cond_signal(&self->cond_have);
    }

    pthread_mutex_unlock(&self->push_mutex);
}

void* blockingqueue_pop(BlockingQueue* self) {
    ASSERT_NOT_NULL(self);

    void* out = NULL;

    pthread_mutex_lock(&self->pop_mutex);

    while(self->size == 0) {
        pthread_cond_wait(&self->cond_have, &self->pop_mutex); // wait for empty
    }

    out = self->queue[self->head++];

    if (self->head == self->max_size) {
        self->head = 0;
    }

    pthread_mutex_lock(&self->size_mutex);
    self->size--;
    pthread_mutex_unlock(&self->size_mutex);

    if (self->size <= self->max_size) {
        pthread_cond_signal(&self->cond_avali);// not full
    }

    pthread_mutex_unlock(&self->pop_mutex);

    return out;
}

int blockingqueue_size(BlockingQueue* self) {
    ASSERT_NOT_NULL(self);
    return self->size;
}

int blockingqueue_capacity(BlockingQueue* self) {
    ASSERT_NOT_NULL(self);
    return self->max_size;
}

