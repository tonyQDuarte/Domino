/* Testes da feature cpu-parceria-animacao. */
#include <math.h>
#include <stdbool.h>

#include "controller.h"
#include "domino.h"
#include "fixtures.h"
#include "harness.h"
#include "layout.h"

/* ---------- helpers ---------- */

static CpuView view_for(Seat seat, int own_n, const Tile *own, int partner_count, int left, int right)
{
    CpuView v;
    memset(&v, 0, sizeof v);
    v.seat = seat;
    v.own.count = own_n;
    for (int i = 0; i < own_n; i++)
        v.own.tiles[i] = own[i];
    for (int s = 0; s < SEAT_COUNT; s++)
        v.counts[s] = 7;
    v.counts[seat] = own_n;
    v.counts[(seat + 2) % SEAT_COUNT] = partner_count;
    v.empty = false;
    v.left = (uint8_t)left;
    v.right = (uint8_t)right;
    v.first_hand = false;
    return v;
}

static Seat partner_of(Seat s) { return (Seat)((s + 2) % SEAT_COUNT); }

static float cx(RectF r) { return r.x + r.w / 2; }
static float cy(RectF r) { return r.y + r.h / 2; }
static float dist(float ax, float ay, float bx, float by) { return hypotf(ax - bx, ay - by); }

static const Flying *flying_with_id(const View *v, int id)
{
    for (int i = 0; i < v->flying_count; i++)
        if (v->flying[i].id == id)
            return &v->flying[i];
    return NULL;
}

static void click_south(Controller *c, int index)
{
    RectF r[HAND_SIZE];
    layout_south_hand(c->game.hands[SEAT_SOUTH].count, r);
    ctl_click(c, cx(r[index]), cy(r[index]));
}

static void south_bate(Controller *c, int us_score);

/* Controlador com animações ligadas e a mesa pronta (sem distribuição em andamento). */
static void anim_setup(Controller *c, int left, int right, Seat turn)
{
    ctl_init(c, 7);
    ctl_set_animations(c, true);
    set_board_ends(&c->game, left, right);
    c->game.turn = turn;
    c->game.opener = SEAT_SOUTH;
    c->anim = ANIM_NONE;
    c->anim_ms = 0;
    c->turn_ms = 0;
    c->message_ms = 0;
    c->choosing = false;
}

/* ---------- S1 ---------- */

static int cpu_ignores_hidden_hands(void)
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
                /* junta as peças dos outros três e redistribui ao contrário */
                Game g2 = g;
                Tile pool[TILE_COUNT];
                int n = 0;
                for (int s = 0; s < SEAT_COUNT; s++)
                    if (s != (int)g.turn)
                        for (int i = 0; i < g.hands[s].count; i++)
                            pool[n++] = g.hands[s].tiles[i];
                int k = n - 1;
                for (int s = 0; s < SEAT_COUNT; s++)
                    if (s != (int)g.turn)
                        for (int i = 0; i < g2.hands[s].count; i++)
                            g2.hands[s].tiles[i] = pool[k--];
                bool hb = cpu_choose(&g2, &ib, &eb);
                CHECK(ha == hb);
                if (ha) {
                    CHECK(ia == ib);
                    CHECK(ea == eb);
                }
                compared++;
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

static int cpu_decide_from_view_only(void)
{
    bool (*decide)(const CpuView *, int *, End *) = cpu_decide; /* assinatura sem Game */
    Tile own[] = {T(1, 2), T(6, 5), T(1, 0)};
    CpuView v = view_for(SEAT_EAST, 3, own, 7, 6, 1);
    int idx;
    End end;
    CHECK(decide(&v, &idx, &end));
    CHECK(idx == 1);
    CHECK(end == END_LEFT);

    Game g;
    game_new_match(&g, 4);
    set_board_ends(&g, 2, 5);
    g.lacks[SEAT_WEST][3] = true;
    g.hands[SEAT_EAST].count = 5;
    CpuView w = cpu_view_of(&g, SEAT_NORTH);
    CHECK(w.seat == SEAT_NORTH);
    CHECK(w.own.count == g.hands[SEAT_NORTH].count);
    for (int i = 0; i < w.own.count; i++) {
        CHECK(w.own.tiles[i].a == g.hands[SEAT_NORTH].tiles[i].a);
        CHECK(w.own.tiles[i].b == g.hands[SEAT_NORTH].tiles[i].b);
    }
    for (int s = 0; s < SEAT_COUNT; s++)
        CHECK(w.counts[s] == g.hands[s].count);
    CHECK(w.counts[SEAT_EAST] == 5);
    CHECK(!w.empty && w.left == 2 && w.right == 5);
    CHECK(w.lacks[SEAT_WEST][3]);
    CHECK(!w.first_hand);
    return 0;
}

