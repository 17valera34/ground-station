#pragma once

#include <pthread.h>
#include <stdint.h>

/* Queue capacity */
#define RX_QUEUE_SIZE 64

/* Thread-safe circular buffer */
typedef struct
{
    pthread_mutex_t m;      /* Protection mutex */
    pthread_cond_t cond;    /* Signal for new data */
    int head;               /* Write index */
    int tail;               /* Read index */
    int stop;               /* Exit flag */
    uint8_t buf[RX_QUEUE_SIZE];
} rx_queue_t;

/* Lifecycle management */
void rx_queue_init(rx_queue_t* rq);
void rx_queue_destroy(rx_queue_t* rq);

/* Control and I/O */
void rx_queue_stop(rx_queue_t* rq);
int rx_queue_push(rx_queue_t* rq, uint8_t byte);
int rx_queue_pop(rx_queue_t* rq, uint8_t* out);