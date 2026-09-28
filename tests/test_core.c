#include <stdbool.h>

#include "cli.h"
#include "controller.h"
#include "domino.h"
#include "fixtures.h"
#include "harness.h"
#include "layout.h"

/* ---------- helpers ---------- */

static Seat holder_of_double_six(const Game *g)
{
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int i = 0; i < g->hands[s].count; i++)
            if (g->hands[s].tiles[i].a == 6 && g->hands[s].tiles[i].b == 6)
                return (Seat)s;
    return SEAT_COUNT;
}

static int find_tile(const Hand *h, int a, int b)
{
    for (int i = 0; i < h->count; i++) {
        Tile t = h->tiles[i];
        if ((t.a == a && t.b == b) || (t.a == b && t.b == a))
            return i;
    }
    return -1;
}

static void click_south_tile(Controller *c, int index)
{
    RectF r[HAND_SIZE];
    layout_south_hand(c->game.hands[SEAT_SOUTH].count, r);
    ctl_click(c, r[index].x + r[index].w / 2, r[index].y + r[index].h / 2);
}

static void click_rect(Controller *c, RectF r)
{
    ctl_click(c, r.x + r.w / 2, r.y + r.h / 2);
}

/* Controlador com a mesa pronta: pontas left/right, vez de `turn`. */
static void ctl_setup(Controller *c, int left, int right, Seat turn)
{
    ctl_init(c, 7);
    set_board_ends(&c->game, left, right);
    c->game.turn = turn;
    c->game.opener = SEAT_SOUTH;
    c->turn_ms = 0;
    c->message_ms = 0;
    c->choosing = false;
}

/* ---------- S1 ---------- */

static int deal_28_unique_7_each(void)
{
    for (uint64_t seed = 0; seed < 1000; seed++) {
        Game g;
        game_new_match(&g, seed);
        int seen[7][7] = {{0}};
        int total = 0;
        for (int s = 0; s < SEAT_COUNT; s++) {
            CHECK(g.hands[s].count == 7);
            for (int i = 0; i < g.hands[s].count; i++) {
                Tile t = g.hands[s].tiles[i];
                CHECK(t.a <= 6 && t.b <= 6);
                int lo = t.a < t.b ? t.a : t.b, hi = t.a < t.b ? t.b : t.a;
                seen[lo][hi]++;
                total++;
            }
        }
        CHECK(total == 28);
        for (int a = 0; a <= 6; a++)
            for (int b = a; b <= 6; b++)
                CHECK(seen[a][b] == 1);
    }
    return 0;
}

static int deal_same_seed_same_hands(void)
{
    for (uint64_t seed = 0; seed < 50; seed++) {
        Game g1, g2;
        game_new_match(&g1, seed);
        game_new_match(&g2, seed);
        for (int s = 0; s < SEAT_COUNT; s++) {
            CHECK(g1.hands[s].count == g2.hands[s].count);
            for (int i = 0; i < g1.hands[s].count; i++) {
                CHECK(g1.hands[s].tiles[i].a == g2.hands[s].tiles[i].a);
                CHECK(g1.hands[s].tiles[i].b == g2.hands[s].tiles[i].b);
            }
        }
    }
    /* seeds diferentes não dão sempre a mesma mão */
    Game a, b;
    game_new_match(&a, 1);
    game_new_match(&b, 2);
    bool differ = false;
    for (int i = 0; i < 7; i++)
        if (a.hands[0].tiles[i].a != b.hands[0].tiles[i].a ||
            a.hands[0].tiles[i].b != b.hands[0].tiles[i].b)
            differ = true;
    CHECK(differ);
    return 0;
}

static int cli_parse_seed(void)
{
    uint64_t seed = 0;
    bool has = false;
    char *a1[] = {"domino", "--seed", "42"};
    CHECK(cli_parse(3, a1, &seed, &has) == 0);
    CHECK(has);
    CHECK(seed == 42);

    char *a2[] = {"domino"};
    has = true;
    CHECK(cli_parse(1, a2, &seed, &has) == 0);
    CHECK(!has);

    char *a3[] = {"domino", "--seed", "x"};
    CHECK(cli_parse(3, a3, &seed, &has) == 2);
    return 0;
}

static int south_hand_sorted_by_sum(void)
{
    for (uint64_t seed = 0; seed < 100; seed++) {
        Controller c;
        ctl_init(&c, seed);
        const Hand *h = &c.game.hands[SEAT_SOUTH];
        for (int i = 1; i < h->count; i++)
            CHECK(tile_sum(h->tiles[i - 1]) <= tile_sum(h->tiles[i]));
    }
    return 0;
}

