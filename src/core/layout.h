#ifndef LAYOUT_H
#define LAYOUT_H

#include "domino.h"

#define SCREEN_W 1280
#define SCREEN_H 720

typedef struct {
    float x, y, w, h;
} RectF;

typedef struct {
    RectF rect;
    bool vertical;           /* eixo comprido da peça na vertical */
    bool horizontal_segment; /* trecho da linha anda na horizontal */
    bool flipped;            /* desenhar line[i].right no lado esquerdo/de cima */
} TileSlot;

bool rect_contains(RectF r, float x, float y);
bool rect_overlaps(RectF a, RectF b);

RectF layout_table_area(void);
void layout_board(const Board *b, TileSlot out[TILE_COUNT]);
/* Onde a próxima peça entraria naquela ponta (alvo do clique de escolha). */
RectF layout_end_target(const Board *b, End end);
void layout_south_hand(int count, RectF out[HAND_SIZE]);
void layout_cpu_row(Seat s, int count, RectF out[HAND_SIZE]);
RectF layout_new_match_button(void);

#endif