static int pass_records_lacks(void)
{
    Game g;
    game_new_match(&g, 2);
    set_board_ends(&g, 2, 5);
    memset(g.lacks, 0, sizeof g.lacks);
    g.turn = SEAT_EAST;
    Tile e[] = {T(0, 1), T(3, 4)};
    set_hand(&g, SEAT_EAST, e, 2);
    game_pass(&g);
    for (int v = 0; v <= 6; v++)
        CHECK(g.lacks[SEAT_EAST][v] == (v == 2 || v == 5));
    for (int s = 0; s < SEAT_COUNT; s++)
        if (s != SEAT_EAST)
            for (int v = 0; v <= 6; v++)
                CHECK(!g.lacks[s][v]);
    /* Norte joga; a marca do Leste continua */
    Tile n[] = {T(5, 6), T(1, 1)};
    set_hand(&g, SEAT_NORTH, n, 2);
    CHECK(game_play(&g, 0, END_RIGHT));
    CHECK(g.lacks[SEAT_EAST][2] && g.lacks[SEAT_EAST][5]);
    return 0;
}

static int new_hand_clears_lacks(void)
{
    Game g;
    game_new_match(&g, 3);
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int v = 0; v <= 6; v++)
            g.lacks[s][v] = true;
    g.phase = PHASE_HAND_OVER;
    g.next_opener = SEAT_EAST;
    game_next_hand(&g);
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int v = 0; v <= 6; v++)
            CHECK(!g.lacks[s][v]);
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int v = 0; v <= 6; v++)
            g.lacks[s][v] = true;
    game_restart_match(&g);
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int v = 0; v <= 6; v++)
            CHECK(!g.lacks[s][v]);
    return 0;
}

