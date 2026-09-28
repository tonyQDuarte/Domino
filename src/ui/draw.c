#include "draw.h"

#define IVORY CLITERAL(Color){245, 240, 225, 255}
#define TILE_BACK CLITERAL(Color){40, 40, 48, 255}
#define PIP CLITERAL(Color){30, 30, 30, 255}

static Rectangle rl(RectF r) { return (Rectangle){r.x, r.y, r.w, r.h}; }

/* Posições das pintas numa grade 3x3 (col, lin), por valor. */
static const int PIPS[7][6][2] = {
    {{0}},
    {{1, 1}},
    {{0, 0}, {2, 2}},
    {{0, 0}, {1, 1}, {2, 2}},
    {{0, 0}, {2, 0}, {0, 2}, {2, 2}},
    {{0, 0}, {2, 0}, {1, 1}, {0, 2}, {2, 2}},
    {{0, 0}, {0, 1}, {0, 2}, {2, 0}, {2, 1}, {2, 2}},
};

static void draw_half(Rectangle h, int value)
{
    float r = (h.width < h.height ? h.width : h.height) * 0.09f;
    float pad = (h.width < h.height ? h.width : h.height) * 0.22f;
    float cw = (h.width - 2 * pad) / 2, ch = (h.height - 2 * pad) / 2;
    for (int k = 0; k < value; k++) {
        int col = PIPS[value][k][0], row = PIPS[value][k][1];
        DrawCircleV((Vector2){h.x + pad + col * cw, h.y + pad + row * ch}, r, PIP);
    }
}

/* Peça virada para cima: `first` vai à esquerda (horizontal) ou em cima (vertical). */
static void draw_tile(RectF rf, bool vertical, int first, int second, bool highlight)
{
    Rectangle r = rl(rf);
    DrawRectangleRounded(r, 0.2f, 6, IVORY);
    DrawRectangleRoundedLinesEx(r, 0.2f, 6, 1.5f, highlight ? YELLOW : DARKGRAY);
    Rectangle a, b;
    if (vertical) {
        a = (Rectangle){r.x, r.y, r.width, r.height / 2};
        b = (Rectangle){r.x, r.y + r.height / 2, r.width, r.height / 2};
        DrawLineEx((Vector2){r.x + 3, r.y + r.height / 2}, (Vector2){r.x + r.width - 3, r.y + r.height / 2}, 1.5f, DARKGRAY);
    } else {
        a = (Rectangle){r.x, r.y, r.width / 2, r.height};
        b = (Rectangle){r.x + r.width / 2, r.y, r.width / 2, r.height};
        DrawLineEx((Vector2){r.x + r.width / 2, r.y + 3}, (Vector2){r.x + r.width / 2, r.y + r.height - 3}, 1.5f, DARKGRAY);
    }
    draw_half(a, first);
    draw_half(b, second);
}

static void draw_back(RectF rf)
{
    Rectangle r = rl(rf);
    DrawRectangleRounded(r, 0.2f, 6, TILE_BACK);
    DrawRectangleRoundedLinesEx(r, 0.2f, 6, 1.5f, GRAY);
}

static void text(Font f, const char *s, float x, float y, float size, Color c)
{
    DrawTextEx(f, s, (Vector2){x, y}, size, 1, c);
}

static void text_centered(Font f, const char *s, float cx, float y, float size, Color c)
{
    Vector2 m = MeasureTextEx(f, s, size, 1);
    DrawTextEx(f, s, (Vector2){cx - m.x / 2, y}, size, 1, c);
}

