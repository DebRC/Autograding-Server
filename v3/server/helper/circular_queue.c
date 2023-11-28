#include <stdio.h>
#include <stdlib.h>
#include "circular_queue.h"

// Initialize the circular queue
void initQueue(CircularQueue *q) {
    q->size = 0;
    q->front = NULL;
}

// Check if the queue is empty
int isEmpty(CircularQueue *q) {
    return q->size == 0;
}

// Enqueue an item
int enqueue(CircularQueue *q, int item) {
    Node * newNode = (Node *)malloc(sizeof(Node));
    newNode->val=item;
    newNode->next=q->front;
    q->front=newNode;
    q->size++;
    return 0;
}

// Dequeue an item
int dequeue(CircularQueue *q) {
    if (isEmpty(q)) {
        printf("Queue is empty, cannot dequeue.\n");
        return -1;
    }

    Node* frontNode=NULL;
    if(q->front){
        frontNode=q->front;
    }

    int item = frontNode->val;
    q->front=frontNode->next;
    free(frontNode);
    q->size--;
    return item;
}

// Count the number of items in the queue
int getSize(CircularQueue *q) {
    return q->size;
}

// Free the memory used by the queue
void freeQueue(CircularQueue *q) {
    Node* temp;
    while(q->front){
        temp = q->front;
        q->front=temp->next;
        free(temp);
    }
}