static int cpu_partner_mode_avoids_blocking(void)
{
    Tile own[] = {T(1, 6), T(3, 0), T(4, 4), T(5, 5)};
    CpuView v = view_for(SEAT_NORTH, 4, own, 2, 3, 1);
    v.lacks[SEAT_SOUTH][3] = v.lacks[SEAT_SOUTH][6] = true;
    int idx;
    End end;
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(v.own.tiles[idx].a == 3 && v.own.tiles[idx].b == 0);
    CHECK(end == END_LEFT);

    /* [1|6] como única jogada: joga mesmo trancando o parceiro */
    Tile only[] = {T(1, 6), T(4, 4), T(5, 5), T(2, 2)};
    CpuView w = view_for(SEAT_NORTH, 4, only, 2, 3, 1);
    w.lacks[SEAT_SOUTH][3] = w.lacks[SEAT_SOUTH][6] = true;
    CHECK(cpu_decide(&w, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_RIGHT);
    return 0;
}

static int cpu_self_mode_when_partner_not_fewer(void)
{
    Tile own[] = {T(1, 6), T(3, 0), T(4, 4), T(5, 5)};
    const int partner_counts[] = {5, 4};
    for (int k = 0; k < 2; k++) {
        CpuView v = view_for(SEAT_NORTH, 4, own, partner_counts[k], 3, 1);
        v.lacks[SEAT_SOUTH][3] = v.lacks[SEAT_SOUTH][6] = true;
        int idx;
        End end;
        CHECK(cpu_decide(&v, &idx, &end));
        CHECK(v.own.tiles[idx].a == 1 && v.own.tiles[idx].b == 6);
    }
    return 0;
}

static int cpu_prefers_opponent_pass(void)
{
    Tile own[] = {T(5, 2), T(5, 6), T(0, 0)};
    int idx;
    End end;

    /* modo próprio: Oeste (seguinte do Norte) passou no 4 e no 2 */
    CpuView v = view_for(SEAT_NORTH, 3, own, 7, 4, 5);
    v.lacks[SEAT_EAST][4] = v.lacks[SEAT_EAST][2] = true; /* não é o seguinte: não conta */
    v.lacks[SEAT_WEST][4] = v.lacks[SEAT_WEST][2] = true;
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_RIGHT);

    /* sem o passe do seguinte, vale o desempate pela soma */
    CpuView u = view_for(SEAT_NORTH, 3, own, 7, 4, 5);
    u.lacks[SEAT_EAST][4] = u.lacks[SEAT_EAST][2] = true;
    CHECK(cpu_decide(&u, &idx, &end));
    CHECK(idx == 1);

    /* modo próprio: AC 5 vem antes do AC 6. [5|2] faz o Oeste passar (0 peças minhas encaixam
       depois); [5|6] deixaria 1 peça minha encaixando, mas não faz ninguém passar */
    Tile own2[] = {T(5, 2), T(5, 6), T(6, 1)};
    CpuView x = view_for(SEAT_NORTH, 3, own2, 7, 4, 5);
    x.lacks[SEAT_WEST][4] = x.lacks[SEAT_WEST][2] = true;
    CHECK(cpu_decide(&x, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_RIGHT);

    /* modo parceiro, com o parceiro também sem 4 e 2: AC 4 vem antes */
    CpuView w = view_for(SEAT_NORTH, 3, own, 2, 4, 5);
    w.lacks[SEAT_WEST][4] = w.lacks[SEAT_WEST][2] = true;
    w.lacks[SEAT_SOUTH][4] = w.lacks[SEAT_SOUTH][2] = true;
    CHECK(cpu_decide(&w, &idx, &end));
    CHECK(idx == 1);
    return 0;
}

static int cpu_self_mode_keeps_own_moves(void)
{
    Tile own[] = {T(1, 3), T(2, 6), T(3, 4)};
    int idx;
    End end;
    CpuView v = view_for(SEAT_EAST, 3, own, 7, 1, 2);
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_LEFT);

    CpuView w = view_for(SEAT_EAST, 3, own, 2, 1, 2);
    CHECK(cpu_decide(&w, &idx, &end));
    CHECK(idx == 1);
    CHECK(end == END_RIGHT);
    return 0;
}

static int cpu_partner_pairs(void)
{
    const Seat cpus[] = {SEAT_NORTH, SEAT_EAST, SEAT_WEST};
    const Seat expected[] = {SEAT_SOUTH, SEAT_WEST, SEAT_EAST};
    Tile own[] = {T(1, 6), T(3, 0), T(4, 4), T(5, 5)};
    for (int k = 0; k < 3; k++) {
        Seat s = cpus[k];
        CHECK(partner_of(s) == expected[k]);
        int idx;
        End end;
        /* passes do parceiro ativam o AC 4 */
        CpuView v = view_for(s, 4, own, 2, 3, 1);
        v.lacks[expected[k]][3] = v.lacks[expected[k]][6] = true;
        CHECK(cpu_decide(&v, &idx, &end));
        CHECK(v.own.tiles[idx].a == 3);
        /* os mesmos passes em outro assento (o anterior, que não é o seguinte) não ativam */
        Seat other = (Seat)((s + 3) % SEAT_COUNT);
        CpuView w = view_for(s, 4, own, 2, 3, 1);
        w.lacks[other][3] = w.lacks[other][6] = true;
        CHECK(cpu_decide(&w, &idx, &end));
        CHECK(w.own.tiles[idx].a == 1 && w.own.tiles[idx].b == 6);
    }
    return 0;
}

