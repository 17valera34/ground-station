#pragma once

#include <pthread.h>
#include <stdint.h>

#include "cmd_queue.h"
#include "tx_cmd_queue.h"

/* Extra space for protocol overhead (SOF, LEN, CRC) */
#define FRAME_OUT_OVERSIZE 5

/* TX Gateway: Routes local commands to transmission queue */
typedef struct
{
    tx_cmd_queue_t* cmq; /* Destination: hardware/network TX queue */
    cmd_queue_t* ctq;    /* Source: UI/Internal command queue */
    pthread_mutex_t m;   /* State protection */
    pthread_t thread;    /* Worker thread handle */
    int running;         /* Operation flag */
} tx_cmd_getway_t;

/* Lifecycle management */
void tx_cmd_getway_init(tx_cmd_getway_t* txg);
void tx_cmd_getway_destroy(tx_cmd_getway_t* txg);

/* Thread control */
int tx_cmd_getway_start(tx_cmd_getway_t* txg, cmd_queue_t* ctq, tx_cmd_queue_t* cmq);
void tx_cmd_getway_stop(tx_cmd_getway_t* txg);