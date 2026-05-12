#pragma once

#include <pthread.h>
#include <stdint.h>

#include "rx_queue.h"
#include "tx_cmd_queue.h"
#include "sim_sat_state.h"

/* Protocol overhead for frame headers/checksums */
#define FRAME_OVERSIZE 5

/* Worker: Core logic engine handling data processing and state */
typedef struct
{
    mission_state_t state;  /* Current internal mission state */
    tx_cmd_queue_t* cmq;    /* Source: Commands to be processed/sent */
    rx_queue_t* rq;         /* Destination: Outgoing telemetry/data */
    pthread_mutex_t m;      /* State and flag protection */
    pthread_t thread;       /* Background processing thread handle */
    int running;            /* Operation flag */
} worker_t;

/* Lifecycle management */
void worker_init(worker_t* wr);
void worker_destroy(worker_t* wr);

/* Thread control and I/O linking */
int worker_start(worker_t* wr, rx_queue_t* rq, tx_cmd_queue_t* cmq);
void worker_stop(worker_t* wr);