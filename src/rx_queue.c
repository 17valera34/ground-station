#include "rx_queue.h"

/* Initialize synchronization primitives and indices */
void rx_queue_init(rx_queue_t* rq)
{
    rq->head = 0;
    rq->tail = 0;
    rq->stop = 0;
    pthread_mutex_init(&rq->m, NULL);
    pthread_cond_init(&rq->cond, NULL);
}

/* Free resources */
void rx_queue_destroy(rx_queue_t* rq)
{
    pthread_mutex_destroy(&rq->m);
    pthread_cond_destroy(&rq->cond);
}

/* Wake up all waiting threads and set stop flag */
void rx_queue_stop(rx_queue_t* rq)
{
    pthread_mutex_lock(&rq->m);
    rq->stop = 1;
    pthread_cond_broadcast(&rq->cond);
    pthread_mutex_unlock(&rq->m);
}

/* Add byte to queue (Non-blocking) */
int rx_queue_push(rx_queue_t* rq, uint8_t byte)
{
    pthread_mutex_lock(&rq->m);

    int next = (rq->head + 1) % RX_QUEUE_SIZE;

    /* Check if full */
    if (next == rq->tail)
    {
        pthread_mutex_unlock(&rq->m);
        return 0;
    }

    rq->buf[rq->head] = byte;
    rq->head          = next;

    pthread_cond_signal(&rq->cond);
    pthread_mutex_unlock(&rq->m);
    
    return 1;
}

/* Get byte from queue (Blocking) */
int rx_queue_pop(rx_queue_t* rq, uint8_t* out)
{
    pthread_mutex_lock(&rq->m);

    /* Wait for data or stop signal */
    while (rq->head == rq->tail && !rq->stop)
    {
        pthread_cond_wait(&rq->cond, &rq->m);
    }

    if (rq->stop)
    {
        pthread_mutex_unlock(&rq->m);
        return 0;
    }

    *out     = rq->buf[rq->tail];
    rq->tail = (rq->tail + 1) % RX_QUEUE_SIZE;

    pthread_mutex_unlock(&rq->m);
    return 1;
}