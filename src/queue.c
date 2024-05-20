#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
    if (q == NULL) return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    q->top = (q->top + 1) % MAX_QUEUE_SIZE;
    q->proc[q->top] = proc;
    q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
    if (empty(q)) return NULL;
    struct pcb_t * rt = q->proc[q->bot];
    q->bot = (q->bot + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return rt;
}
