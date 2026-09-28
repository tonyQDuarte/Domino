#include <stdio.h>
#include <string.h>

#include "controller.h"

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

void ctl_init(Controller *c, uint64_t seed)
{
    memset(c, 0, sizeof *c);
    game_new_match(&c->game, seed);
}

static void after_action(Controller *c)
{
    c->turn_ms = 0;
    c->choosing = false;
}

static void south_play(Controller *c, int index, End end)
{
    if (game_play(&c->game, index, end))
        after_action(c);
}

void ctl_click(Controller *c, float x, float y)
{
    Game *g = &c->game;
    if (g->phase == PHASE_HAND_OVER) {
        game_next_hand(g);
        after_action(c);
        return;
    }
    if (g->phase == PHASE_MATCH_OVER) {
        if (rect_contains(layout_new_match_button(), x, y)) {
            game_restart_match(g);
            after_action(c);
        }
        return;
    }
    if (g->turn != SEAT_SOUTH)
        return;

    if (c->choosing) {
        for (int e = END_LEFT; e <= END_RIGHT; e++)
            if (rect_contains(layout_end_target(&g->board, (End)e), x, y)) {
                south_play(c, c->choosing_index, (End)e);
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
                south_play(c, i, END_RIGHT);
            } else {
                c->choosing = true;
                c->choosing_index = i;
            }
        } else {
            south_play(c, i, left ? END_LEFT : END_RIGHT);
        }
        return;
    }
}

void ctl_enter(Controller *c)
{
    if (c->game.phase == PHASE_HAND_OVER) {
        game_next_hand(&c->game);
        after_action(c);
    }
}

void ctl_update(Controller *c, double dt_ms)
{
    Game *g = &c->game;
    if (c->message_ms > 0)
        c->message_ms -= dt_ms;
    if (g->phase != PHASE_PLAYING)
        return;

    if (g->turn == SEAT_SOUTH) {
        if (!game_has_move(g, SEAT_SOUTH)) {
            game_pass(g);
            say(c, pass_message(SEAT_SOUTH));
            after_action(c);
        }
        return;
    }

    c->turn_ms += dt_ms;
    if (c->turn_ms < CPU_DELAY_MS)
        return;
    int index;
    End end;
    Seat s = g->turn;
    if (cpu_choose(g, &index, &end)) {
        game_play(g, index, end);
    } else {
        game_pass(g);
        say(c, pass_message(s));
    }
    after_action(c);
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
    for (int s = 0; s < SEAT_COUNT; s++) {
        v->hand_count[s] = g->hands[s].count;
        v->face_up[s] = s == SEAT_SOUTH || g->phase != PHASE_PLAYING;
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
}
