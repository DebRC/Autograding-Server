#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

typedef struct {
    int *array;
    int size;
    int front;
    int rear;
    int count;
} CircularQueue;

void initQueue(CircularQueue *q, int size);
int isEmpty(CircularQueue *q);
int isFull(CircularQueue *q);
int enqueue(CircularQueue *q, int item);
int dequeue(CircularQueue *q);
void freeQueue(CircularQueue *q);
int countItems(CircularQueue *q);

#endif