static int south_hand_in_bottom_band(void)
{
    for (int n = 1; n <= HAND_SIZE; n++) {
        RectF r[HAND_SIZE];
        layout_south_hand(n, r);
        for (int i = 0; i < n; i++) {
            CHECK(r[i].y >= 560);
            CHECK(r[i].y + r[i].h <= SCREEN_H);
            CHECK(r[i].x >= 0 && r[i].x + r[i].w <= SCREEN_W);
        }
    }
    return 0;
}

static int check_cpu_rows(const Controller *c)
{
    View v;
    ctl_view(c, &v);
    RectF table = layout_table_area();
    const Seat cpus[] = {SEAT_NORTH, SEAT_EAST, SEAT_WEST};
    for (int k = 0; k < 3; k++) {
        Seat s = cpus[k];
        int n = c->game.hands[s].count;
        CHECK(v.hand_count[s] == n);
        CHECK(!v.face_up[s]);
        RectF r[HAND_SIZE];
        layout_cpu_row(s, n, r);
        for (int i = 0; i < n; i++) {
            CHECK(r[i].x >= 0 && r[i].x + r[i].w <= SCREEN_W);
            CHECK(r[i].y >= 0 && r[i].y + r[i].h <= SCREEN_H);
            CHECK(!rect_overlaps(r[i], table));
        }
    }
    return 0;
}

static int cpu_rows_match_hand_count(void)
{
    Controller c;
    ctl_init(&c, 3);
    CHECK(check_cpu_rows(&c) == 0);
    /* deixa as CPUs jogarem até alguma peça de CPU sair */
    int before = c.game.hands[SEAT_NORTH].count + c.game.hands[SEAT_EAST].count +
                 c.game.hands[SEAT_WEST].count;
    for (int i = 0; i < 20 && c.game.turn != SEAT_SOUTH; i++)
        ctl_update(&c, CPU_DELAY_MS);
    int after = c.game.hands[SEAT_NORTH].count + c.game.hands[SEAT_EAST].count +
                c.game.hands[SEAT_WEST].count;
    CHECK(after < before);
    CHECK(check_cpu_rows(&c) == 0);
    return 0;
}

/* ---------- S2 ---------- */

static int first_hand_double_six_starts(void)
{
    for (uint64_t seed = 0; seed < 200; seed++) {
        Game g;
        game_new_match(&g, seed);
        CHECK(g.first_hand);
        CHECK(g.turn == holder_of_double_six(&g));
    }
    return 0;
}

static int first_hand_rejects_non_double_six(void)
{
    for (uint64_t seed = 0; seed < 50; seed++) {
        Game g;
        game_new_match(&g, seed);
        Hand *h = &g.hands[g.turn];
        int six = find_tile(h, 6, 6);
        CHECK(six >= 0);
        for (int i = 0; i < h->count; i++) {
            if (i == six)
                continue;
            CHECK(!game_play(&g, i, END_LEFT));
            CHECK(!game_play(&g, i, END_RIGHT));
            CHECK(g.board.count == 0);
            CHECK(h->count == 7);
        }
        Seat opener = g.turn;
        CHECK(game_play(&g, six, END_LEFT));
        CHECK(g.board.count == 1);
        CHECK(g.turn == next_seat(opener));
    }
    return 0;
}

static int turn_order_counterclockwise(void)
{
    Game g;
    game_new_match(&g, 5);
    set_board_ends(&g, 0, 0);
    Tile none[] = {T(1, 2), T(3, 4)};
    for (int s = 0; s < SEAT_COUNT; s++)
        set_hand(&g, (Seat)s, none, 2);
    g.turn = SEAT_SOUTH;
    game_pass(&g);
    CHECK(g.turn == SEAT_EAST);
    game_pass(&g);
    CHECK(g.turn == SEAT_NORTH);
    game_pass(&g);
    CHECK(g.turn == SEAT_WEST);
    Tile w[] = {T(0, 5), T(3, 4)};
    set_hand(&g, SEAT_WEST, w, 2);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.turn == SEAT_SOUTH);
    return 0;
}

