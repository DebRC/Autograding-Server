#include <stdio.h>
#include <stdlib.h>
#include "circular_queue.h"

// Initialize the circular queue
void initQueue(CircularQueue *q, int size) {
    q->size = size;
    q->array = (int *)malloc(size * sizeof(int));
    q->front = -1;
    q->rear = -1;
    q->count = 0;
}

// Check if the queue is empty
int isEmpty(CircularQueue *q) {
    return q->front == -1;
}

// Check if the queue is full
int isFull(CircularQueue *q) {
    return (q->front == 0 && q->rear == q->size - 1) || (q->front == q->rear + 1);
}

// Enqueue an item
int enqueue(CircularQueue *q, int item) {
    if (isFull(q)) {
        printf("Queue is full, cannot enqueue.\n");
        return -1;
    }

    if (q->front == -1) {
        q->front = 0;
    }

    q->rear = (q->rear + 1) % q->size;
    q->array[q->rear] = item;
    q->count++;
    return 0;
}

// Dequeue an item
int dequeue(CircularQueue *q) {
    int item;
    if (isEmpty(q)) {
        printf("Queue is empty, cannot dequeue.\n");
        return -1;
    }

    item = q->array[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % q->size;
    }

    q->count--;
    return item;
}

// Count the number of items in the queue
int countItems(CircularQueue *q) {
    return q->count;
}

// Free the memory used by the queue
void freeQueue(CircularQueue *q) {
    free(q->array);
}
