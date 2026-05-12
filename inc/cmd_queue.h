#pragma once

#include <pthread.h>
#include <stdint.h>

#include "cmd.h"

/* Capacity for high-level command structures */
#define CMD_QUEUE_SIZE 128

/* Thread-safe queue for internal command processing */
typedef struct
{
    pthread_mutex_t m;      /* State protection */
    pthread_cond_t cond;    /* Blocking pop signal */
    int head;               /* Write index */
    int tail;               /* Read index */
    int stop;               /* Shutdown flag */
    cmd_t buf[CMD_QUEUE_SIZE];
} cmd_queue_t;

/* Lifecycle management */
void cmd_queue_init(cmd_queue_t* ctq);
void cmd_queue_destroy(cmd_queue_t* ctq);

/* Thread-safe I/O operations */
void cmd_queue_stop(cmd_queue_t* ctq);
int cmd_queue_push(cmd_queue_t* ctq, cmd_t data);
int cmd_queue_pop(cmd_queue_t* ctq, cmd_t* data);