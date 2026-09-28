#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "domino.h"
#include "layout.h"

#define CPU_DELAY_MS 800.0
#define MESSAGE_MS 1500.0
#define SLIDE_MS 350.0
#define DEAL_STEP_MS 50.0
#define DEAL_FLIGHT_MS 300.0
#define DEAL_MS (DEAL_STEP_MS * (TILE_COUNT - 1) + DEAL_FLIGHT_MS)
#define REVEAL_MS 400.0
#define PULSE_MS 800.0

typedef enum { ANIM_NONE, ANIM_DEAL, ANIM_SLIDE, ANIM_REVEAL } AnimKind;

typedef struct {
    Game game;
    double turn_ms;
    char message[48];
    double message_ms;
    bool choosing;      /* Sul clicou numa peça que serve nas duas pontas */
    int choosing_index;
    bool animate;       /* desligado no ctl_init; a app liga */
    double clock_ms;
    AnimKind anim;
    double anim_ms;
    bool reveal_after;  /* virar as mãos quando o deslize terminar */
    RectF slide_from;
    int slide_index;
} Controller;

/* Peça em voo, no retângulo do momento. */
typedef struct {
    int id;             /* distribuição: ordem k (0..27); deslize: índice na linha */
    RectF rect;
    uint8_t first, second;
    bool vertical;
    bool face_up;
} Flying;

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
    Flying flying[TILE_COUNT];
    int flying_count;
    int hand_shown[SEAT_COUNT];
    int board_hidden;   /* índice da linha que não deve ser desenhado, ou -1 */
    float flip_scale;   /* escala horizontal das peças das CPUs na virada */
    bool overlay_visible;
    float pulse;        /* opacidade dos destaques */
} View;

void ctl_init(Controller *c, uint64_t seed);
void ctl_click(Controller *c, float x, float y);
void ctl_enter(Controller *c);
void ctl_update(Controller *c, double dt_ms);
void ctl_view(const Controller *c, View *v);
void ctl_set_animations(Controller *c, bool on);

#endif