static int cpu_tiebreak_like_before(void)
{
    int idx;
    End end;
    Tile h1[] = {T(1, 2), T(6, 5), T(1, 0)};
    CpuView v = view_for(SEAT_EAST, 3, h1, 7, 6, 1);
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 1);

    Tile h2[] = {T(6, 2), T(4, 4)};
    v = view_for(SEAT_EAST, 2, h2, 7, 6, 4);
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 1);
    CHECK(end == END_RIGHT);

    Tile h3[] = {T(3, 4), T(5, 2)};
    v = view_for(SEAT_EAST, 2, h3, 7, 3, 5);
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 0);

    Tile h4[] = {T(2, 5), T(0, 0)};
    v = view_for(SEAT_EAST, 2, h4, 7, 2, 5);
    CHECK(cpu_decide(&v, &idx, &end));
    CHECK(idx == 0);
    CHECK(end == END_LEFT);
    return 0;
}

/* ---------- S2 ---------- */

static int slide_moves_hand_to_board(void)
{
    /* Sul joga [3|5] na direita das pontas 1/3 */
    Controller c;
    anim_setup(&c, 1, 3, SEAT_SOUTH);
    Tile s[] = {T(3, 5), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, s, 2);
    RectF src[HAND_SIZE];
    layout_south_hand(2, src);
    click_south(&c, 0);
    CHECK(c.game.board.count == 2);
    TileSlot slots[TILE_COUNT];
    layout_board(&c.game.board, slots);
    RectF dst = slots[1].rect;
    View v;
    ctl_view(&c, &v);
    CHECK(v.flying_count == 1);
    CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(src[0]), cy(src[0])) < 0.5f);
    ctl_update(&c, 175);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 1);
    float total = dist(cx(src[0]), cy(src[0]), cx(dst), cy(dst));
    float to_dst = dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(dst), cy(dst));
    float to_src = dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(src[0]), cy(src[0]));
    CHECK(to_dst > 0.5f && to_src > 0.5f && to_dst < total && to_src < total);
    /* ease-out cúbica: aos 175 ms (metade do tempo) percorreu 87,5% do caminho */
    CHECK(fabsf(cx(v.flying[0].rect) - (cx(src[0]) + 0.875f * (cx(dst) - cx(src[0])))) < 0.5f);
    CHECK(fabsf(cy(v.flying[0].rect) - (cy(src[0]) + 0.875f * (cy(dst) - cy(src[0])))) < 0.5f);
    /* a peça voa de face, com a orientação e os valores do lugar de destino */
    CHECK(v.flying[0].face_up);
    CHECK(v.flying[0].vertical == slots[1].vertical);
    CHECK(v.flying[0].first == 3 && v.flying[0].second == 5);
    ctl_update(&c, 174);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 1);
    CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(dst), cy(dst)) < 1.0f);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 0);

    /* Leste (CPU) joga [3|4] na esquerda das pontas 3/1 */
    anim_setup(&c, 3, 1, SEAT_EAST);
    Tile e[] = {T(3, 4), T(6, 6)};
    set_hand(&c.game, SEAT_EAST, e, 2);
    RectF row[HAND_SIZE];
    layout_cpu_row(SEAT_EAST, 2, row);
    ctl_update(&c, CPU_DELAY_MS);
    CHECK(c.game.board.count == 2);
    layout_board(&c.game.board, slots);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 1);
    CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(row[0]), cy(row[0])) < 0.5f);
    CHECK(v.flying[0].face_up);
    CHECK(v.flying[0].first == 4 && v.flying[0].second == 3);
    ctl_update(&c, 349);
    ctl_view(&c, &v);
    CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(slots[0].rect), cy(slots[0].rect)) < 1.0f);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 0);
    return 0;
}

