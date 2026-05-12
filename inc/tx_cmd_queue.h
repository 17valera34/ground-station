#pragma once

#include <pthread.h>
#include <stdint.h>

/* Queue capacity for outgoing bytes */
#define CMD_QUEUE_SIZE 128

/* Thread-safe circular buffer for transmission */
typedef struct
{
    pthread_mutex_t m;      /* Protection mutex */
    pthread_cond_t cond;    /* Signal for new data */
    int head;               /* Write index */
    int tail;               /* Read index */
    int stop;               /* Exit flag */
    uint8_t buf[CMD_QUEUE_SIZE];
} tx_cmd_queue_t;

/* Lifecycle management */
void tx_cmd_queue_init(tx_cmd_queue_t* cmq);
void tx_cmd_queue_destroy(tx_cmd_queue_t* cmq);

/* Control and I/O */
void tx_cmd_queue_stop(tx_cmd_queue_t* cmq);
int tx_cmd_queue_push(tx_cmd_queue_t* cmq, uint8_t data);
int tx_cmd_queue_pop(tx_cmd_queue_t* cmq, uint8_t* out);