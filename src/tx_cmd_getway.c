#include "tx_cmd_getway.h"
#include <stdio.h>
#include <string.h>
#include "cmd.h"
#include "frame_build.h"

/* Initialize resources */
void tx_cmd_getway_init(tx_cmd_getway_t* txg)
{
    txg->running = 0;
    pthread_mutex_init(&txg->m, NULL);
}

/* Free resources */
void tx_cmd_getway_destroy(tx_cmd_getway_t* txg) { pthread_mutex_destroy(&txg->m); }

/* Background thread: Pack and send commands */
static void* tx_func(void* arg)
{
    tx_cmd_getway_t* txg = (tx_cmd_getway_t*)arg;
    cmd_t cmd             = {0};

    while (1)
    {
        /* Check thread status */
        pthread_mutex_lock(&txg->m);
        int run = txg->running;
        pthread_mutex_unlock(&txg->m);

        if (!run)
            break;

        /* Get command from internal queue */
        if (!cmd_queue_pop(txg->ctq, &cmd))
            break;

        /* Create protocol frame */
        uint8_t buf[sizeof(cmd_t) + FRAME_OUT_OVERSIZE];
        int n = frame_build(buf, &cmd, sizeof(cmd_t));

        /* Push frame bytes to TX queue */
        for (int i = 0; i < n; i++)
        {
            tx_cmd_queue_push(txg->cmq, buf[i]);
        }
    }
    return NULL;
}

/* Start the gateway thread */
int tx_cmd_getway_start(tx_cmd_getway_t* txg, cmd_queue_t* ctq, tx_cmd_queue_t* cmq)
{
    txg->ctq = ctq;
    txg->cmq = cmq;

    pthread_mutex_lock(&txg->m);
    txg->running = 1;
    pthread_mutex_unlock(&txg->m);

    return pthread_create(&txg->thread, NULL, tx_func, txg);
}

/* Stop the gateway and wait for thread exit */
void tx_cmd_getway_stop(tx_cmd_getway_t* txg)
{
    pthread_mutex_lock(&txg->m);
    txg->running = 0;
    pthread_mutex_unlock(&txg->m);

    /* Unblock pop() by stopping the source queue */
    cmd_queue_stop(txg->ctq);

    pthread_join(txg->thread, NULL);
}