static int slide_hides_board_slot(void)
{
    Controller c;
    anim_setup(&c, 3, 1, SEAT_SOUTH);
    Tile s[] = {T(4, 3), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, s, 2);
    View v;
    ctl_view(&c, &v);
    CHECK(v.board_hidden == -1);
    click_south(&c, 0); /* encaixa só na esquerda: índice 0 */
    ctl_view(&c, &v);
    CHECK(v.board_hidden == 0);
    ctl_update(&c, 349);
    ctl_view(&c, &v);
    CHECK(v.board_hidden == 0);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(v.board_hidden == -1);

    anim_setup(&c, 1, 3, SEAT_SOUTH);
    Tile r[] = {T(3, 5), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, r, 2);
    click_south(&c, 0); /* direita: último índice */
    ctl_view(&c, &v);
    CHECK(v.board_hidden == 1);
    return 0;
}

static int slide_blocks_timer_and_clicks(void)
{
    Controller c;
    anim_setup(&c, 1, 3, SEAT_SOUTH);
    Tile s[] = {T(3, 5), T(6, 6)};
    Tile e[] = {T(5, 4), T(2, 2)};
    set_hand(&c.game, SEAT_SOUTH, s, 2);
    set_hand(&c.game, SEAT_EAST, e, 2);
    click_south(&c, 0);
    CHECK(c.game.turn == SEAT_EAST);
    ctl_update(&c, 350);
    ctl_update(&c, 799);
    CHECK(c.game.hands[SEAT_EAST].count == 2);
    ctl_update(&c, 1);
    CHECK(c.game.hands[SEAT_EAST].count == 1);

    /* Oeste joga, a vez é do Sul, mas o clique só vale depois do deslize */
    anim_setup(&c, 1, 3, SEAT_WEST);
    Tile w[] = {T(3, 2), T(6, 6)};
    Tile s2[] = {T(2, 4), T(6, 5)};
    set_hand(&c.game, SEAT_WEST, w, 2);
    set_hand(&c.game, SEAT_SOUTH, s2, 2);
    ctl_update(&c, CPU_DELAY_MS);
    CHECK(c.game.turn == SEAT_SOUTH);
    CHECK(c.anim == ANIM_SLIDE);
    int board = c.game.board.count;
    click_south(&c, 0);
    CHECK(c.game.board.count == board);
    CHECK(c.game.hands[SEAT_SOUTH].count == 2);
    ctl_update(&c, 350);
    click_south(&c, 0);
    CHECK(c.game.board.count == board + 1);
    CHECK(c.game.hands[SEAT_SOUTH].count == 1);
    return 0;
}

/* ---------- S3 ---------- */

static RectF hand_slot(int k)
{
    RectF r[HAND_SIZE];
    Seat s = (Seat)(k % 4);
    if (s == SEAT_SOUTH)
        layout_south_hand(HAND_SIZE, r);
    else
        layout_cpu_row(s, HAND_SIZE, r);
    return r[k / 4];
}

static void dealing(Controller *c, uint64_t seed)
{
    ctl_init(c, seed);
    ctl_set_animations(c, true);
}

