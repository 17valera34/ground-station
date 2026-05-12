#pragma once

#include "sat_data_telemetry.h"
#include <stdint.h>

/* Terminal and global UI settings */
void ncurses_init(void);

/* Keyboard input processing for command line */
void handle_input(int ch, uint8_t* cmd_buf, int* cmd_idx, int cmd_max);

/* Static UI elements (frames, headers, labels) */
void draw_ui_layout(int y, int x);

/* Dynamic data visualization */
void render_telemetry_data(sat_data_telemetry_t* tl, int y, int x);