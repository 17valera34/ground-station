#pragma once

#include <stdint.h>

/* Satellite operational modes */
typedef enum
{
    MISSION_INIT = 0,   /* System startup and initialization */
    MISSION_ECLIPS,     /* Operation in Earth's shadow (no solar power) */
    MISSION_SUN,        /* Operation in sunlight (charging/active) */
    MISSION_DRIFT       /* Passive flight / uncontrolled rotation */
} mission_state_t;