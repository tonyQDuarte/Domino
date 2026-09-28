#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "domino.h"
#include "layout.h"

#define CPU_DELAY_MS 800.0
#define MESSAGE_MS 1500.0

typedef struct {
    Game game;
    double turn_ms;
    char message[48];
    double message_ms;
    bool choosing;      /* Sul clicou numa peça que serve nas duas pontas */
    int choosing_index;
} Controller;

typedef struct {
    Phase phase;
    Seat turn;
    char score_text[32];
    float score_x, score_y;
    bool message_visible;
    char message[48];
    bool face_up[SEAT_COUNT];
    int hand_count[SEAT_COUNT];
    bool highlight_end[2];
    char result_title[32];
    char result_points[48];
    char match_message[32];
    bool show_new_match_button;
} View;

void ctl_init(Controller *c, uint64_t seed);
void ctl_click(Controller *c, float x, float y);
void ctl_enter(Controller *c);
void ctl_update(Controller *c, double dt_ms);
void ctl_view(const Controller *c, View *v);

#endif
