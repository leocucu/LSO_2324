#ifndef QUEUE_H
#define QUEUE_H
    #define MAXQUEUE 16
    void initQueue(void *Q[]);
    void enqueue(void *Q[], void *item);
    void *dequeue(void *Q[]);
    int isEmpty(void *Q[]);
    int isFull(void *Q[]);
#endif