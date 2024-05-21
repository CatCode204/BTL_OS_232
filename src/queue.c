#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
    if (q == NULL)
        return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    if (q->size < 0)
        exit(0);
#ifdef PRIO_OVERWRITE
    if (q->size >= MAX_QUEUE_SIZE && proc->prio < MAX_PRIO)
    {
        proc->prio++;
        enqueue(q->next, proc);
    }
    else if (proc->prio <= MAX_PRIO)
    {
        q->proc[q->size] = proc;
        q->size++;
    }
#else
    if (q->size >= MAX_QUEUE_SIZE && proc->priority < MAX_PRIO)
    {
        proc->priority++;
        enqueue(q->next, proc);
    }
    else if (proc->priority <= MAX_PRIO)
    {
        q->proc[q->size] = proc;
        q->size++;
    }
#endif
    // q->top = (q->top + 1) % MAX_QUEUE_SIZE;
    // q->proc[q->top] = proc;
    // q->size++;
}

struct pcb_t *dequeue(struct queue_t *q)
{
    if (!q->size)
        return NULL;
    int id = 0;
    for (int i = 0; i < q->size; i++)
#ifdef PRIO_OVERWRITE
        if (q->proc[id]->prio < q->proc[i]->prio)
            id = i;
#else
        if (q->proc[id]->priority < q->proc[i]->priority)
            id = i;
#endif
    struct pcb_t *curr = q->proc[id];
    q->size--;
    for (int i = id; i < q->size; i++)
        q->proc[i] = q->proc[i + 1];
    return curr;

    // if (empty(q)) return NULL;
    // struct pcb_t * rt = q->proc[q->bot];
    // q->bot = (q->bot + 1) % MAX_QUEUE_SIZE;
    // q->size--;
    // return rt;
}
