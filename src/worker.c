#include "worker.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmd.h"
#include "frame_build.h"
#include "frame_parser.h"
#include "sat_data_telemetry.h"

/* Zero out telemetry structure */
static inline void sat_data_telemetry_init(sat_data_telemetry_t* t) { memset(t, 0, sizeof(*t)); }

void worker_init(worker_t* wr)
{
    wr->running = 0;
    pthread_mutex_init(&wr->m, NULL);
}

void worker_destroy(worker_t* wr) { pthread_mutex_destroy(&wr->m); }

/* Main Simulation Loop: Emulates satellite physics and OBC logic */
static void* wr_func(void* arg)
{
    worker_t* wr            = (worker_t*)arg;
    sat_data_telemetry_t tl = {0};
    sat_data_telemetry_init(&tl);
    uint8_t byte = 0;
    frame_t frame;
    frame_parser_t p;
    frame_parser_init(&p);
    cmd_t tl_cmd = {0};

    while (1)
    {
        /* Thread safety check */
        pthread_mutex_lock(&wr->m);
        int run = wr->running;
        pthread_mutex_unlock(&wr->m);

        if (!run)
            break;

        /* Process incoming commands (Simulating radio uplink) */
        while (tx_cmd_queue_pop(wr->cmq, &byte))
        {
            if (frame_parser_push(&p, byte, &frame))
            {
                if (sizeof(tl_cmd) == frame.len)
                {
                    memcpy(&tl_cmd, frame.payload, frame.len);

                    /* Handle specific command strings */
                    if (strstr(tl_cmd.buf, "STAB"))
                    {
                        tl.gyro_z = tl.roll = tl.pitch = tl.yaw = 0.0f;
                        tl.ack                                  = 100;
                    }
                    else if (strstr(tl_cmd.buf, "SUN"))
                    {
                        wr->state = MISSION_SUN;
                        tl.ack    = 200;
                    }
                    else if (strstr(tl_cmd.buf, "ECLIPSE"))
                    {
                        wr->state = MISSION_ECLIPS;
                        tl.ack    = 300;
                    }
                    else if (strstr(tl_cmd.buf, "RESET"))
                    {
                        wr->state = MISSION_INIT;
                        tl.uptime = tl.id = 0;
                        tl.ack            = 777;
                    }
                }
            }
        }

        /* Update base telemetry markers */
        tl.uptime += 1;
        tl.id += 1;
        tl.state = wr->state;

        /* Physics Simulation: Noise and Attitude dynamics */
        float noise      = ((rand() % 10) - 5) / 100.0f;
        float gyro_noise = ((rand() % 100) - 50) / 500.0f;

        tl.gyro_z += gyro_noise;
        tl.roll += (tl.gyro_z * 0.1f) + 0.01f;
        tl.pitch += (tl.gyro_z * 0.05f) + 0.02f;
        tl.yaw += (tl.gyro_z * 0.03f) - 0.01f;

        /* Mission State Machine: Emulate orbit environment */
        switch (wr->state)
        {
            case MISSION_INIT:
                tl.v_bat     = 3.7f;
                tl.v_solar   = 0.0f;
                tl.temp_side = 20.0f;
                tl.i_draw    = 0.050f;
                tl.shadow    = 1;
                tl.gyro_z    = 0.0f;
                if (tl.uptime > 10)
                    wr->state = MISSION_ECLIPS;
                break;

            case MISSION_ECLIPS:
                tl.shadow  = 1;
                tl.v_solar = 0.00f + (noise / 10);
                if (tl.v_bat > 3.20f)
                    tl.v_bat -= 0.005f; /* Battery discharge */
                tl.temp_side -= 0.05f;
                tl.i_draw = 0.120f + (noise / 50);
                if (tl.uptime % 30 == 0)
                    wr->state = MISSION_SUN;
                break;

            case MISSION_SUN:
                tl.shadow  = 0;
                tl.v_solar = 5.50f + noise;
                if (tl.v_bat < 4.20f)
                    tl.v_bat += 0.012f; /* Battery charging */
                tl.temp_side += 0.08f;
                tl.i_draw = 0.040f - (noise / 50);
                if (tl.uptime % 60 == 0)
                    wr->state = MISSION_DRIFT;
                break;

            case MISSION_DRIFT:
                /* Lost attitude control simulation */
                tl.gyro_z += 0.5f;
                tl.roll += 0.2f;
                tl.pitch += 0.1f;
                tl.temp_side += 0.01f;
                tl.i_draw = 0.090f + (noise / 20);
                if (tl.gyro_z > 10.0f || tl.uptime % 25 == 0)
                    wr->state = MISSION_ECLIPS;
                break;
        }

        /* Build and transmit telemetry frame (Simulating radio downlink) */
        uint8_t buf[sizeof(tl) + FRAME_OVERSIZE];
        int n = frame_build(buf, &tl, sizeof(tl));
        for (int i = 0; i < n; i++)
        {
            rx_queue_push(wr->rq, buf[i]);
        }

        sleep(1); /* Simulation step: 1 second */
    }
    return NULL;
}

/* Link queues and start simulation thread */
int worker_start(worker_t* wr, rx_queue_t* rq, tx_cmd_queue_t* cmq)
{
    wr->rq    = rq;
    wr->cmq   = cmq;
    wr->state = MISSION_INIT;

    pthread_mutex_lock(&wr->m);
    wr->running = 1;
    pthread_mutex_unlock(&wr->m);

    return pthread_create(&wr->thread, NULL, wr_func, wr);
}

/* Stop simulation */
void worker_stop(worker_t* wr)
{
    pthread_mutex_lock(&wr->m);
    wr->running = 0;
    pthread_mutex_unlock(&wr->m);

    pthread_join(wr->thread, NULL);
}
