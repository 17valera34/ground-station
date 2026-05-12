#pragma once

#include <stdint.h>
#include "sim_sat_state.h"

/* Satellite telemetry packet */
typedef struct
{
    mission_state_t state;  /* Current mission mode */
    float v_bat;            /* Battery voltage */
    float v_solar;          /* Solar panel voltage */
    float i_draw;           /* Current consumption */
    float temp_side;        /* Exterior temperature */
    float gyro_z;           /* Angular velocity (Z-axis) */
    float roll;             /* Attitude: Roll */
    float pitch;            /* Attitude: Pitch */
    float yaw;              /* Attitude: Yaw */
    uint32_t id;            /* Packet identifier */
    uint32_t uptime;        /* System uptime in seconds */
    uint8_t shadow;         /* Eclipse/Shadow flag (0 or 1) */
    uint32_t ack;           /* Command acknowledgement ID */
} sat_data_telemetry_t;