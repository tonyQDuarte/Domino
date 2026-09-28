/* Testes da feature abertura-carroca. */
#include <stdbool.h>

#include "controller.h"
#include "domino.h"
#include "fixtures.h"
#include "harness.h"
#include "layout.h"

static bool has_double(const Hand *h)
{
    for (int i = 0; i < h->count; i++)
        if (h->tiles[i].a == h->tiles[i].b)
            return true;
    return false;
}

/* A ordem do AC 5, calculada pelo teste. */
static Seat expected_opener(const Game *g, Seat lead)
{
    Seat order[4] = {lead, (Seat)((lead + 2) % 4), (Seat)((lead + 1) % 4), (Seat)((lead + 3) % 4)};
    for (int k = 0; k < 4; k++)
        if (has_double(&g->hands[order[k]]))
            return order[k];
    return SEAT_COUNT;
}

/* Mesa vazia, mão que não é a primeira, vez de `turn`. */
static void empty_board(Game *g, Seat turn)
{
    memset(&g->board, 0, sizeof g->board);
    g->first_hand = false;
    g->phase = PHASE_PLAYING;
    g->passes = 0;
    g->turn = turn;
}

/* ---------- S1 ---------- */

static int open_requires_double(void)
{
    Game g;
    game_new_match(&g, 1);
    for (int d = 0; d <= 6; d++) {
        empty_board(&g, SEAT_SOUTH);
        Tile h[] = {T(d, d), T((d + 1) % 7, (d + 3) % 7)};
        set_hand(&g, SEAT_SOUTH, h, 2);
        CHECK(!game_play(&g, 1, END_LEFT));
        CHECK(!game_play(&g, 1, END_RIGHT));
        CHECK(g.board.count == 0);
        CHECK(g.hands[SEAT_SOUTH].count == 2);
        CHECK(game_play(&g, 0, END_LEFT));
        CHECK(g.board.count == 1);
        CHECK(g.board.line[0].left == d && g.board.line[0].right == d);
    }
    return 0;
}

static int south_open_non_double_invalid(void)
{
    Controller c;
    ctl_init(&c, 7);
    empty_board(&c.game, SEAT_SOUTH);
    Tile h[] = {T(2, 5), T(3, 3)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    RectF r[HAND_SIZE];
    layout_south_hand(2, r);
    ctl_click(&c, r[0].x + r[0].w / 2, r[0].y + r[0].h / 2);
    CHECK(c.game.board.count == 0);
    View v;
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Jogada inválida") == 0);
    ctl_click(&c, r[1].x + r[1].w / 2, r[1].y + r[1].h / 2);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.board.line[0].left == 3 && c.game.board.line[0].right == 3);
    return 0;
}

static int cpu_opens_with_double(void)
{
    CpuView v;
    memset(&v, 0, sizeof v);
    v.seat = SEAT_EAST;
    Tile own[] = {T(6, 5), T(2, 2), T(1, 4)};
    v.own.count = 3;
    for (int i = 0; i < 3; i++)
        v.own.tiles[i] = own[i];
    for (int s = 0; s < SEAT_COUNT; s++)
        v.counts[s] = 7;
    v.counts[SEAT_EAST] = 3;
    v.empty = true;
    v.first_hand = false;
    int idx;
    End end;
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 1);

    int openings = 0;
    for (uint64_t seed = 0; seed < 200; seed++) {
        Game g;
        game_new_match(&g, seed);
        for (int hand = 0; hand < 6 && g.phase != PHASE_MATCH_OVER; hand++) {
            if (hand > 0) {
                game_next_hand(&g);
                int i;
                End e;
                CHECK(cpu_choose(&g, &i, &e));
                CHECK(game_play(&g, i, e));
                CHECK(g.board.line[0].left == g.board.line[0].right);
                openings++;
            }
            for (int step = 0; step < 80 && g.phase == PHASE_PLAYING; step++) {
                int i;
                End e;
                if (cpu_choose(&g, &i, &e))
                    game_play(&g, i, e);
                else
                    game_pass(&g);
            }
            CHECK(g.phase != PHASE_PLAYING);
        }
    }
    CHECK(openings > 200);
    return 0;
}

/* ---------- S2 ---------- */

static int opener_order_after_batida(void)
{
    Tile with[] = {T(4, 4), T(0, 1)};
    Tile without[] = {T(0, 1), T(2, 3)};
    for (int l = 0; l < SEAT_COUNT; l++) {
        Seat lead = (Seat)l;
        Seat partner = (Seat)((l + 2) % 4), next = (Seat)((l + 1) % 4), next_partner = (Seat)((l + 3) % 4);
        Game g;
        game_new_match(&g, 3);
        g.first_hand = false;

        for (int s = 0; s < SEAT_COUNT; s++)
            set_hand(&g, (Seat)s, with, 2);
        CHECK(game_choose_opener(&g, lead) == lead);

        set_hand(&g, lead, without, 2);
        CHECK(game_choose_opener(&g, lead) == partner);

        set_hand(&g, partner, without, 2);
        CHECK(game_choose_opener(&g, lead) == next);

        set_hand(&g, next, without, 2);
        CHECK(game_choose_opener(&g, lead) == next_partner);
    }
    return 0;
}