static int click_single_end_places(void)
{
    Controller c;
    ctl_setup(&c, 1, 3, SEAT_SOUTH);
    Tile h[] = {T(5, 3), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    click_south_tile(&c, 0);
    CHECK(c.game.board.count == 2);
    CHECK(c.game.board.line[1].left == 3);
    CHECK(c.game.board.line[1].right == 5);
    CHECK(c.game.board.line[0].left == 1);
    CHECK(c.game.hands[SEAT_SOUTH].count == 1);
    CHECK(c.game.turn == SEAT_EAST);

    /* ponta esquerda: [4|1] em pontas 1/5 encosta o 1 */
    ctl_setup(&c, 1, 5, SEAT_SOUTH);
    Tile h2[] = {T(4, 1), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, h2, 2);
    click_south_tile(&c, 0);
    CHECK(c.game.board.count == 2);
    CHECK(c.game.board.line[0].left == 4);
    CHECK(c.game.board.line[0].right == 1);
    return 0;
}

static int click_both_ends_waits_choice(void)
{
    Controller c;
    ctl_setup(&c, 2, 5, SEAT_SOUTH);
    Tile h[] = {T(2, 5), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    click_south_tile(&c, 0);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.hands[SEAT_SOUTH].count == 2);
    View v;
    ctl_view(&c, &v);
    CHECK(v.highlight_end[END_LEFT]);
    CHECK(v.highlight_end[END_RIGHT]);
    click_rect(&c, layout_end_target(&c.game.board, END_RIGHT));
    CHECK(c.game.board.count == 2);
    CHECK(c.game.board.line[1].left == 5);
    CHECK(c.game.board.line[1].right == 2);
    ctl_view(&c, &v);
    CHECK(!v.highlight_end[END_LEFT] && !v.highlight_end[END_RIGHT]);
    return 0;
}

static int click_equal_ends_goes_right(void)
{
    Controller c;
    ctl_setup(&c, 3, 3, SEAT_SOUTH);
    Tile h[] = {T(3, 1), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    click_south_tile(&c, 0);
    View v;
    ctl_view(&c, &v);
    CHECK(!v.highlight_end[END_LEFT] && !v.highlight_end[END_RIGHT]);
    CHECK(c.game.board.count == 2);
    CHECK(c.game.board.line[0].left == 3 && c.game.board.line[0].right == 3);
    CHECK(c.game.board.line[1].left == 3);
    CHECK(c.game.board.line[1].right == 1);
    return 0;
}

static int click_invalid_shows_message(void)
{
    Controller c;
    ctl_setup(&c, 1, 1, SEAT_SOUTH);
    Tile h[] = {T(2, 3), T(1, 6)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    click_south_tile(&c, 0);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.board.line[0].left == 1 && c.game.board.line[0].right == 1);
    CHECK(c.game.hands[SEAT_SOUTH].count == 2);
    CHECK(c.game.turn == SEAT_SOUTH);
    View v;
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Jogada inválida") == 0);
    ctl_update(&c, 1499);
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(!v.message_visible);
    return 0;
}

static int click_ignored_off_turn(void)
{
    Controller c;
    ctl_setup(&c, 1, 3, SEAT_EAST);
    Tile h[] = {T(3, 5), T(1, 6)};
    set_hand(&c.game, SEAT_SOUTH, h, 2);
    int east = c.game.hands[SEAT_EAST].count;
    click_south_tile(&c, 0);
    click_south_tile(&c, 1);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.hands[SEAT_SOUTH].count == 2);
    CHECK(c.game.hands[SEAT_EAST].count == east);
    CHECK(c.game.turn == SEAT_EAST);
    return 0;
}

static int south_auto_pass(void)
{
    Controller c;
    ctl_setup(&c, 0, 0, SEAT_SOUTH);
    Tile s[] = {T(1, 2)};
    Tile e[] = {T(0, 5), T(6, 6)};
    Tile n[] = {T(0, 4), T(6, 5)};
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    set_hand(&c.game, SEAT_EAST, e, 2);
    set_hand(&c.game, SEAT_NORTH, n, 2);
    ctl_update(&c, 1);
    CHECK(c.game.turn == SEAT_EAST);
    CHECK(c.game.hands[SEAT_SOUTH].count == 1);
    View v;
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Você passou") == 0);
    ctl_update(&c, 1499);
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Você passou") == 0);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(!v.message_visible);
    return 0;
}

/* ---------- S3 ---------- */

static int cpu_moves_at_800ms(void)
{
    Controller c;
    ctl_setup(&c, 1, 3, SEAT_EAST);
    Tile e[] = {T(3, 4), T(6, 6)};
    set_hand(&c.game, SEAT_EAST, e, 2);
    ctl_update(&c, 799);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.turn == SEAT_EAST);
    ctl_update(&c, 1);
    CHECK(c.game.board.count == 2);
    CHECK(c.game.hands[SEAT_EAST].count == 1);
    CHECK(c.game.turn == SEAT_NORTH);
    return 0;
}

