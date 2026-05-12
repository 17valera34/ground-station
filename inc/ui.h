#pragma once

#include <ncurses.h>
#include <pthread.h>
#include <stdint.h>

#include "cmd_queue.h"
#include "ui_queue.h"

/* UI Manager: Handles ncurses display and user input */
typedef struct
{
    cmd_queue_t* ctq;    /* Source: Commands for the satellite */
    ui_queue_t* uq;      /* Destination: Received telemetry for display */
    pthread_mutex_t m;   /* State protection */
    pthread_t thread;    /* UI thread handle */
    int running;         /* Operation flag */
} ui_t;

/* Lifecycle management */
void ui_init(ui_t* uit);
void ui_destroy(ui_t* uit);

/* Thread control */
int ui_start(ui_t* uit, ui_queue_t* uq, cmd_queue_t* ctq);
void ui_stop(ui_t* uit);