static void draw_board(const Controller *c, const View *v)
{
    const Board *b = &c->game.board;
    TileSlot slots[TILE_COUNT];
    layout_board(b, slots);
    for (int i = 0; i < b->count; i++) {
        if (i == v->board_hidden)
            continue;
        PlacedTile p = b->line[i];
        int first = slots[i].flipped ? p.right : p.left;
        int second = slots[i].flipped ? p.left : p.right;
        if (p.left == p.right) first = second = p.left;
        draw_tile(slots[i].rect, slots[i].vertical, first, second, false);
    }
    for (int e = END_LEFT; e <= END_RIGHT; e++)
        if (v->highlight_end[e]) {
            RectF t = layout_end_target(b, (End)e);
            DrawRectangleRec(rl(t), Fade(YELLOW, v->pulse));
            DrawRectangleLinesEx(rl(t), 2, Fade(YELLOW, v->pulse + 0.15f));
        }
}

static void draw_hands(const Controller *c, const View *v)
{
    const Game *g = &c->game;
    RectF r[HAND_SIZE];
    const Hand *south = &g->hands[SEAT_SOUTH];
    layout_south_hand(south->count, r);
    for (int i = 0; i < v->hand_shown[SEAT_SOUTH]; i++) {
        bool sel = c->choosing && c->choosing_index == i && g->phase == PHASE_PLAYING;
        RectF rr = r[i];
        if (sel) rr.y -= 12;
        draw_tile(rr, true, south->tiles[i].a, south->tiles[i].b, false);
        if (sel)
            DrawRectangleRoundedLinesEx(rl(rr), 0.2f, 6, 3, Fade(YELLOW, v->pulse + 0.15f));
    }
    const Seat cpus[] = {SEAT_NORTH, SEAT_EAST, SEAT_WEST};
    for (int k = 0; k < 3; k++) {
        Seat s = cpus[k];
        const Hand *h = &g->hands[s];
        layout_cpu_row(s, h->count, r);
        for (int i = 0; i < v->hand_shown[s]; i++) {
            RectF ri = r[i];
            ri.x += ri.w * (1 - v->flip_scale) / 2;
            ri.w *= v->flip_scale;
            if (ri.w < 1)
                continue;
            if (v->face_up[s])
                draw_tile(ri, s == SEAT_NORTH, h->tiles[i].a, h->tiles[i].b, false);
            else
                draw_back(ri);
        }
    }
}

static void draw_overlay(const View *v, Font f)
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.35f));
    Rectangle panel = {390, 230, 500, 270};
    DrawRectangleRounded(panel, 0.08f, 8, Fade(BLACK, 0.8f));
    float cx = SCREEN_W / 2.0f;
    if (v->phase == PHASE_MATCH_OVER) {
        text_centered(f, v->match_message, cx, 250, 44, GOLD);
        text_centered(f, v->result_title, cx, 305, 26, RAYWHITE);
        text_centered(f, v->score_text, cx, 345, 28, RAYWHITE);
        RectF b = layout_new_match_button();
        DrawRectangleRounded(rl(b), 0.3f, 8, (Color){40, 140, 70, 255});
        text_centered(f, "Nova partida", b.x + b.w / 2, b.y + 12, 26, RAYWHITE);
    } else {
        text_centered(f, v->result_title, cx, 260, 40, GOLD);
        text_centered(f, v->result_points, cx, 320, 28, RAYWHITE);
        text_centered(f, v->score_text, cx, 365, 28, RAYWHITE);
        text_centered(f, "Clique ou Enter para continuar", cx, 440, 22, LIGHTGRAY);
    }
}

void draw_frame(const Controller *c, Font font)
{
    View v;
    ctl_view(c, &v);
    ClearBackground(TABLE_COLOR);
    draw_board(c, &v);
    draw_hands(c, &v);
    text(font, v.score_text, v.score_x, v.score_y, 28, RAYWHITE);
    if (v.message_visible)
        text_centered(font, v.message, SCREEN_W / 2.0f, 565, 26, RAYWHITE);
    for (int i = 0; i < v.flying_count; i++) {
        const Flying *f = &v.flying[i];
        if (f->face_up)
            draw_tile(f->rect, f->vertical, f->first, f->second, false);
        else
            draw_back(f->rect);
    }
    if (v.overlay_visible)
        draw_overlay(&v, font);
}