static int deal_animation_timing(void)
{
    Controller c;
    dealing(&c, 1);
    RectF t = layout_table_area();
    View v;
    ctl_view(&c, &v);
    CHECK(v.flying_count == 1);
    CHECK(v.flying[0].id == 0);
    CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(t), cy(t)) < 0.5f);

    double now = 0;
    for (int k = 0; k < TILE_COUNT; k++) {
        double start = DEAL_STEP_MS * k;
        /* sai do centro */
        ctl_update(&c, start - now);
        now = start;
        ctl_view(&c, &v);
        const Flying *f = flying_with_id(&v, k);
        CHECK(f != NULL);
        CHECK(dist(cx(f->rect), cy(f->rect), cx(t), cy(t)) < 0.5f);
    }
    /* chegada: confere cada peça 1 ms antes de pousar */
    Controller d;
    dealing(&d, 1);
    now = 0;
    for (int k = 0; k < TILE_COUNT; k++) {
        double arrive = DEAL_STEP_MS * k + DEAL_FLIGHT_MS;
        ctl_update(&d, arrive - 1 - now);
        now = arrive - 1;
        ctl_view(&d, &v);
        const Flying *f = flying_with_id(&v, k);
        CHECK(f != NULL);
        RectF slot = hand_slot(k);
        CHECK(dist(cx(f->rect), cy(f->rect), cx(slot), cy(slot)) < 1.0f);
        ctl_update(&d, 1);
        now += 1;
        ctl_view(&d, &v);
        CHECK(flying_with_id(&v, k) == NULL);
    }
    CHECK(now == 1650);
    ctl_view(&d, &v);
    CHECK(v.flying_count == 0);

    /* na distribuição as peças voam de costas */
    Controller e;
    dealing(&e, 1);
    ctl_update(&e, 100);
    ctl_view(&e, &v);
    CHECK(v.flying_count >= 1);
    for (int i = 0; i < v.flying_count; i++)
        CHECK(!v.flying[i].face_up);

    /* a distribuição também começa na mão seguinte e na Nova partida */
    for (int match = 0; match < 2; match++) {
        Controller m;
        south_bate(&m, match ? 5 : 0);
        ctl_update(&m, 350); /* deslize */
        ctl_update(&m, 400); /* virada */
        CHECK(m.anim == ANIM_NONE);
        if (match)
            ctl_click(&m, cx(layout_new_match_button()), cy(layout_new_match_button()));
        else
            ctl_click(&m, 10, 10);
        CHECK(m.game.phase == PHASE_PLAYING);
        CHECK(m.anim == ANIM_DEAL);
        ctl_view(&m, &v);
        CHECK(v.flying_count == 1);
        CHECK(v.flying[0].id == 0);
        CHECK(dist(cx(v.flying[0].rect), cy(v.flying[0].rect), cx(t), cy(t)) < 0.5f);
        for (int s = 0; s < SEAT_COUNT; s++)
            CHECK(v.hand_shown[s] == 0);
    }
    return 0;
}

static int deal_shows_arrived_only(void)
{
    Controller c;
    dealing(&c, 2);
    for (int t = 0; t <= 1700; t += 25) {
        if (t > 0)
            ctl_update(&c, 25);
        View v;
        ctl_view(&c, &v);
        for (int s = 0; s < SEAT_COUNT; s++) {
            int arrived = 0;
            for (int k = 0; k < TILE_COUNT; k++)
                if (k % 4 == s && DEAL_STEP_MS * k + DEAL_FLIGHT_MS <= t)
                    arrived++;
            if (v.hand_shown[s] != arrived) {
                fprintf(stderr, "t=%d s=%d mostrado=%d esperado=%d\n", t, s, v.hand_shown[s], arrived);
                CHECK(false);
            }
        }
    }
    return 0;
}

static int deal_blocks_play(void)
{
    /* procura uma seed em que uma CPU abre e outra em que o Sul abre */
    uint64_t cpu_seed = 0, south_seed = 0;
    bool got_cpu = false, got_south = false;
    for (uint64_t s = 0; s < 100 && !(got_cpu && got_south); s++) {
        Game g;
        game_new_match(&g, s);
        if (g.turn == SEAT_SOUTH && !got_south) { south_seed = s; got_south = true; }
        if (g.turn != SEAT_SOUTH && !got_cpu) { cpu_seed = s; got_cpu = true; }
    }
    CHECK(got_cpu && got_south);

    Controller c;
    dealing(&c, cpu_seed);
    ctl_update(&c, 1649);
    ctl_update(&c, 1);
    ctl_update(&c, 799);
    CHECK(c.game.board.count == 0);
    ctl_update(&c, 1);
    CHECK(c.game.board.count == 1);

    dealing(&c, south_seed);
    int six = -1;
    for (int i = 0; i < c.game.hands[SEAT_SOUTH].count; i++)
        if (c.game.hands[SEAT_SOUTH].tiles[i].a == 6 && c.game.hands[SEAT_SOUTH].tiles[i].b == 6)
            six = i;
    CHECK(six >= 0);
    ctl_update(&c, 1000);
    click_south(&c, six);
    CHECK(c.game.board.count == 0);
    ctl_update(&c, 650);
    click_south(&c, six);
    CHECK(c.game.board.count == 1);

    /* Sul sem jogada não passa durante a distribuição */
    dealing(&c, cpu_seed);
    c.game.turn = SEAT_SOUTH;
    c.game.first_hand = false;
    set_board_ends(&c.game, 0, 0);
    Tile none[] = {T(1, 2)};
    set_hand(&c.game, SEAT_SOUTH, none, 1);
    ctl_update(&c, 1000);
    CHECK(c.game.turn == SEAT_SOUTH);
    CHECK(c.game.passes == 0);
    ctl_update(&c, 650);
    ctl_update(&c, 1);
    CHECK(c.game.turn == SEAT_EAST);
    return 0;
}