static int cpu_picks_heaviest(void)
{
    Game g;
    int idx;
    End end;
    game_new_match(&g, 1);

    /* maior soma */
    set_board_ends(&g, 6, 1);
    g.turn = SEAT_EAST;
    Tile h1[] = {T(1, 2), T(6, 5), T(1, 0)};
    set_hand(&g, SEAT_EAST, h1, 3);
    CHECK(cpu_choose(&g, &idx, &end));
    CHECK(idx == 1);
    CHECK(end == END_LEFT);

    /* empate de soma: carroça */
    set_board_ends(&g, 6, 4);
    Tile h2[] = {T(6, 2), T(4, 4)};
    set_hand(&g, SEAT_EAST, h2, 2);
    CHECK(cpu_choose(&g, &idx, &end));
    CHECK(idx == 1);
    CHECK(end == END_RIGHT);

    /* empate sem carroça: primeira da mão */
    set_board_ends(&g, 3, 5);
    Tile h3[] = {T(3, 4), T(5, 2)};
    set_hand(&g, SEAT_EAST, h3, 2);
    CHECK(cpu_choose(&g, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_LEFT);

    /* encaixa nas duas: ponta esquerda */
    set_board_ends(&g, 2, 5);
    Tile h4[] = {T(2, 5), T(0, 0)};
    set_hand(&g, SEAT_EAST, h4, 2);
    CHECK(cpu_choose(&g, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_LEFT);

    /* sem jogada */
    set_board_ends(&g, 0, 0);
    Tile h5[] = {T(1, 2)};
    set_hand(&g, SEAT_EAST, h5, 1);
    CHECK(!cpu_choose(&g, &idx, &end));
    return 0;
}

static int cpu_pass_message(void)
{
    const Seat seats[] = {SEAT_EAST, SEAT_NORTH, SEAT_WEST};
    const char *msgs[] = {"Leste passou", "Parceiro passou", "Oeste passou"};
    for (int k = 0; k < 3; k++) {
        Controller c;
        ctl_setup(&c, 0, 0, seats[k]);
        Tile can[] = {T(0, 5), T(6, 6)};
        Tile cannot[] = {T(1, 2), T(3, 4)};
        for (int s = 0; s < SEAT_COUNT; s++)
            set_hand(&c.game, (Seat)s, can, 2);
        set_hand(&c.game, seats[k], cannot, 2);
        ctl_update(&c, 800);
        CHECK(c.game.turn == next_seat(seats[k]));
        CHECK(c.game.hands[seats[k]].count == 2);
        View v;
        ctl_view(&c, &v);
        CHECK(v.message_visible);
        CHECK(strcmp(v.message, msgs[k]) == 0);
        ctl_update(&c, 1499);
        ctl_view(&c, &v);
        CHECK(v.message_visible);
        CHECK(strcmp(v.message, msgs[k]) == 0);
        ctl_update(&c, 1);
        ctl_view(&c, &v);
        CHECK(!v.message_visible);
    }
    return 0;
}

static int cpu_faces_hidden_in_play(void)
{
    for (uint64_t seed = 0; seed < 20; seed++) {
        Controller c;
        ctl_init(&c, seed);
        for (int step = 0; step < 40 && c.game.phase == PHASE_PLAYING; step++) {
            View v;
            ctl_view(&c, &v);
            CHECK(v.face_up[SEAT_SOUTH]);
            CHECK(!v.face_up[SEAT_EAST]);
            CHECK(!v.face_up[SEAT_NORTH]);
            CHECK(!v.face_up[SEAT_WEST]);
            if (c.game.turn == SEAT_SOUTH) {
                int idx;
                End end;
                if (cpu_choose(&c.game, &idx, &end))
                    game_play(&c.game, idx, end);
            }
            ctl_update(&c, CPU_DELAY_MS);
        }
    }
    return 0;
}

/* ---------- S4 ---------- */

static int score_simples(void)
{
    Game g;
    game_new_match(&g, 1);
    set_board_ends(&g, 1, 3);
    g.score[0] = g.score[1] = 0;
    g.turn = SEAT_EAST;
    Tile e[] = {T(3, 5)};
    set_hand(&g, SEAT_EAST, e, 1);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.phase == PHASE_HAND_OVER);
    CHECK(g.result == RESULT_SIMPLES);
    CHECK(g.score[TEAM_THEM] == 1);
    CHECK(g.score[TEAM_US] == 0);
    return 0;
}

static int score_carroca(void)
{
    Game g;
    game_new_match(&g, 1);
    set_board_ends(&g, 1, 3);
    g.score[0] = g.score[1] = 0;
    g.turn = SEAT_NORTH;
    Tile n[] = {T(3, 3)};
    set_hand(&g, SEAT_NORTH, n, 1);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.result == RESULT_CARROCA);
    CHECK(g.score[TEAM_US] == 2);
    CHECK(g.score[TEAM_THEM] == 0);
    return 0;
}

