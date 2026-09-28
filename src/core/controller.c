#include <math.h>
#include <stdio.h>
#include <string.h>

#include "controller.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void say(Controller *c, const char *text)
{
    snprintf(c->message, sizeof c->message, "%s", text);
    c->message_ms = MESSAGE_MS;
}

static const char *pass_message(Seat s)
{
    switch (s) {
    case SEAT_SOUTH: return "Você passou";
    case SEAT_EAST: return "Leste passou";
    case SEAT_NORTH: return "Parceiro passou";
    default: return "Oeste passou";
    }
}

static void start_anim(Controller *c, AnimKind kind)
{
    c->anim = kind;
    c->anim_ms = 0;
}

void ctl_init(Controller *c, uint64_t seed)
{
    memset(c, 0, sizeof *c);
    game_new_match(&c->game, seed);
}

void ctl_set_animations(Controller *c, bool on)
{
    c->animate = on;
    if (!on)
        c->anim = ANIM_NONE;
    else if (c->game.phase == PHASE_PLAYING && c->game.board.count == 0)
        start_anim(c, ANIM_DEAL);
}

static void after_action(Controller *c)
{
    c->turn_ms = 0;
    c->choosing = false;
}

static void new_hand_started(Controller *c)
{
    after_action(c);
    if (c->animate)
        start_anim(c, ANIM_DEAL);
}

static RectF hand_rect(Seat s, int count, int index)
{
    RectF r[HAND_SIZE];
    if (s == SEAT_SOUTH)
        layout_south_hand(count, r);
    else
        layout_cpu_row(s, count, r);
    return r[index];
}

/* Joga a peça da vez e, com animações, abre o deslize até a linha. */
static bool play(Controller *c, int index, End end)
{
    Game *g = &c->game;
    Seat s = g->turn;
    RectF from = hand_rect(s, g->hands[s].count, index);
    bool was_empty = g->board.count == 0;
    if (!game_play(g, index, end))
        return false;
    if (c->animate) {
        start_anim(c, ANIM_SLIDE);
        c->slide_from = from;
        c->slide_index = (was_empty || end == END_LEFT) ? 0 : g->board.count - 1;
        c->reveal_after = g->phase != PHASE_PLAYING;
    }
    after_action(c);
    return true;
}

static void pass(Controller *c)
{
    Seat s = c->game.turn;
    game_pass(&c->game);
    say(c, pass_message(s));
    after_action(c);
    if (c->animate && c->game.phase != PHASE_PLAYING)
        start_anim(c, ANIM_REVEAL);
}

void ctl_click(Controller *c, float x, float y)
{
    Game *g = &c->game;
    if (g->phase == PHASE_HAND_OVER) {
        if (c->anim != ANIM_NONE) {
            c->anim = ANIM_NONE; /* pula a animação e mostra o resultado */
            return;
        }
        game_next_hand(g);
        new_hand_started(c);
        return;
    }
    if (g->phase == PHASE_MATCH_OVER) {
        if (rect_contains(layout_new_match_button(), x, y)) {
            game_restart_match(g);
            new_hand_started(c);
        }
        return;
    }
    if (g->turn != SEAT_SOUTH || c->anim != ANIM_NONE)
        return;

    if (c->choosing) {
        for (int e = END_LEFT; e <= END_RIGHT; e++)
            if (rect_contains(layout_end_target(&g->board, (End)e), x, y)) {
                play(c, c->choosing_index, (End)e);
                return;
            }
    }

    const Hand *h = &g->hands[SEAT_SOUTH];
    RectF r[HAND_SIZE];
    layout_south_hand(h->count, r);
    for (int i = 0; i < h->count; i++) {
        if (!rect_contains(r[i], x, y))
            continue;
        c->choosing = false;
        bool left, right;
        game_fits(g, h->tiles[i], &left, &right);
        if (!left && !right) {
            say(c, "Jogada inválida");
        } else if (left && right) {
            const Board *b = &g->board;
            if (b->line[0].left == b->line[b->count - 1].right) {
                play(c, i, END_RIGHT);
            } else {
                c->choosing = true;
                c->choosing_index = i;
            }
        } else {
            play(c, i, left ? END_LEFT : END_RIGHT);
        }
        return;
    }
}

void ctl_enter(Controller *c)
{
    if (c->game.phase != PHASE_HAND_OVER)
        return;
    if (c->anim != ANIM_NONE) {
        c->anim = ANIM_NONE;
        return;
    }
    game_next_hand(&c->game);
    new_hand_started(c);
}

static double anim_length(AnimKind k)
{
    switch (k) {
    case ANIM_DEAL: return DEAL_MS;
    case ANIM_SLIDE: return SLIDE_MS;
    case ANIM_REVEAL: return REVEAL_MS;
    default: return 0;
    }
}

void ctl_update(Controller *c, double dt_ms)
{
    Game *g = &c->game;
    c->clock_ms += dt_ms;
    if (c->message_ms > 0)
        c->message_ms -= dt_ms;

    /* enquanto anima, o jogo espera; o que sobra do dt é descartado */
    if (c->anim != ANIM_NONE) {
        c->anim_ms += dt_ms;
        if (c->anim_ms >= anim_length(c->anim)) {
            if (c->anim == ANIM_SLIDE && c->reveal_after)
                start_anim(c, ANIM_REVEAL);
            else
                c->anim = ANIM_NONE;
        }
        return;
    }
    if (g->phase != PHASE_PLAYING)
        return;

    if (g->turn == SEAT_SOUTH) {
        if (!game_has_move(g, SEAT_SOUTH))
            pass(c);
        return;
    }

    c->turn_ms += dt_ms;
    if (c->turn_ms < CPU_DELAY_MS)
        return;
    int index;
    End end;
    if (cpu_choose(g, &index, &end))
        play(c, index, end);
    else
        pass(c);
}

