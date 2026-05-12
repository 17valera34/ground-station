#include "ui.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cmd.h"
#include "render.h"

/* Initialize UI state */
void ui_init(ui_t* uit)
{
    uit->running = 0;
    pthread_mutex_init(&uit->m, NULL);
}

/* Free UI resources */
void ui_destroy(ui_t* uit) { pthread_mutex_destroy(&uit->m); }

/* Main UI Thread: Handles rendering and keyboard input */
static void* ui_func(void* arg)
{
    ui_t* uit               = (ui_t*)arg;
    sat_data_telemetry_t tl = {0};
    cmd_t cmd               = {0};
    uint8_t cmd_buffer[64]  = {0};
    int cmd_idx             = 0;
    int cmd_max             = 28;
    
    ncurses_init();

    while (1)
    {
        /* Thread safety check for running state */
        pthread_mutex_lock(&uit->m);
        int run = uit->running;
        pthread_mutex_unlock(&uit->m);

        if (!run)
            break;

        /* Non-blocking telemetry update */
        if (!ui_queue_pop(uit->uq, &tl))
        {
            /* Keep previous data if queue is empty */
        }

        erase();

        /* Calculate center position for the UI layout */
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int start_y = (max_y - 18) / 2;
        int start_x = (max_x - 74) / 2;

        /* Draw static borders and dynamic data */
        draw_ui_layout(start_y, start_x);
        render_telemetry_data(&tl, start_y, start_x);

        /* Handle keyboard input */
        int ch = getch();
        if (ch != ERR)
        {
            if (ch == '\n' || ch == KEY_ENTER)
            {
                /* Package and send command on Enter */
                memset(&cmd, 0, sizeof(cmd_t));
                memcpy(cmd.buf, cmd_buffer, sizeof(cmd.buf) - 1);
                cmd_queue_push(uit->ctq, cmd);

                cmd_idx = 0;
                memset(cmd_buffer, 0, sizeof(cmd_buffer));
            }
            else if (ch == KEY_BACKSPACE || ch == 127)
            {
                if (cmd_idx > 0)
                {
                    cmd_idx--;
                    cmd_buffer[cmd_idx] = '\0';
                }
            }
            else if (ch >= 32 && ch <= 126 && cmd_idx < cmd_max)
            {
                /* Append printable characters to buffer */
                cmd_buffer[cmd_idx++] = ch;
                cmd_buffer[cmd_idx]   = '\0';
            }
        }

        /* Render command input line and cursor */
        mvprintw(start_y + 17, start_x + 2, "> %-28s", cmd_buffer);
        move(start_y + 17, start_x + 4 + cmd_idx);

        refresh();
        usleep(20000); /* ~50 FPS refresh rate */
    }

    endwin(); /* Restore terminal state */
    return NULL;
}

/* Start the UI thread */
int ui_start(ui_t* uit, ui_queue_t* uq, cmd_queue_t* ctq)
{
    uit->ctq = ctq;
    uit->uq  = uq;

    pthread_mutex_lock(&uit->m);
    uit->running = 1;
    pthread_mutex_unlock(&uit->m);

    return pthread_create(&uit->thread, NULL, ui_func, uit);
}

/* Stop UI and unblock associated queues */
void ui_stop(ui_t* uit)
{
    pthread_mutex_lock(&uit->m);
    uit->running = 0;
    pthread_mutex_unlock(&uit->m);

    ui_queue_stop(uit->uq);
    pthread_join(uit->thread, NULL);
}