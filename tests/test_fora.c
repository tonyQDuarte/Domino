/* Testes da feature pecas-de-fora. */
#include <math.h>
#include <stdbool.h>

#include "controller.h"
#include "domino.h"
#include "fixtures.h"
#include "harness.h"
#include "layout.h"

static float cx(RectF r) { return r.x + r.w / 2; }
static float cy(RectF r) { return r.y + r.h / 2; }
static float dist(float ax, float ay, float bx, float by) { return hypotf(ax - bx, ay - by); }

/* Maior carroça nas mãos: valor e quem a tem. */
static int highest_double(const Game *g, Seat *holder)
{
    for (int d = 6; d >= 0; d--)
        for (int s = 0; s < SEAT_COUNT; s++)
            for (int i = 0; i < g->hands[s].count; i++)
                if (g->hands[s].tiles[i].a == d && g->hands[s].tiles[i].b == d) {
                    if (holder)
                        *holder = (Seat)s;
                    return d;
                }
    return -1;
}

static int find_tile(const Hand *h, int a, int b)
{
    for (int i = 0; i < h->count; i++)
        if ((h->tiles[i].a == a && h->tiles[i].b == b) || (h->tiles[i].a == b && h->tiles[i].b == a))
            return i;
    return -1;
}

static void click_south(Controller *c, int index)
{
    RectF r[HAND_SIZE];
    layout_south_hand(c->game.hands[SEAT_SOUTH].count, r);
    ctl_click(c, cx(r[index]), cy(r[index]));
}

/* Sul bate com [3|5] nas pontas 1/3. */
static void south_bate(Controller *c, bool animate, int us_score)
{
    ctl_init(c, 7);
    if (animate) {
        ctl_set_animations(c, true);
        c->anim = ANIM_NONE;
    }
    set_board_ends(&c->game, 1, 3);
    c->game.turn = SEAT_SOUTH;
    c->game.score[TEAM_US] = us_score;
    c->game.score[TEAM_THEM] = 0;
    Tile s[] = {T(3, 5)};
    set_hand(&c->game, SEAT_SOUTH, s, 1);
    click_south(c, 0);
}

/* ---------- S1 ---------- */

static int deal_6_each_4_out(void)
{
    for (uint64_t seed = 0; seed < 1000; seed++) {
        Game g;
        game_new_match(&g, seed);
        int seen[7][7] = {{0}};
        int total = 0;
        for (int s = 0; s < SEAT_COUNT; s++) {
            CHECK(g.hands[s].count == 6);
            for (int i = 0; i < g.hands[s].count; i++) {
                Tile t = g.hands[s].tiles[i];
                seen[t.a < t.b ? t.a : t.b][t.a < t.b ? t.b : t.a]++;
                total++;
            }
        }
        for (int i = 0; i < 4; i++) {
            Tile t = g.sleeping[i];
            CHECK(t.a <= 6 && t.b <= 6);
            seen[t.a < t.b ? t.a : t.b][t.a < t.b ? t.b : t.a]++;
            total++;
        }
        CHECK(total == 28);
        for (int a = 0; a <= 6; a++)
            for (int b = a; b <= 6; b++)
                CHECK(seen[a][b] == 1);
        /* a mão seguinte também */
        g.phase = PHASE_HAND_OVER;
        game_next_hand(&g);
        int n = 0;
        for (int s = 0; s < SEAT_COUNT; s++) {
            CHECK(g.hands[s].count == 6);
            n += g.hands[s].count;
        }
        CHECK(n + 4 == 28);
    }
    return 0;
}

static int sleeping_corner_face_down(void)
{
    Controller c;
    ctl_init(&c, 3);
    View v;
    ctl_view(&c, &v);
    CHECK(v.phase == PHASE_PLAYING);
    CHECK(v.sleeping_shown == 4);
    CHECK(!v.sleeping_face_up);

    RectF r[SLEEPING_COUNT], north[HAND_SIZE], east[HAND_SIZE];
    layout_sleeping(r);
    layout_cpu_row(SEAT_NORTH, HAND_SIZE, north);
    layout_cpu_row(SEAT_EAST, HAND_SIZE, east);
    RectF table = layout_table_area();
    RectF score = {0, 0, 300, 45};
    for (int i = 0; i < SLEEPING_COUNT; i++) {
        CHECK(r[i].x >= 1000);
        CHECK(r[i].y >= 0 && r[i].y + r[i].h <= 110);
        CHECK(r[i].x + r[i].w <= SCREEN_W);
        CHECK(!rect_overlaps(r[i], table));
        CHECK(!rect_overlaps(r[i], score));
        for (int k = 0; k < HAND_SIZE; k++) {
            CHECK(!rect_overlaps(r[i], north[k]));
            CHECK(!rect_overlaps(r[i], east[k]));
        }
        for (int j = i + 1; j < SLEEPING_COUNT; j++)
            CHECK(!rect_overlaps(r[i], r[j]));
    }
    return 0;
}

