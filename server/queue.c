#include "queue.h"

void initQueue(void *Q[]) {
    Q[0] = 0;
    Q[MAXQUEUE + 1] = 1;
}

void enqueue(void *Q[], void *item) {
    if(!isFull(Q)) {
        Q[(int) Q[MAXQUEUE + 1]] = item;
        if (Q[0] == 0) {
            Q[0] = 1;
        }
        Q[MAXQUEUE + 1] = ((int) Q[MAXQUEUE + 1] % MAXQUEUE) + 1;
    }
}

void *dequeue(void *Q[]) {
    if(!isEmpty(Q)) {
        void *item = Q[(int)Q[0]];
        Q[0] = ((int)Q[0] % MAXQUEUE) + 1;
        if (Q[0] == Q[MAXQUEUE + 1]) {
            Q[0] = 0;
            Q[MAXQUEUE + 1] = 1;
        }
        return item;
    }
}
int isEmpty(void *Q[]) {
    return Q[0] == 0;
}

int isFull(void *Q[]) {
    return Q[0] == Q[MAXQUEUE];
}