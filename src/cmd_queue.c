#include "cmd_queue.h"

/* Initialize resources for the command structure queue */
void cmd_queue_init(cmd_queue_t* ctq)
{
    ctq->head = 0;
    ctq->tail = 0;
    ctq->stop = 0;
    pthread_mutex_init(&ctq->m, NULL);
    pthread_cond_init(&ctq->cond, NULL);
}

/* Cleanup synchronization primitives */
void cmd_queue_destroy(cmd_queue_t* ctq)
{
    pthread_mutex_destroy(&ctq->m);
    pthread_cond_destroy(&ctq->cond);
}

/* Stop the queue and unblock the waiting consumer thread */
void cmd_queue_stop(cmd_queue_t* ctq)
{
    pthread_mutex_lock(&ctq->m);
    ctq->stop = 1;
    pthread_cond_broadcast(&ctq->cond);
    pthread_mutex_unlock(&ctq->m);
}

/* Push command structure into the queue (Non-blocking) */
int cmd_queue_push(cmd_queue_t* ctq, cmd_t data)
{
    pthread_mutex_lock(&ctq->m);

    int next = (ctq->head + 1) % CMD_QUEUE_SIZE;

    /* Drop command if the queue is full */
    if (next == ctq->tail)
    {
        pthread_mutex_unlock(&ctq->m);
        return 0;
    }

    ctq->buf[ctq->head] = data;
    ctq->head           = next;

    /* Signal the consumer thread that a command is ready */
    pthread_cond_signal(&ctq->cond);
    pthread_mutex_unlock(&ctq->m);

    return 1;
}

/* Pop command from the queue (Blocking version) */
int cmd_queue_pop(cmd_queue_t* ctq, cmd_t* data)
{
    pthread_mutex_lock(&ctq->m);

    /* Wait for data or stop signal */
    while (ctq->head == ctq->tail && !ctq->stop)
    {
        pthread_cond_wait(&ctq->cond, &ctq->m);
    }

    /* Check if we were woken up by a stop request */
    if (ctq->stop)
    {
        pthread_mutex_unlock(&ctq->m);
        return 0;
    }

    *data     = ctq->buf[ctq->tail];
    ctq->tail = (ctq->tail + 1) % CMD_QUEUE_SIZE;

    pthread_mutex_unlock(&ctq->m);
    return 1;
}