static int cpu_ignores_sleeping(void)
{
    int compared = 0;
    for (uint64_t seed = 0; seed < 200; seed++) {
        Game g;
        game_new_match(&g, seed);
        for (int step = 0; step < 60 && g.phase == PHASE_PLAYING; step++) {
            int ia, ib;
            End ea, eb;
            bool ha = cpu_choose(&g, &ia, &ea);
            if (g.turn != SEAT_SOUTH) {
                Game g2 = g;
                int swapped = 0;
                for (int k = 1; k < SEAT_COUNT && swapped < SLEEPING_COUNT; k++) {
                    Seat other = (Seat)((g.turn + k) % SEAT_COUNT);
                    for (int i = 0; i < g2.hands[other].count && swapped < SLEEPING_COUNT; i++) {
                        Tile t = g2.hands[other].tiles[i];
                        g2.hands[other].tiles[i] = g2.sleeping[swapped];
                        g2.sleeping[swapped] = t;
                        swapped++;
                    }
                }
                if (swapped > 0) {
                    bool hb = cpu_choose(&g2, &ib, &eb);
                    CHECK(ha == hb);
                    if (ha) {
                        CHECK(ia == ib);
                        CHECK(ea == eb);
                    }
                    compared++;
                }
            }
            if (ha)
                game_play(&g, ia, ea);
            else
                game_pass(&g);
        }
    }
    CHECK(compared > 1000);
    return 0;
}

static RectF deal_slot(int k)
{
    RectF r[HAND_SIZE], z[SLEEPING_COUNT];
    if (k >= 24) {
        layout_sleeping(z);
        return z[k - 24];
    }
    Seat s = (Seat)(k % 4);
    if (s == SEAT_SOUTH)
        layout_south_hand(HAND_SIZE, r);
    else
        layout_cpu_row(s, HAND_SIZE, r);
    return r[k / 4];
}

static const Flying *flying_with_id(const View *v, int id)
{
    for (int i = 0; i < v->flying_count; i++)
        if (v->flying[i].id == id)
            return &v->flying[i];
    return NULL;
}

static int deal_sends_four_to_corner(void)
{
    RectF t = layout_table_area();
    Controller c;
    ctl_init(&c, 1);
    ctl_set_animations(&c, true);
    double now = 0;
    View v;
    for (int k = 0; k < TILE_COUNT; k++) {
        double start = DEAL_STEP_MS * k;
        ctl_update(&c, start - now);
        now = start;
        ctl_view(&c, &v);
        const Flying *f = flying_with_id(&v, k);
        CHECK(f != NULL);
        CHECK(dist(cx(f->rect), cy(f->rect), cx(t), cy(t)) < 0.5f);
    }
    Controller d;
    ctl_init(&d, 1);
    ctl_set_animations(&d, true);
    now = 0;
    for (int k = 0; k < TILE_COUNT; k++) {
        double arrive = DEAL_STEP_MS * k + DEAL_FLIGHT_MS;
        ctl_update(&d, arrive - 1 - now);
        now = arrive - 1;
        ctl_view(&d, &v);
        const Flying *f = flying_with_id(&v, k);
        CHECK(f != NULL);
        RectF slot = deal_slot(k);
        if (dist(cx(f->rect), cy(f->rect), cx(slot), cy(slot)) >= 1.0f) {
            fprintf(stderr, "peça %d não chegou ao lugar\n", k);
            CHECK(false);
        }
        ctl_update(&d, 1);
        now += 1;
        ctl_view(&d, &v);
        CHECK(flying_with_id(&v, k) == NULL);
    }
    CHECK(now == 1650);
    CHECK(v.flying_count == 0);
    return 0;
}

static int deal_corner_shows_arrived_only(void)
{
    Controller c;
    ctl_init(&c, 2);
    ctl_set_animations(&c, true);
    for (int t = 0; t <= 1700; t += 25) {
        if (t > 0)
            ctl_update(&c, 25);
        View v;
        ctl_view(&c, &v);
        int arrived = 0;
        for (int k = 24; k < TILE_COUNT; k++)
            if (DEAL_STEP_MS * k + DEAL_FLIGHT_MS <= t)
                arrived++;
        if (v.sleeping_shown != arrived) {
            fprintf(stderr, "t=%d mostradas=%d esperado=%d\n", t, v.sleeping_shown, arrived);
            CHECK(false);
        }
    }
    return 0;
}