static int score_la_e_lo(void)
{
    Game g;
    game_new_match(&g, 1);
    set_board_ends(&g, 2, 5);
    g.score[0] = g.score[1] = 0;
    g.turn = SEAT_SOUTH;
    Tile s[] = {T(2, 5)};
    set_hand(&g, SEAT_SOUTH, s, 1);
    CHECK(game_play(&g, 0, END_LEFT));
    CHECK(g.result == RESULT_LA_E_LO);
    CHECK(g.score[TEAM_US] == 3);

    game_new_match(&g, 1);
    set_board_ends(&g, 3, 3);
    g.score[0] = g.score[1] = 0;
    g.turn = SEAT_WEST;
    Tile w[] = {T(3, 1)};
    set_hand(&g, SEAT_WEST, w, 1);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.result == RESULT_LA_E_LO);
    CHECK(g.score[TEAM_THEM] == 3);
    CHECK(g.score[TEAM_US] == 0);
    return 0;
}

static int score_cruzada(void)
{
    Game g;
    game_new_match(&g, 1);
    set_board_ends(&g, 4, 4);
    g.score[0] = g.score[1] = 0;
    g.turn = SEAT_SOUTH;
    Tile s[] = {T(4, 4)};
    set_hand(&g, SEAT_SOUTH, s, 1);
    CHECK(game_play(&g, 0, END_LEFT));
    CHECK(g.result == RESULT_CRUZADA);
    CHECK(g.score[TEAM_US] == 4);
    CHECK(g.score[TEAM_THEM] == 0);
    return 0;
}

static void setup_tranque(Game *g, int us_a, int us_b, int them_a, int them_b)
{
    game_new_match(g, 1);
    set_board_ends(g, 0, 0);
    g->score[0] = g->score[1] = 0;
    g->turn = SEAT_SOUTH;
    Tile s[] = {T(us_a, us_a)};      /* soma 2*us_a */
    Tile n[] = {T(us_b, us_b)};
    Tile e[] = {T(them_a, them_a)};
    Tile w[] = {T(them_b, them_b)};
    set_hand(g, SEAT_SOUTH, s, 1);
    set_hand(g, SEAT_NORTH, n, 1);
    set_hand(g, SEAT_EAST, e, 1);
    set_hand(g, SEAT_WEST, w, 1);
}

static int score_trancado_lower_sum(void)
{
    Game g;
    /* Nós: 12 + 2 = 14; Eles: 4 + 6 = 10 */
    setup_tranque(&g, 6, 1, 2, 3);
    for (int i = 0; i < 3; i++) {
        game_pass(&g);
        CHECK(g.phase == PHASE_PLAYING);
    }
    game_pass(&g);
    CHECK(g.phase == PHASE_HAND_OVER);
    CHECK(g.result == RESULT_TRANCADO);
    CHECK(g.score[TEAM_THEM] == 1);
    CHECK(g.score[TEAM_US] == 0);

    /* e o contrário: Nós menor */
    setup_tranque(&g, 1, 2, 5, 6);
    for (int i = 0; i < 4; i++)
        game_pass(&g);
    CHECK(g.result == RESULT_TRANCADO);
    CHECK(g.score[TEAM_US] == 1);
    CHECK(g.score[TEAM_THEM] == 0);
    return 0;
}

