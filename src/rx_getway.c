#include "rx_getway.h"
#include <stdio.h>
#include <string.h>
#include "frame_parser.h"
#include "sat_data_telemetry.h"

/* Initialize resources */
void rx_getway_init(rx_getway_t* get)
{
    get->running = 0;
    pthread_mutex_init(&get->m, NULL);
}

/* Free resources */
void rx_getway_destroy(rx_getway_t* get) { pthread_mutex_destroy(&get->m); }

/* Background thread: process and route incoming telemetry */
static void* get_func(void* arg)
{
    rx_getway_t* get = (rx_getway_t*)arg;
    uint8_t byte     = 0;
    frame_t frame;
    frame_parser_t p;
    frame_parser_init(&p);
    sat_data_telemetry_t tl = {0};

    while (1)
    {
        /* Check thread status */
        pthread_mutex_lock(&get->m);
        int run = get->running;
        pthread_mutex_unlock(&get->m);

        if (!run)
            break;

        /* Get data from RX queue */
        if (!rx_queue_pop(get->rq, &byte))
            break;

        /* Parse bytes into frames */
        if (frame_parser_push(&p, byte, &frame))
        {
            /* If telemetry frame is valid, send to UI queue */
            if (frame.len == sizeof(sat_data_telemetry_t))
            {
                memcpy(&tl, frame.payload, sizeof(tl));
                ui_queue_push(get->uq, tl);
            }
        }
    }
    return NULL;
}

/* Start the gateway thread */
int rx_getway_start(rx_getway_t* get, rx_queue_t* rq, ui_queue_t* uq)
{
    get->uq = uq;
    get->rq = rq;

    pthread_mutex_lock(&get->m);
    get->running = 1;
    pthread_mutex_unlock(&get->m);

    return pthread_create(&get->thread, NULL, get_func, get);
}

/* Stop the gateway and wait for thread exit */
void rx_getway_stop(rx_getway_t* get)
{
    pthread_mutex_lock(&get->m);
    get->running = 0;
    pthread_mutex_unlock(&get->m);

    /* Unblock pop() by stopping the queue */
    rx_queue_stop(get->rq);

    pthread_join(get->thread, NULL);
}