/* ---------- S2 ---------- */

static int first_hand_highest_double(void)
{
    int six_out = 0;
    for (uint64_t seed = 0; seed < 1000; seed++) {
        Game g;
        game_new_match(&g, seed);
        Seat holder;
        int d = highest_double(&g, &holder);
        CHECK(d >= 0);
        CHECK(g.turn == holder);
        for (int i = 0; i < 4; i++)
            if (g.sleeping[i].a == 6 && g.sleeping[i].b == 6)
                six_out++;
        Hand *h = &g.hands[g.turn];
        int idx = find_tile(h, d, d);
        CHECK(idx >= 0);
        for (int i = 0; i < h->count; i++) {
            if (i == idx)
                continue;
            CHECK(!game_play(&g, i, END_LEFT));
            CHECK(g.board.count == 0);
        }
        int ci;
        End ce;
        CHECK(cpu_choose(&g, &ci, &ce));
        CHECK(ci == idx);
        CHECK(game_play(&g, idx, END_LEFT));
        CHECK(g.board.line[0].left == d && g.board.line[0].right == d);
    }
    CHECK(six_out > 0);
    return 0;
}

static int south_first_open_invalid(void)
{
    uint64_t seed = 0;
    bool found = false;
    for (uint64_t s = 0; s < 200 && !found; s++) {
        Game g;
        game_new_match(&g, s);
        if (g.turn == SEAT_SOUTH) {
            seed = s;
            found = true;
        }
    }
    CHECK(found);
    Controller c;
    ctl_init(&c, seed);
    int d = highest_double(&c.game, NULL);
    int idx = find_tile(&c.game.hands[SEAT_SOUTH], d, d);
    CHECK(idx >= 0);
    int other = idx == 0 ? 1 : 0;
    click_south(&c, other);
    CHECK(c.game.board.count == 0);
    View v;
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Jogada inválida") == 0);
    click_south(&c, idx);
    CHECK(c.game.board.count == 1);
    CHECK(c.game.board.line[0].left == d);
    return 0;
}

/* ---------- S3 ---------- */

static int reveal_flips_sleeping(void)
{
    Controller c;
    south_bate(&c, true, 0);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    View v;
    ctl_view(&c, &v);
    CHECK(!v.sleeping_face_up);
    CHECK(v.sleeping_shown == 4);
    ctl_update(&c, 350);
    ctl_view(&c, &v);
    CHECK(!v.sleeping_face_up);
    CHECK(v.sleeping_face_up == v.face_up[SEAT_EAST]);
    ctl_update(&c, 199);
    ctl_view(&c, &v);
    CHECK(!v.sleeping_face_up);
    CHECK(v.flip_scale < 0.01f);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(v.sleeping_face_up);
    CHECK(v.sleeping_face_up == v.face_up[SEAT_EAST]);
    CHECK(v.sleeping_shown == 4);
    return 0;
}

static int sleeping_face_up_without_animation(void)
{
    Controller c;
    south_bate(&c, false, 0);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    View v;
    ctl_view(&c, &v);
    CHECK(v.sleeping_face_up);
    CHECK(v.sleeping_shown == 4);
    return 0;
}

static int sleeping_shown_with_overlay(void)
{
    for (int match = 0; match < 2; match++) {
        Controller c;
        south_bate(&c, true, match ? 5 : 0);
        CHECK(c.game.phase == (match ? PHASE_MATCH_OVER : PHASE_HAND_OVER));
        ctl_update(&c, 350);
        ctl_update(&c, 400);
        View v;
        ctl_view(&c, &v);
        CHECK(v.overlay_visible);
        CHECK(v.sleeping_face_up);
        CHECK(v.sleeping_shown == 4);
        ctl_update(&c, 5000);
        ctl_view(&c, &v);
        CHECK(v.overlay_visible && v.sleeping_face_up);
    }
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"deal_6_each_4_out", deal_6_each_4_out},
        {"sleeping_corner_face_down", sleeping_corner_face_down},
        {"cpu_ignores_sleeping", cpu_ignores_sleeping},
        {"deal_sends_four_to_corner", deal_sends_four_to_corner},
        {"deal_corner_shows_arrived_only", deal_corner_shows_arrived_only},
        {"first_hand_highest_double", first_hand_highest_double},
        {"south_first_open_invalid", south_first_open_invalid},
        {"reveal_flips_sleeping", reveal_flips_sleeping},
        {"sleeping_face_up_without_animation", sleeping_face_up_without_animation},
        {"sleeping_shown_with_overlay", sleeping_shown_with_overlay},
    };
    return RUN_TESTS(cases);
}
