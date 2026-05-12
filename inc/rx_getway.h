#pragma once

#include <pthread.h>
#include <stdint.h>

#include "rx_queue.h"
#include "ui_queue.h"

/* RX Gateway: Links RX queue to UI queue */
typedef struct
{
    ui_queue_t *uq;      /* Pointer to UI queue */
    rx_queue_t* rq;      /* Pointer to RX queue */
    pthread_mutex_t m;   /* State protection */
    pthread_t thread;    /* Background thread handle */
    int running;         /* Operation flag */
} rx_getway_t;

/* Lifecycle management */
void rx_getway_init(rx_getway_t* get);
void rx_getway_destroy(rx_getway_t* get);

/* Thread control */
int rx_getway_start(rx_getway_t* get, rx_queue_t* rq, ui_queue_t *uq);
void rx_getway_stop(rx_getway_t* get);