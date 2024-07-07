#include "queuelst.h"
#include <stdlib.h>

void initq(struct queue* queue){
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

void enqueue(struct queue* queue, void* item){
    struct Node* newNode = malloc(sizeof(struct Node));
    newNode->item = item;
    newNode->next = NULL;
    (queue->size)++;

    if(queue->head != NULL){
        queue->head->next = newNode;
        queue->head = newNode;
    } else {
        queue->head = newNode;
        queue->tail = newNode;
    }
}

void* dequeue(struct queue* queue){
    void* ret;
    struct Node* temp;
    if(queue->tail == NULL){
        return NULL;
    } else if(queue->head == queue->tail){
        temp = queue->tail;
        ret = temp->item;

        queue->head = NULL;
        queue->tail = NULL;
    } else {
        temp = queue->tail;
        ret = temp->item;
        queue->tail = queue->tail->next;
    }

    (queue->size)--;

    free(temp);
    return ret;
}

void findAndRemove(struct queue* queue, void* item){
    struct Node *temp, *pred;
    temp = queue->tail;
    pred = NULL;
    

    while(temp != NULL && temp->item != item){
        temp = temp->next;
        pred = temp;
    }

    if(temp != NULL){
        queue->size--;
        if(pred == NULL){
            dequeue(queue);
        } else {
            pred->next = temp->next;
            free(temp);
        }
    }
}