/* ---------- S4 ---------- */

static void south_bate(Controller *c, int us_score)
{
    anim_setup(c, 1, 3, SEAT_SOUTH);
    c->game.score[TEAM_US] = us_score;
    c->game.score[TEAM_THEM] = 0;
    Tile s[] = {T(3, 5)};
    set_hand(&c->game, SEAT_SOUTH, s, 1);
    click_south(c, 0);
}

static int cpus_face(const View *v, bool up)
{
    return v->face_up[SEAT_EAST] == up && v->face_up[SEAT_NORTH] == up && v->face_up[SEAT_WEST] == up;
}

static int reveal_flips_cpu_hands(void)
{
    Controller c;
    south_bate(&c, 0);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    View v;
    ctl_view(&c, &v);
    CHECK(cpus_face(&v, false));
    ctl_update(&c, 350);
    ctl_view(&c, &v);
    CHECK(c.anim == ANIM_REVEAL);
    CHECK(fabsf(v.flip_scale - 1.0f) < 1e-4f);
    CHECK(cpus_face(&v, false));
    ctl_update(&c, 199);
    ctl_view(&c, &v);
    CHECK(cpus_face(&v, false));
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(v.flip_scale < 0.05f);
    CHECK(cpus_face(&v, true));
    ctl_update(&c, 200);
    ctl_view(&c, &v);
    CHECK(fabsf(v.flip_scale - 1.0f) < 1e-4f);
    CHECK(cpus_face(&v, true));
    CHECK(c.anim == ANIM_NONE);

    /* tranque: a virada começa sem deslize */
    anim_setup(&c, 0, 0, SEAT_SOUTH);
    c.game.passes = 3;
    Tile one[] = {T(1, 2)};
    for (int s = 0; s < SEAT_COUNT; s++)
        set_hand(&c.game, (Seat)s, one, 1);
    ctl_update(&c, 1);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    CHECK(c.anim == ANIM_REVEAL);
    ctl_view(&c, &v);
    CHECK(v.flying_count == 0);
    CHECK(cpus_face(&v, false));
    ctl_update(&c, 400);
    ctl_view(&c, &v);
    CHECK(cpus_face(&v, true));
    CHECK(v.overlay_visible);
    return 0;
}

static int reveal_hides_overlay(void)
{
    for (int match = 0; match < 2; match++) {
        Controller c;
        south_bate(&c, match ? 5 : 0);
        CHECK(c.game.phase == (match ? PHASE_MATCH_OVER : PHASE_HAND_OVER));
        View v;
        for (int t = 0; t < 750; t += 10) {
            ctl_view(&c, &v);
            if (v.overlay_visible) {
                fprintf(stderr, "overlay visível em t=%d (match=%d)\n", t, match);
                CHECK(false);
            }
            ctl_update(&c, 10);
        }
        ctl_view(&c, &v);
        CHECK(v.overlay_visible);
    }
    return 0;
}

static int reveal_skip_on_input(void)
{
    for (int use_enter = 0; use_enter < 2; use_enter++) {
        Controller c;
        south_bate(&c, 0);
        ctl_update(&c, 350);
        ctl_update(&c, 50);
        CHECK(c.anim == ANIM_REVEAL);
        if (use_enter)
            ctl_enter(&c);
        else
            ctl_click(&c, 10, 10);
        View v;
        ctl_view(&c, &v);
        CHECK(v.overlay_visible);
        CHECK(fabsf(v.flip_scale - 1.0f) < 1e-4f);
        CHECK(c.game.phase == PHASE_HAND_OVER);
        if (use_enter)
            ctl_enter(&c);
        else
            ctl_click(&c, 10, 10);
        CHECK(c.game.phase == PHASE_PLAYING);
        CHECK(c.game.board.count == 0);
    }
    return 0;
}

