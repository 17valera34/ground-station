#include "ui_queue.h"

/* Initialize sync primitives and buffer indices */
void ui_queue_init(ui_queue_t* uq)
{
    uq->head = 0;
    uq->tail = 0;
    uq->stop = 0;
    pthread_mutex_init(&uq->m, NULL);
    pthread_cond_init(&uq->cond, NULL);
}

/* Free resources */
void ui_queue_destroy(ui_queue_t* uq)
{
    pthread_mutex_destroy(&uq->m);
    pthread_cond_destroy(&uq->cond);
}

/* Stop queue and wake up waiting threads */
void ui_queue_stop(ui_queue_t* uq)
{
    pthread_mutex_lock(&uq->m);
    uq->stop = 1;
    pthread_cond_broadcast(&uq->cond);
    pthread_mutex_unlock(&uq->m);
}

/* Push telemetry data into queue (Non-blocking) */
int ui_queue_push(ui_queue_t* uq, sat_data_telemetry_t data)
{
    pthread_mutex_lock(&uq->m);

    int next = (uq->head + 1) % UI_QUEUE_SIZE;

    /* Check if full */
    if (next == uq->tail)
    {
        pthread_mutex_unlock(&uq->m);
        return 0;
    }

    uq->buf[uq->head] = data;
    uq->head          = next;

    pthread_cond_signal(&uq->cond);
    pthread_mutex_unlock(&uq->m);

    return 1;
}

/* Get telemetry data from queue (Non-blocking) */
int ui_queue_pop(ui_queue_t* uq, sat_data_telemetry_t* data)
{
    pthread_mutex_lock(&uq->m);

    /* Return if empty */
    while (uq->head == uq->tail && !uq->stop)
    {
        pthread_mutex_unlock(&uq->m);
        return 0;
    }

    if (uq->stop)
    {
        pthread_mutex_unlock(&uq->m);
        return 0;
    }

    *data    = uq->buf[uq->tail];
    uq->tail = (uq->tail + 1) % UI_QUEUE_SIZE;

    pthread_mutex_unlock(&uq->m);
    return 1;
}