#pragma once

#include <pthread.h>
#include <stdint.h>

#include "sat_data_telemetry.h"

/* UI display queue capacity */
#define UI_QUEUE_SIZE 64

/* Thread-safe queue for telemetry structures */
typedef struct
{
    pthread_mutex_t m;      /* Protection mutex */
    pthread_cond_t cond;    /* Signal for new telemetry */
    int head;               /* Write index */
    int tail;               /* Read index */
    int stop;               /* Exit flag */
    sat_data_telemetry_t buf[UI_QUEUE_SIZE];
} ui_queue_t;

/* Lifecycle management */
void ui_queue_init(ui_queue_t* uq);
void ui_queue_destroy(ui_queue_t* uq);

/* Control and I/O */
void ui_queue_stop(ui_queue_t* uq);
int ui_queue_push(ui_queue_t* uq, sat_data_telemetry_t data);
int ui_queue_pop(ui_queue_t* uq, sat_data_telemetry_t* data);