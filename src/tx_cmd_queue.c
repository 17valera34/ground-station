#include "tx_cmd_queue.h"

/* Initialize synchronization and buffer indices */
void tx_cmd_queue_init(tx_cmd_queue_t* cmq)
{
    cmq->head = 0;
    cmq->tail = 0;
    cmq->stop = 0;
    pthread_mutex_init(&cmq->m, NULL);
    pthread_cond_init(&cmq->cond, NULL);
}

/* Free resources */
void tx_cmd_queue_destroy(tx_cmd_queue_t* cmq)
{
    pthread_mutex_destroy(&cmq->m);
    pthread_cond_destroy(&cmq->cond);
}

/* Stop queue and wake up blocked threads */
void tx_cmd_queue_stop(tx_cmd_queue_t* cmq)
{
    pthread_mutex_lock(&cmq->m);
    cmq->stop = 1;
    pthread_cond_broadcast(&cmq->cond);
    pthread_mutex_unlock(&cmq->m);
}

/* Push data byte into queue (Non-blocking) */
int tx_cmd_queue_push(tx_cmd_queue_t* cmq, uint8_t data)
{
    pthread_mutex_lock(&cmq->m);

    int next = (cmq->head + 1) % CMD_QUEUE_SIZE;
    /* Check for overflow */
    if (next == cmq->tail)
    {
        pthread_mutex_unlock(&cmq->m);
        return 0;
    }

    cmq->buf[cmq->head] = data;
    cmq->head           = next;
    pthread_cond_signal(&cmq->cond);
    pthread_mutex_unlock(&cmq->m);

    return 1;
}

/* Pop data byte from queue (Non-blocking version) */
int tx_cmd_queue_pop(tx_cmd_queue_t* cmq, uint8_t* out)
{
    pthread_mutex_lock(&cmq->m);

    /* Return if empty or stopped */
    if (cmq->head == cmq->tail && !cmq->stop)
    {
        pthread_mutex_unlock(&cmq->m);
        return 0;
    }

    *out      = cmq->buf[cmq->tail];
    cmq->tail = (cmq->tail + 1) % CMD_QUEUE_SIZE;

    pthread_mutex_unlock(&cmq->m);

    return 1;
}