static int score_trancado_tie(void)
{
    Game g;
    /* Nós: 2 + 6 = 8; Eles: 4 + 4 = 8 */
    setup_tranque(&g, 1, 3, 2, 2);
    for (int i = 0; i < 4; i++)
        game_pass(&g);
    CHECK(g.phase == PHASE_HAND_OVER);
    CHECK(g.result == RESULT_TRANCADO);
    CHECK(g.score[TEAM_US] == 0);
    CHECK(g.score[TEAM_THEM] == 0);
    return 0;
}

static int hand_end_overlay(void)
{
    for (int via_enter = 0; via_enter < 2; via_enter++) {
        Controller c;
        ctl_setup(&c, 1, 3, SEAT_SOUTH);
        c.game.score[0] = c.game.score[1] = 0;
        Tile s[] = {T(3, 5)};
        set_hand(&c.game, SEAT_SOUTH, s, 1);
        click_south_tile(&c, 0);
        CHECK(c.game.phase == PHASE_HAND_OVER);
        View v;
        ctl_view(&c, &v);
        for (int k = 0; k < SEAT_COUNT; k++)
            CHECK(v.face_up[k]);
        CHECK(strcmp(v.result_title, "Batida simples") == 0);
        CHECK(strcmp(v.result_points, "+1 para Nós") == 0);
        CHECK(strcmp(v.score_text, "Nós 1 × 0 Eles") == 0);
        ctl_update(&c, 60000);
        CHECK(c.game.phase == PHASE_HAND_OVER);
        if (via_enter)
            ctl_enter(&c);
        else
            ctl_click(&c, 10, 10);
        CHECK(c.game.phase == PHASE_PLAYING);
        for (int k = 0; k < SEAT_COUNT; k++)
            CHECK(c.game.hands[k].count == 7);
        CHECK(c.game.board.count == 0);
    }

    /* tranque com empate */
    Controller c;
    ctl_setup(&c, 0, 0, SEAT_SOUTH);
    c.game.score[0] = c.game.score[1] = 0;
    c.game.passes = 3;
    Tile s[] = {T(1, 2)};
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    Tile e[] = {T(1, 2)};
    set_hand(&c.game, SEAT_EAST, e, 1);
    Tile n[] = {T(1, 1)};
    set_hand(&c.game, SEAT_NORTH, n, 1);
    Tile w[] = {T(1, 1)};
    set_hand(&c.game, SEAT_WEST, w, 1);
    ctl_update(&c, 1);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    View v;
    ctl_view(&c, &v);
    CHECK(strcmp(v.result_title, "Jogo trancado") == 0);
    CHECK(strcmp(v.result_points, "Ninguém pontua") == 0);
    return 0;
}

static int next_hand_winner_starts(void)
{
    Game g;
    game_new_match(&g, 9);
    set_board_ends(&g, 1, 3);
    g.opener = SEAT_SOUTH;
    g.turn = SEAT_EAST;
    Tile e[] = {T(3, 5)};
    set_hand(&g, SEAT_EAST, e, 1);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.phase == PHASE_HAND_OVER);
    game_next_hand(&g);
    CHECK(g.phase == PHASE_PLAYING);
    CHECK(!g.first_hand);
    CHECK(g.turn == SEAT_EAST);
    Hand *h = &g.hands[SEAT_EAST];
    int idx = -1;
    for (int i = 0; i < h->count; i++)
        if (!(h->tiles[i].a == 6 && h->tiles[i].b == 6)) {
            idx = i;
            break;
        }
    CHECK(idx >= 0);
    CHECK(game_play(&g, idx, END_LEFT));
    CHECK(g.board.count == 1);
    return 0;
}

static int next_hand_after_tranque(void)
{
    Game g;
    setup_tranque(&g, 6, 1, 2, 3);
    g.opener = SEAT_WEST;
    for (int i = 0; i < 4; i++)
        game_pass(&g);
    CHECK(g.phase == PHASE_HAND_OVER);
    game_next_hand(&g);
    CHECK(g.turn == SEAT_WEST);
    CHECK(!g.first_hand);
    Hand *h = &g.hands[SEAT_WEST];
    int idx = -1;
    for (int i = 0; i < h->count; i++)
        if (!(h->tiles[i].a == 6 && h->tiles[i].b == 6)) {
            idx = i;
            break;
        }
    CHECK(idx >= 0);
    CHECK(game_play(&g, idx, END_LEFT));
    return 0;
}

