#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

typedef struct Node {
    int val;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    int size;
} CircularQueue;

void initQueue(CircularQueue *q);
int isEmpty(CircularQueue *q);
int enqueue(CircularQueue *q, int item);
int dequeue(CircularQueue *q);
int getSize(CircularQueue *q);
void freeQueue(CircularQueue *q);

#endif