static int next_hand_uses_opener_order(void)
{
    int not_batedor = 0;
    for (uint64_t seed = 0; seed < 200; seed++) {
        Game g;
        game_new_match(&g, seed);
        set_board_ends(&g, 1, 3);
        g.turn = SEAT_EAST;
        g.opener = SEAT_SOUTH;
        Tile e[] = {T(3, 5)};
        set_hand(&g, SEAT_EAST, e, 1);
        CHECK(game_play(&g, 0, END_RIGHT));
        CHECK(g.phase == PHASE_HAND_OVER);
        game_next_hand(&g);
        CHECK(!g.first_hand);
        Seat want = expected_opener(&g, SEAT_EAST);
        CHECK(want != SEAT_COUNT);
        CHECK(g.turn == want);
        CHECK(has_double(&g.hands[g.turn]));
        if (want != SEAT_EAST)
            not_batedor++;
    }
    CHECK(not_batedor > 0);
    return 0;
}

/* Mesa 0/0 e mãos de uma peça sem 0: quatro passes trancam. */
static void tranque(Game *g, Seat opener, Tile s, Tile e, Tile n, Tile w)
{
    game_new_match(g, 5);
    set_board_ends(g, 0, 0);
    g->score[0] = g->score[1] = 0;
    g->opener = opener;
    g->turn = opener;
    set_hand(g, SEAT_SOUTH, &s, 1);
    set_hand(g, SEAT_EAST, &e, 1);
    set_hand(g, SEAT_NORTH, &n, 1);
    set_hand(g, SEAT_WEST, &w, 1);
    for (int i = 0; i < 4; i++)
        game_pass(g);
}

static int tranque_winner_leads_opening(void)
{
    Game g;
    /* Nós 2+3=5, Eles 12+10=22: Nós ganham */
    tranque(&g, SEAT_SOUTH, T(1, 1), T(6, 6), T(1, 2), T(5, 5));
    CHECK(g.result == RESULT_TRANCADO && g.result_team == TEAM_US);
    CHECK(g.next_opener == SEAT_SOUTH);
    game_next_hand(&g);
    CHECK(g.turn == expected_opener(&g, SEAT_SOUTH));

    /* quem abriu (Leste) é da outra dupla: começa no seguinte, o Norte */
    tranque(&g, SEAT_EAST, T(1, 1), T(6, 6), T(1, 2), T(5, 5));
    CHECK(g.result_team == TEAM_US);
    CHECK(g.next_opener == SEAT_NORTH);
    game_next_hand(&g);
    CHECK(g.turn == expected_opener(&g, SEAT_NORTH));

    /* Eles ganham, quem abriu (Norte) é de Nós: começa no Oeste */
    tranque(&g, SEAT_NORTH, T(6, 6), T(1, 1), T(5, 5), T(1, 2));
    CHECK(g.result_team == TEAM_THEM);
    CHECK(g.next_opener == SEAT_WEST);

    /* Eles ganham, quem abriu (Oeste) é deles: começa no Oeste */
    tranque(&g, SEAT_WEST, T(6, 6), T(1, 1), T(5, 5), T(1, 2));
    CHECK(g.result_team == TEAM_THEM);
    CHECK(g.next_opener == SEAT_WEST);
    return 0;
}

static int tranque_tie_leads_from_opener(void)
{
    const Seat openers[] = {SEAT_SOUTH, SEAT_EAST, SEAT_NORTH, SEAT_WEST};
    for (int k = 0; k < 4; k++) {
        Game g;
        /* Nós 3+4=7, Eles 5+2=7 */
        tranque(&g, openers[k], T(1, 2), T(2, 3), T(2, 2), T(1, 1));
        CHECK(g.result == RESULT_TRANCADO && g.result_team < 0);
        CHECK(g.next_opener == openers[k]);
        game_next_hand(&g);
        CHECK(g.turn == expected_opener(&g, openers[k]));
    }
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"open_requires_double", open_requires_double},
        {"south_open_non_double_invalid", south_open_non_double_invalid},
        {"cpu_opens_with_double", cpu_opens_with_double},
        {"opener_order_after_batida", opener_order_after_batida},
        {"next_hand_uses_opener_order", next_hand_uses_opener_order},
        {"tranque_winner_leads_opening", tranque_winner_leads_opening},
        {"tranque_tie_leads_from_opener", tranque_tie_leads_from_opener},
    };
    return RUN_TESTS(cases);
}