static int score_text_format(void)
{
    Controller c;
    ctl_init(&c, 1);
    View v;
    ctl_view(&c, &v);
    CHECK(strcmp(v.score_text, "Nós 0 × 0 Eles") == 0);
    CHECK(v.score_x <= 20 && v.score_y <= 20);
    c.game.score[TEAM_US] += 3;
    c.game.score[TEAM_THEM] += 1;
    ctl_view(&c, &v);
    CHECK(strcmp(v.score_text, "Nós 3 × 1 Eles") == 0);
    return 0;
}

static int match_end_message(void)
{
    /* Nós chegam a 6 */
    Controller c;
    ctl_setup(&c, 1, 3, SEAT_SOUTH);
    c.game.score[TEAM_US] = 5;
    c.game.score[TEAM_THEM] = 2;
    Tile s[] = {T(3, 5)};
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    click_south_tile(&c, 0);
    CHECK(c.game.phase == PHASE_MATCH_OVER);
    View v;
    ctl_view(&c, &v);
    CHECK(strcmp(v.match_message, "Vocês venceram!") == 0);
    CHECK(v.show_new_match_button);
    CHECK(strcmp(v.score_text, "Nós 6 × 2 Eles") == 0);

    /* Eles passam de 6 com cruzada */
    ctl_setup(&c, 4, 4, SEAT_EAST);
    c.game.score[TEAM_US] = 1;
    c.game.score[TEAM_THEM] = 4;
    Tile e[] = {T(4, 4)};
    set_hand(&c.game, SEAT_EAST, e, 1);
    ctl_update(&c, CPU_DELAY_MS);
    CHECK(c.game.phase == PHASE_MATCH_OVER);
    ctl_view(&c, &v);
    CHECK(strcmp(v.match_message, "Eles venceram!") == 0);
    CHECK(v.show_new_match_button);
    CHECK(strcmp(v.score_text, "Nós 1 × 8 Eles") == 0);

    /* 5 pontos não encerram */
    ctl_setup(&c, 1, 3, SEAT_SOUTH);
    c.game.score[TEAM_US] = 4;
    c.game.score[TEAM_THEM] = 0;
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    click_south_tile(&c, 0);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    ctl_view(&c, &v);
    CHECK(!v.show_new_match_button);
    return 0;
}

static int new_match_resets(void)
{
    Controller c;
    ctl_setup(&c, 1, 3, SEAT_SOUTH);
    c.game.score[TEAM_US] = 5;
    Tile s[] = {T(3, 5)};
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    click_south_tile(&c, 0);
    CHECK(c.game.phase == PHASE_MATCH_OVER);
    ctl_enter(&c);
    CHECK(c.game.phase == PHASE_MATCH_OVER);
    click_rect(&c, layout_new_match_button());
    CHECK(c.game.phase == PHASE_PLAYING);
    CHECK(c.game.score[TEAM_US] == 0);
    CHECK(c.game.score[TEAM_THEM] == 0);
    CHECK(c.game.first_hand);
    CHECK(c.game.board.count == 0);
    CHECK(c.game.turn == holder_of_double_six(&c.game));
    View v;
    ctl_view(&c, &v);
    CHECK(strcmp(v.score_text, "Nós 0 × 0 Eles") == 0);
    return 0;
}

/* ---------- S5 ---------- */

static void make_line(Board *b, int n, int anchor, bool doubles)
{
    memset(b, 0, sizeof *b);
    for (int i = 0; i < n; i++) {
        if (doubles && i % 3 == 0) {
            b->line[i].left = b->line[i].right = (uint8_t)(i % 7);
        } else {
            b->line[i].left = (uint8_t)(i % 7);
            b->line[i].right = (uint8_t)((i + 1) % 7 == i % 7 ? 0 : (i + 1) % 7);
            if (b->line[i].left == b->line[i].right)
                b->line[i].right = (uint8_t)((b->line[i].left + 1) % 7);
        }
    }
    b->count = n;
    b->anchor = anchor;
}

static int layout_first_tile_centered(void)
{
    RectF t = layout_table_area();
    const int vals[][2] = {{6, 6}, {2, 5}};
    for (int k = 0; k < 2; k++) {
        Board b;
        memset(&b, 0, sizeof b);
        b.line[0].left = (uint8_t)vals[k][0];
        b.line[0].right = (uint8_t)vals[k][1];
        b.count = 1;
        TileSlot s[TILE_COUNT];
        layout_board(&b, s);
        float cx = s[0].rect.x + s[0].rect.w / 2, cy = s[0].rect.y + s[0].rect.h / 2;
        CHECK(cx > t.x + t.w / 2 - 0.5f && cx < t.x + t.w / 2 + 0.5f);
        CHECK(cy > t.y + t.h / 2 - 0.5f && cy < t.y + t.h / 2 + 0.5f);
    }
    return 0;
}