static const char *result_title(HandResult r)
{
    switch (r) {
    case RESULT_SIMPLES: return "Batida simples";
    case RESULT_CARROCA: return "Batida de carroça";
    case RESULT_LA_E_LO: return "Lá-e-lô";
    case RESULT_CRUZADA: return "Cruzada";
    case RESULT_TRANCADO: return "Jogo trancado";
    default: return "";
    }
}

static float ease_out(double x)
{
    if (x < 0) x = 0;
    if (x > 1) x = 1;
    double r = 1 - x;
    return (float)(1 - r * r * r);
}

static RectF lerp_rect(RectF a, RectF b, float p)
{
    return (RectF){a.x + (b.x - a.x) * p, a.y + (b.y - a.y) * p, a.w + (b.w - a.w) * p,
                   a.h + (b.h - a.h) * p};
}

static void view_deal(const Controller *c, View *v)
{
    RectF t = layout_table_area();
    for (int s = 0; s < SEAT_COUNT; s++)
        v->hand_shown[s] = 0;
    for (int k = 0; k < TILE_COUNT; k++) {
        double start = DEAL_STEP_MS * k;
        Seat s = (Seat)(k % SEAT_COUNT);
        if (c->anim_ms >= start + DEAL_FLIGHT_MS) {
            v->hand_shown[s]++;
        } else if (c->anim_ms >= start) {
            RectF to = hand_rect(s, HAND_SIZE, k / SEAT_COUNT);
            /* sai pequena do centro da mesa e cresce até o tamanho da mão */
            float w = to.w * 0.4f, h = to.h * 0.4f;
            RectF from = {t.x + t.w / 2 - w / 2, t.y + t.h / 2 - h / 2, w, h};
            Flying *f = &v->flying[v->flying_count++];
            f->id = k;
            f->rect = lerp_rect(from, to, ease_out((c->anim_ms - start) / DEAL_FLIGHT_MS));
            f->vertical = to.h > to.w;
            f->face_up = false;
        }
    }
}

static void view_slide(const Controller *c, View *v)
{
    const Board *b = &c->game.board;
    TileSlot slots[TILE_COUNT];
    layout_board(b, slots);
    TileSlot to = slots[c->slide_index];
    PlacedTile p = b->line[c->slide_index];
    Flying *f = &v->flying[v->flying_count++];
    f->id = c->slide_index;
    f->rect = lerp_rect(c->slide_from, to.rect, ease_out(c->anim_ms / SLIDE_MS));
    f->first = to.flipped ? p.right : p.left;
    f->second = to.flipped ? p.left : p.right;
    f->vertical = to.vertical;
    f->face_up = true;
    v->board_hidden = c->slide_index;
}

void ctl_view(const Controller *c, View *v)
{
    const Game *g = &c->game;
    memset(v, 0, sizeof *v);
    v->phase = g->phase;
    v->turn = g->turn;
    snprintf(v->score_text, sizeof v->score_text, "Nós %d × %d Eles", g->score[TEAM_US],
             g->score[TEAM_THEM]);
    v->score_x = 16;
    v->score_y = 12;
    v->message_visible = c->message_ms > 0;
    if (v->message_visible)
        snprintf(v->message, sizeof v->message, "%s", c->message);

    bool hidden_reveal = c->anim == ANIM_SLIDE || (c->anim == ANIM_REVEAL && c->anim_ms < REVEAL_MS / 2);
    for (int s = 0; s < SEAT_COUNT; s++) {
        v->hand_count[s] = g->hands[s].count;
        v->hand_shown[s] = g->hands[s].count;
        v->face_up[s] = s == SEAT_SOUTH || (g->phase != PHASE_PLAYING && !hidden_reveal);
    }
    if (g->phase == PHASE_PLAYING && c->choosing)
        v->highlight_end[END_LEFT] = v->highlight_end[END_RIGHT] = true;
    if (g->phase != PHASE_PLAYING) {
        snprintf(v->result_title, sizeof v->result_title, "%s", result_title(g->result));
        if (g->result_team < 0)
            snprintf(v->result_points, sizeof v->result_points, "Ninguém pontua");
        else
            snprintf(v->result_points, sizeof v->result_points, "+%d para %s", g->result_points,
                     g->result_team == TEAM_US ? "Nós" : "Eles");
    }
    if (g->phase == PHASE_MATCH_OVER) {
        snprintf(v->match_message, sizeof v->match_message, "%s",
                 g->score[TEAM_US] >= WIN_SCORE ? "Vocês venceram!" : "Eles venceram!");
        v->show_new_match_button = true;
    }

    v->board_hidden = -1;
    v->flip_scale = 1.0f;
    v->overlay_visible = g->phase != PHASE_PLAYING && c->anim == ANIM_NONE;
    v->pulse = c->animate ? (float)(0.6 + 0.25 * sin(2 * M_PI * c->clock_ms / PULSE_MS)) : 0.45f;
    if (c->anim == ANIM_DEAL)
        view_deal(c, v);
    else if (c->anim == ANIM_SLIDE)
        view_slide(c, v);
    else if (c->anim == ANIM_REVEAL)
        v->flip_scale = (float)fabs(cos(M_PI * c->anim_ms / REVEAL_MS));
}
