#include <stdio.h>
#include <unistd.h>

/* Project modules */
#include "cmd_queue.h"
#include "rx_getway.h"
#include "rx_queue.h"
#include "tx_cmd_queue.h"
#include "tx_cmd_getway.h"
#include "ui.h"
#include "ui_queue.h"
#include "worker.h"

/* Static project instances */
static rx_queue_t rq      = {0};
static worker_t wr        = {0};
static rx_getway_t get    = {0};
static ui_queue_t uq      = {0};
static ui_t uit           = {0};
static cmd_queue_t ctq    = {0};
static tx_cmd_getway_t txg   = {0};
static tx_cmd_queue_t cmq = {0};

int main(void)
{
    /* Initialize all components */
    rx_queue_init(&rq);
    ui_queue_init(&uq);
    cmd_queue_init(&ctq);
    worker_init(&wr);
    rx_getway_init(&get);
    ui_init(&uit);
    tx_cmd_getway_init(&txg);
    tx_cmd_queue_init(&cmq);

    /* Start background tasks */
    worker_start(&wr, &rq, &cmq);
    rx_getway_start(&get, &rq, &uq);
    ui_start(&uit, &uq, &ctq);
    tx_cmd_getway_start(&txg, &ctq, &cmq);

    /* Main execution delay */
    sleep(5700);

    /* Stop tasks */
    worker_stop(&wr);
    rx_getway_stop(&get);
    ui_stop(&uit);
    tx_cmd_getway_stop(&txg);

    /* Cleanup memory and resources */
    rx_queue_destroy(&rq);
    worker_destroy(&wr);
    rx_getway_destroy(&get);
    ui_queue_destroy(&uq);
    ui_destroy(&uit);
    cmd_queue_destroy(&ctq);
    tx_cmd_getway_destroy(&txg);
    tx_cmd_queue_destroy(&cmq);

    return 0;
}