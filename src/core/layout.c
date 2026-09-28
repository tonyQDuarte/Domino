#include <string.h>

#include "layout.h"

#define TILE_L 48.0f   /* comprimento de uma peça na linha */
#define TILE_B 24.0f   /* largura */
#define ROW_GAP 72.0f  /* distância entre a fileira do meio e a fileira da volta */
#define MARGIN 8.0f

bool rect_contains(RectF r, float x, float y)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

bool rect_overlaps(RectF a, RectF b)
{
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

RectF layout_table_area(void) { return (RectF){130, 110, 1020, 450}; }

static bool is_double(PlacedTile p) { return p.left == p.right; }

/* Um braço da linha a partir da âncora: dir = +1 (direita, volta por baixo) ou -1. */
static void lay_arm(const Board *b, TileSlot out[], int dir, float x, float cy0, float hb)
{
    RectF t = layout_table_area();
    float lo = t.x + MARGIN, hi = t.x + t.w - MARGIN;
    float cy = cy0;
    int row = 1; /* 1 = fileira do meio, 2 = fileira da volta */
    int first = b->anchor + dir, stop = dir > 0 ? b->count : -1;

    for (int i = first; i != stop; i += dir) {
        bool dbl = is_double(b->line[i]);
        float len = dbl ? TILE_B : TILE_L, wid = dbl ? TILE_L : TILE_B;
        TileSlot *s = &out[i];
        bool fits = row == 2 || (dir > 0 ? x + len <= hi : x - len >= lo);
        if (fits) {
            /* fileira 1 anda em `dir`, fileira 2 volta em `-dir` */
            float step = row == 1 ? (float)dir : (float)-dir;
            float x0 = step > 0 ? x : x - len;
            s->rect = (RectF){x0, cy - wid / 2, len, wid};
            s->vertical = dbl;
            s->horizontal_segment = true;
            s->flipped = row == 2;
            x += step * len;
            hb = wid / 2;
        } else {
            /* curva: uma peça na vertical, para baixo (dir > 0) ou para cima */
            float vlen = dbl ? TILE_B : TILE_L, vwid = dbl ? TILE_L : TILE_B;
            float x0 = dir > 0 ? x - vwid : x;
            float y0 = dir > 0 ? cy + hb : cy - hb - vlen;
            s->rect = (RectF){x0, y0, vwid, vlen};
            s->vertical = !dbl;
            s->horizontal_segment = false;
            s->flipped = false;
            x = dir > 0 ? x - vwid : x + vwid;
            cy = cy0 + dir * ROW_GAP;
            row = 2;
        }
    }
}

void layout_board(const Board *b, TileSlot out[TILE_COUNT])
{
    if (b->count == 0)
        return;
    RectF t = layout_table_area();
    float cx = t.x + t.w / 2, cy = t.y + t.h / 2;
    bool dbl = is_double(b->line[b->anchor]);
    float len = dbl ? TILE_B : TILE_L, wid = dbl ? TILE_L : TILE_B;
    TileSlot *a = &out[b->anchor];
    a->rect = (RectF){cx - len / 2, cy - wid / 2, len, wid};
    a->vertical = dbl;
    a->horizontal_segment = true;
    a->flipped = false;
    lay_arm(b, out, +1, cx + len / 2, cy, wid / 2);
    lay_arm(b, out, -1, cx - len / 2, cy, wid / 2);
}

RectF layout_end_target(const Board *b, End end)
{
    TileSlot slots[TILE_COUNT];
    if (b->count == 0 || b->count >= TILE_COUNT) {
        if (b->count == 0)
            return (RectF){0, 0, 0, 0};
        layout_board(b, slots);
        return slots[end == END_LEFT ? 0 : b->count - 1].rect;
    }
    /* coloca uma peça qualquer (não carroça) na ponta e vê onde ela cai */
    Board tmp = *b;
    PlacedTile probe = {0, 1};
    int at;
    if (end == END_LEFT) {
        memmove(&tmp.line[1], &tmp.line[0], sizeof(PlacedTile) * (size_t)tmp.count);
        tmp.line[0] = probe;
        tmp.anchor++;
        at = 0;
    } else {
        tmp.line[tmp.count] = probe;
        at = tmp.count;
    }
    tmp.count++;
    layout_board(&tmp, slots);
    return slots[at].rect;
}

void layout_south_hand(int count, RectF out[HAND_SIZE])
{
    const float w = 50, h = 100, gap = 8;
    float total = count * (w + gap) - gap;
    float x0 = SCREEN_W / 2.0f - total / 2;
    for (int i = 0; i < count; i++)
        out[i] = (RectF){x0 + i * (w + gap), 605, w, h};
}

void layout_cpu_row(Seat s, int count, RectF out[HAND_SIZE])
{
    const float l = 60, b = 30, gap = 6;
    float total = count * (b + gap) - gap;
    for (int i = 0; i < count; i++) {
        float off = i * (b + gap);
        switch (s) {
        case SEAT_NORTH:
            out[i] = (RectF){SCREEN_W / 2.0f - total / 2 + off, 40, b, l};
            break;
        case SEAT_WEST:
            out[i] = (RectF){40, 335 - total / 2 + off, l, b};
            break;
        case SEAT_EAST:
            out[i] = (RectF){SCREEN_W - 40 - l, 335 - total / 2 + off, l, b};
            break;
        default:
            out[i] = (RectF){0, 0, 0, 0};
            break;
        }
    }
}

RectF layout_new_match_button(void) { return (RectF){540, 430, 200, 50}; }

void layout_sleeping(RectF out[SLEEPING_COUNT])
{
    const float w = 30, h = 60, gap = 6;
    float x0 = SCREEN_W - 40 - (SLEEPING_COUNT * (w + gap) - gap);
    for (int i = 0; i < SLEEPING_COUNT; i++)
        out[i] = (RectF){x0 + i * (w + gap), 40, w, h};
}
