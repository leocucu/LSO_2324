#ifndef QUEUELST_H
#define QUEUELST_H

struct Node{
    void* item;
    struct Node* next;
};

struct queue{
    struct Node* head;
    struct Node* tail;

    int size;
};

void initq(struct queue* queue);
void enqueue(struct queue* queue, void* item);
void* dequeue(struct queue* queue);
void findAndRemove(struct queue* queue, void* item);

#endif