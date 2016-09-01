// thread-safe FIFO-Queue

#ifndef LEMON_BLOCKING_QUEUE_H_
#define LEMON_BLOCKING_QUEUE_H_

typedef struct BlockingQueue BlockingQueue;

BlockingQueue* blockingqueue_new(int size);

void blockingqueue_free(BlockingQueue* self);

void blockingqueue_push(BlockingQueue* self, void* in);

void* blockingqueue_pop(BlockingQueue* self);

int blockingqueue_size(BlockingQueue* self);

int blockingqueue_capacity(BlockingQueue* self);

#endif /* BLOCKING_QUEUE_H_ */
