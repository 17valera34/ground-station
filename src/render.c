#include "render.h"
#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/* Setup ncurses environment: colors, keyboard, and cursor */
void ncurses_init(void)
{
    initscr();
    noecho();
    start_color();
    use_default_colors();
    curs_set(2);            /* Block cursor */
    nodelay(stdscr, TRUE);  /* Non-blocking input getch() */
    keypad(stdscr, TRUE);   /* Enable arrows/function keys */

    /* Define color pairs for telemetry status */
    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, COLOR_CYAN, -1);   /* Titles / Info */
    init_pair(3, COLOR_GREEN, -1);  /* Nominal / Sunlit */
    init_pair(4, COLOR_YELLOW, -1); /* Warning */
    init_pair(5, COLOR_RED, -1);    /* Critical / Overheat */
    init_pair(6, COLOR_MAGENTA, -1);/* Eclipse / Safe Mode */
}

/* Draw static UI frames and labels */
void draw_ui_layout(int y, int x)
{
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y + 0, x + 23, " MISSION CONTROL: SAT-OS ");
    attroff(COLOR_PAIR(2) | A_BOLD);

    /* Main dashboard frame */
    mvprintw(y + 1, x, "+----------------------------------+------------------------------------+");
    for(int i = 2; i < 16; i++) {
        mvprintw(y + i, x, "|                                  |                                    |");
    }
    mvprintw(y + 16, x, "+----------------------------------+------------------------------------+");
    mvprintw(y + 17, x, "|                                                                       |");
    mvprintw(y + 18, x, "+----------------------------------+------------------------------------+");

    attron(COLOR_PAIR(2));
    mvprintw(y + 2, x + 2, "GND_STATION_V1  // ALPHA - 01");
    attroff(COLOR_PAIR(2));
}

/* Visualize telemetry data with status-based coloring */
void render_telemetry_data(sat_data_telemetry_t* tl, int y, int x)
{
    /* --- LEFT COLUMN: POWER & THERMAL --- */
    mvprintw(y + 4, x + 2, "ID: SAT-%08u", tl->id);
    attron(COLOR_PAIR(2));
    mvprintw(y + 6, x + 2, ">> SECTION: POWER & THERMAL");
    attroff(COLOR_PAIR(2));

    mvprintw(y + 8, x + 2, "MET:       %08u s", tl->uptime);

    /* Solar status indicator */
    mvprintw(y + 10, x + 2, "SOLAR:     ");
    if (tl->shadow) {
        attron(COLOR_PAIR(6)); printw("ECLIPSE"); attroff(COLOR_PAIR(6));
    } else {
        attron(COLOR_PAIR(3)); printw("SUNLIT "); attroff(COLOR_PAIR(3));
    }
    printw("     %1.3f V", tl->v_solar);

    /* Battery level bar calculation (3.2V - 4.2V scale) */
    float vol = (tl->v_bat < 3.2f) ? 3.2f : (tl->v_bat > 4.2f ? 4.2f : tl->v_bat);
    int bars = (int)((vol - 3.2f) * 10.0f + 0.5f);

    mvprintw(y + 11, x + 2, "BATTERY:   [");
    if (vol < 3.4f) attron(COLOR_PAIR(5));
    else if (vol < 3.7f) attron(COLOR_PAIR(2));
    else attron(COLOR_PAIR(3));

    for (int b = 0; b < 10; b++) printw(b < bars ? "#" : "-");
    attroff(COLOR_PAIR(3) | COLOR_PAIR(5) | COLOR_PAIR(2));
    printw("] %.3f V", tl->v_bat);

    /* Thermal status labels */
    mvprintw(y + 13, x + 2, "TEMP SIDE: %7.3f C   ", tl->temp_side);
    if (tl->temp_side <= -40.0f) { attron(COLOR_PAIR(5) | A_BOLD); printw("CRITICAL"); attroff(COLOR_PAIR(5) | A_BOLD); }
    else if (tl->temp_side <= 0.0f) { attron(COLOR_PAIR(4)); printw("WARNING"); attroff(COLOR_PAIR(4)); }
    else { printw("NOMINAL"); }

    mvprintw(y + 14, x + 2, "BUS DRAW:  %7.3f A   ", tl->i_draw);

    /* --- RIGHT COLUMN: MISSION & ADCS --- */
    int R = x + 37;
    attron(COLOR_PAIR(2)); mvprintw(y + 2, R, "MISSION: "); attroff(COLOR_PAIR(2));

    switch (tl->state) {
        case MISSION_INIT:   printw("BOOT SEQUENCE   "); break;
        case MISSION_ECLIPS: attron(COLOR_PAIR(6)); printw("ECLIPSE MODE    "); attroff(COLOR_PAIR(6)); break;
        case MISSION_SUN:    attron(COLOR_PAIR(3)); printw("SOLAR CHARGING  "); attroff(COLOR_PAIR(3)); break;
        case MISSION_DRIFT:  attron(COLOR_PAIR(2)); printw("ATTITUDE DRIFT  "); attroff(COLOR_PAIR(2)); break;
    }

    attron(COLOR_PAIR(2)); mvprintw(y + 6, R, ">> SECTION: ADCS (ADAPTIVE)"); attroff(COLOR_PAIR(2));
    mvprintw(y + 8, R, "ROLL:  %7.1f deg", tl->roll);
    mvprintw(y + 9, R, "PITCH: %7.1f deg", tl->pitch);
    mvprintw(y + 10, R, "YAW:   %7.1f deg", tl->yaw);
    mvprintw(y + 12, R, "ANG.VEL Z: %.3f deg/s", tl->gyro_z);

    /* Attitude stabilization status */
    mvprintw(y + 13, R, "STAB:      ");
    if (abs(tl->gyro_z) >= 10) {
        attron(COLOR_PAIR(5) | A_BOLD | A_BLINK); printw("!! DRIFT !!"); attroff(COLOR_PAIR(5) | A_BOLD | A_BLINK);
    } else {
        attron(COLOR_PAIR(3)); printw("NOMINAL"); attroff(COLOR_PAIR(3));
    }

    attron(COLOR_PAIR(2));
    mvprintw(y + 17, R, "ACK: %u", tl->ack);
    attroff(COLOR_PAIR(2));
}

/* Process character input for the command buffer */
void handle_input(int ch, uint8_t* cmd_buf, int* cmd_idx, int cmd_max)
{
    if (ch == '\n' || ch == KEY_ENTER) {
        *cmd_idx = 0;
        memset(cmd_buf, 0, cmd_max);
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (*cmd_idx > 0) {
            (*cmd_idx)--;
            cmd_buf[*cmd_idx] = '\0';
        }
    } else if (ch >= 32 && ch <= 126 && *cmd_idx < (cmd_max - 1)) {
        cmd_buf[(*cmd_idx)] = (uint8_t)ch;
        (*cmd_idx)++;
    }
}