/* ---------- S5 ---------- */

static int pulse_period_800(void)
{
    Controller c;
    anim_setup(&c, 2, 5, SEAT_SOUTH);
    Tile s[] = {T(2, 5), T(6, 6)};
    set_hand(&c.game, SEAT_SOUTH, s, 2);
    click_south(&c, 0);
    View v;
    ctl_view(&c, &v);
    CHECK(v.highlight_end[END_LEFT] && v.highlight_end[END_RIGHT]);
    float samples[800];
    float lo = 10, hi = -10;
    for (int t = 0; t < 800; t++) {
        ctl_view(&c, &v);
        samples[t] = v.pulse;
        if (v.pulse < lo) lo = v.pulse;
        if (v.pulse > hi) hi = v.pulse;
        ctl_update(&c, 1);
    }
    CHECK(lo >= 0.35f - 1e-6f && lo <= 0.36f);
    CHECK(hi >= 0.84f && hi <= 0.85f + 1e-6f);
    for (int t = 0; t < 800; t++) {
        ctl_view(&c, &v);
        CHECK(fabsf(v.pulse - samples[t]) < 1e-4f);
        ctl_update(&c, 1);
    }
    return 0;
}

/* ---------- S6 ---------- */

static int animations_off_by_default(void)
{
    Controller c;
    ctl_init(&c, 5);
    CHECK(!c.animate);
    View v;
    ctl_view(&c, &v);
    CHECK(v.flying_count == 0);
    for (int s = 0; s < SEAT_COUNT; s++)
        CHECK(v.hand_shown[s] == c.game.hands[s].count);
    CHECK(v.board_hidden == -1);

    ctl_init(&c, 5);
    set_board_ends(&c.game, 1, 3);
    c.game.turn = SEAT_SOUTH;
    Tile s[] = {T(3, 5)};
    set_hand(&c.game, SEAT_SOUTH, s, 1);
    click_south(&c, 0);
    CHECK(c.game.phase == PHASE_HAND_OVER);
    ctl_view(&c, &v);
    CHECK(v.overlay_visible);
    CHECK(v.flying_count == 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"cpu_ignores_hidden_hands", cpu_ignores_hidden_hands},
        {"cpu_decide_from_view_only", cpu_decide_from_view_only},
        {"pass_records_lacks", pass_records_lacks},
        {"new_hand_clears_lacks", new_hand_clears_lacks},
        {"cpu_partner_mode_avoids_blocking", cpu_partner_mode_avoids_blocking},
        {"cpu_self_mode_when_partner_not_fewer", cpu_self_mode_when_partner_not_fewer},
        {"cpu_prefers_opponent_pass", cpu_prefers_opponent_pass},
        {"cpu_self_mode_keeps_own_moves", cpu_self_mode_keeps_own_moves},
        {"cpu_partner_pairs", cpu_partner_pairs},
        {"cpu_tiebreak_like_before", cpu_tiebreak_like_before},
        {"slide_moves_hand_to_board", slide_moves_hand_to_board},
        {"slide_hides_board_slot", slide_hides_board_slot},
        {"slide_blocks_timer_and_clicks", slide_blocks_timer_and_clicks},
        {"deal_animation_timing", deal_animation_timing},
        {"deal_shows_arrived_only", deal_shows_arrived_only},
        {"deal_blocks_play", deal_blocks_play},
        {"reveal_flips_cpu_hands", reveal_flips_cpu_hands},
        {"reveal_hides_overlay", reveal_hides_overlay},
        {"reveal_skip_on_input", reveal_skip_on_input},
        {"pulse_period_800", pulse_period_800},
        {"animations_off_by_default", animations_off_by_default},
    };
    return RUN_TESTS(cases);
}