static int layout_within_table(void)
{
    RectF t = layout_table_area();
    for (int dbl = 0; dbl < 2; dbl++)
        for (int n = 1; n <= TILE_COUNT; n++) {
            int anchors[3] = {0, n - 1, n / 2};
            for (int p = 0; p < 3; p++) {
                Board b;
                make_line(&b, n, anchors[p], dbl);
                TileSlot s[TILE_COUNT];
                layout_board(&b, s);
                for (int i = 0; i < n; i++) {
                    RectF r = s[i].rect;
                    if (!(r.x >= t.x && r.y >= t.y && r.x + r.w <= t.x + t.w &&
                          r.y + r.h <= t.y + t.h)) {
                        fprintf(stderr, "fora: n=%d anchor=%d dbl=%d i=%d\n", n, anchors[p], dbl, i);
                        CHECK(false);
                    }
                    for (int j = i + 1; j < n; j++)
                        if (rect_overlaps(r, s[j].rect)) {
                            fprintf(stderr, "sobreposição: n=%d anchor=%d dbl=%d i=%d j=%d\n", n,
                                    anchors[p], dbl, i, j);
                            CHECK(false);
                        }
                }
            }
        }
    return 0;
}

static int layout_doubles_perpendicular(void)
{
    int doubles_seen = 0, vertical_segments = 0;
    for (int n = 1; n <= TILE_COUNT; n++) {
        int anchors[3] = {0, n - 1, n / 2};
        for (int p = 0; p < 3; p++) {
            Board b;
            make_line(&b, n, anchors[p], true);
            TileSlot s[TILE_COUNT];
            layout_board(&b, s);
            for (int i = 0; i < n; i++) {
                if (!s[i].horizontal_segment)
                    vertical_segments++;
                if (b.line[i].left == b.line[i].right) {
                    doubles_seen++;
                    CHECK(s[i].vertical == s[i].horizontal_segment);
                }
            }
        }
    }
    CHECK(doubles_seen > 0);
    CHECK(vertical_segments > 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"deal_28_unique_7_each", deal_28_unique_7_each},
        {"deal_same_seed_same_hands", deal_same_seed_same_hands},
        {"cli_parse_seed", cli_parse_seed},
        {"south_hand_sorted_by_sum", south_hand_sorted_by_sum},
        {"south_hand_in_bottom_band", south_hand_in_bottom_band},
        {"cpu_rows_match_hand_count", cpu_rows_match_hand_count},
        {"first_hand_double_six_starts", first_hand_double_six_starts},
        {"first_hand_rejects_non_double_six", first_hand_rejects_non_double_six},
        {"turn_order_counterclockwise", turn_order_counterclockwise},
        {"click_single_end_places", click_single_end_places},
        {"click_both_ends_waits_choice", click_both_ends_waits_choice},
        {"click_equal_ends_goes_right", click_equal_ends_goes_right},
        {"click_invalid_shows_message", click_invalid_shows_message},
        {"click_ignored_off_turn", click_ignored_off_turn},
        {"south_auto_pass", south_auto_pass},
        {"cpu_moves_at_800ms", cpu_moves_at_800ms},
        {"cpu_picks_heaviest", cpu_picks_heaviest},
        {"cpu_pass_message", cpu_pass_message},
        {"cpu_faces_hidden_in_play", cpu_faces_hidden_in_play},
        {"score_simples", score_simples},
        {"score_carroca", score_carroca},
        {"score_la_e_lo", score_la_e_lo},
        {"score_cruzada", score_cruzada},
        {"score_trancado_lower_sum", score_trancado_lower_sum},
        {"score_trancado_tie", score_trancado_tie},
        {"hand_end_overlay", hand_end_overlay},
        {"next_hand_winner_starts", next_hand_winner_starts},
        {"next_hand_after_tranque", next_hand_after_tranque},
        {"score_text_format", score_text_format},
        {"match_end_message", match_end_message},
        {"new_match_resets", new_match_resets},
        {"layout_first_tile_centered", layout_first_tile_centered},
        {"layout_within_table", layout_within_table},
        {"layout_doubles_perpendicular", layout_doubles_perpendicular},
    };
    return RUN_TESTS(cases);
}
