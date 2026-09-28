#ifndef FIXTURES_H
#define FIXTURES_H

#include <string.h>

#include "domino.h"

static Tile T(int a, int b)
{
    Tile t = {(uint8_t)a, (uint8_t)b};
    return t;
}

static void set_hand(Game *g, Seat s, const Tile *tiles, int n)
{
    g->hands[s].count = n;
    for (int i = 0; i < n; i++)
        g->hands[s].tiles[i] = tiles[i];
}

/* Mesa com uma peça só, pontas `left` e `right`. Mão em andamento, sem ser a primeira. */
static void set_board_ends(Game *g, int left, int right)
{
    memset(&g->board, 0, sizeof g->board);
    g->board.line[0].left = (uint8_t)left;
    g->board.line[0].right = (uint8_t)right;
    g->board.count = 1;
    g->board.anchor = 0;
    g->first_hand = false;
    g->passes = 0;
    g->phase = PHASE_PLAYING;
}

#endif
