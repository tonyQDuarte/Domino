/* Testes da feature bots-parceiros (regras, menu, controlador e layout). */
#include <stdbool.h>

#include "bots.h"
#include "controller.h"
#include "domino.h"
#include "fixtures.h"
#include "harness.h"
#include "layout.h"
#include "menu.h"

/* ---------- helpers ---------- */

static CpuView view_for(Seat seat, CpuStyle style, int own_n, const Tile *own, int partner_count,
                        int left, int right)
{
    CpuView v;
    memset(&v, 0, sizeof v);
    v.seat = seat;
    v.style = style;
    v.own.count = own_n;
    for (int i = 0; i < own_n; i++)
        v.own.tiles[i] = own[i];
    for (int s = 0; s < SEAT_COUNT; s++)
        v.counts[s] = 7;
    v.counts[seat] = own_n;
    v.counts[(seat + 2) % SEAT_COUNT] = partner_count;
    v.left = (uint8_t)left;
    v.right = (uint8_t)right;
    return v;
}

/* Cenário P: Norte com [1|6] [3|0] [4|4] [5|5], pontas 3 e 1, Sul passou no 3 e no 6. */
static CpuView scenario_p(CpuStyle style, int partner_count)
{
    Tile own[] = {T(1, 6), T(3, 0), T(4, 4), T(5, 5)};
    CpuView v = view_for(SEAT_NORTH, style, 4, own, partner_count, 3, 1);
    v.lacks[SEAT_SOUTH][3] = v.lacks[SEAT_SOUTH][6] = true;
    return v;
}

/* Cenário L: Norte com [5|2] [5|6] [0|0], parceiro com 2, pontas 4 e 5, Sul e Oeste sem 4 e 2. */
static CpuView scenario_l(CpuStyle style)
{
    Tile own[] = {T(5, 2), T(5, 6), T(0, 0)};
    CpuView v = view_for(SEAT_NORTH, style, 3, own, 2, 4, 5);
    v.lacks[SEAT_SOUTH][4] = v.lacks[SEAT_SOUTH][2] = true;
    v.lacks[SEAT_WEST][4] = v.lacks[SEAT_WEST][2] = true;
    return v;
}

/* Cenário J: Leste com [1|3] [2|6] [3|4], pontas 1 e 2, parceiro com 7, ninguém passou. */
static CpuView scenario_j(CpuStyle style)
{
    Tile own[] = {T(1, 3), T(2, 6), T(3, 4)};
    return view_for(SEAT_EAST, style, 3, own, 7, 1, 2);
}

/* A peça escolhida é [a|b] (e, se end >= 0, nessa ponta). */
static bool picks(const CpuView *v, int a, int b, int end)
{
    int idx;
    End e;
    if (!cpu_decide(v, &idx, &e))
        return false;
    Tile t = v->own.tiles[idx];
    return t.a == a && t.b == b && (end < 0 || (int)e == end);
}

static float cx(RectF r) { return r.x + r.w / 2; }
static float cy(RectF r) { return r.y + r.h / 2; }

static bool bots_ok(const Controller *c, int partner)
{
    int n = c->bot[SEAT_NORTH], e = c->bot[SEAT_EAST], w = c->bot[SEAT_WEST];
    if (n != partner || e < 0 || e >= BOT_COUNT || w < 0 || w >= BOT_COUNT)
        return false;
    if (e == w || e == partner || w == partner)
        return false;
    return c->game.styles[SEAT_NORTH] == BOTS[n].style && c->game.styles[SEAT_EAST] == BOTS[e].style &&
           c->game.styles[SEAT_WEST] == BOTS[w].style;
}

/* ---------- S1 ---------- */

static int bots_table(void)
{
    static const char *names[] = {"Seu Paulo", "Neca", "Audi", "Seu Leo", "Guede", "Manezinho"};
    static const char *texts[] = {"Fidelidade total", "Colaboração", "Equilíbrio",
                                  "Bloqueio", "Jogo próprio", "Descarrego"};
    static const CpuStyle styles[] = {STYLE_SEU_PAULO, STYLE_NECA, STYLE_AUDI,
                                      STYLE_SEU_LEO, STYLE_GUEDE, STYLE_MANEZINHO};
    CHECK(BOT_COUNT == 6);
    for (int i = 0; i < 6; i++) {
        CHECK(strcmp(BOTS[i].name, names[i]) == 0);
        CHECK(strcmp(BOTS[i].style_text, texts[i]) == 0);
        CHECK(BOTS[i].style == styles[i]);
        for (int j = i + 1; j < 6; j++)
            CHECK(BOTS[i].style != BOTS[j].style);
    }
    return 0;
}

static int style_seu_paulo_always_partner(void)
{
    const int counts[] = {2, 4, 5};
    for (int k = 0; k < 3; k++) {
        CpuView v = scenario_p(STYLE_SEU_PAULO, counts[k]);
        CHECK(picks(&v, 3, 0, -1));
    }
    return 0;
}

static int style_neca_partner_when_not_more(void)
{
    CpuView v = scenario_p(STYLE_NECA, 2);
    CHECK(picks(&v, 3, 0, -1));
    v = scenario_p(STYLE_NECA, 4);
    CHECK(picks(&v, 3, 0, -1));
    v = scenario_p(STYLE_NECA, 5);
    CHECK(picks(&v, 1, 6, -1));
    return 0;
}

static int style_audi_is_current_rule(void)
{
    CHECK(STYLE_AUDI == 0);
    CpuView v = scenario_p(STYLE_AUDI, 2);
    CHECK(picks(&v, 3, 0, -1));
    v = scenario_p(STYLE_AUDI, 4);
    CHECK(picks(&v, 1, 6, -1));
    v = scenario_p(STYLE_AUDI, 5);
    CHECK(picks(&v, 1, 6, -1));
    v = scenario_l(STYLE_AUDI);
    CHECK(picks(&v, 5, 6, -1));
    v = scenario_j(STYLE_AUDI);
    CHECK(picks(&v, 1, 3, END_LEFT));
    return 0;
}

static int style_seu_leo_blocks_first(void)
{
    CpuView v = scenario_l(STYLE_SEU_LEO);
    CHECK(picks(&v, 5, 2, -1));
    v = scenario_p(STYLE_SEU_LEO, 2);
    CHECK(picks(&v, 3, 0, -1));
    v = scenario_p(STYLE_SEU_LEO, 4);
    CHECK(picks(&v, 1, 6, -1));
    return 0;
}

static int style_guede_never_partner(void)
{
    CpuView v = scenario_p(STYLE_GUEDE, 2);
    CHECK(picks(&v, 1, 6, -1));
    v = scenario_l(STYLE_GUEDE);
    CHECK(picks(&v, 5, 2, -1));
    v = scenario_j(STYLE_GUEDE);
    CHECK(picks(&v, 1, 3, END_LEFT));
    return 0;
}

static int style_manezinho_tiebreak_only(void)
{
    CpuView v = scenario_p(STYLE_MANEZINHO, 2);
    CHECK(picks(&v, 1, 6, -1));
    v = scenario_l(STYLE_MANEZINHO);
    CHECK(picks(&v, 5, 6, -1));
    v = scenario_j(STYLE_MANEZINHO);
    CHECK(picks(&v, 2, 6, END_RIGHT));
    return 0;
}

/* Embaralha as peças escondidas de quem não é `s` (mãos dos outros e peças de fora). */
static void shuffle_hidden(Game *g, Seat s, uint64_t *state)
{
    Tile pool[TILE_COUNT];
    int n = 0;
    for (int k = 0; k < SEAT_COUNT; k++)
        if (k != (int)s)
            for (int i = 0; i < g->hands[k].count; i++)
                pool[n++] = g->hands[k].tiles[i];
    for (int i = 0; i < SLEEPING_COUNT; i++)
        pool[n++] = g->sleeping[i];
    for (int i = n - 1; i > 0; i--) {
        *state = *state * 6364136223846793005ULL + 1442695040888963407ULL;
        int j = (int)((*state >> 33) % (uint64_t)(i + 1));
        Tile t = pool[i];
        pool[i] = pool[j];
        pool[j] = t;
    }
    int p = 0;
    for (int k = 0; k < SEAT_COUNT; k++)
        if (k != (int)s)
            for (int i = 0; i < g->hands[k].count; i++)
                g->hands[k].tiles[i] = pool[p++];
    for (int i = 0; i < SLEEPING_COUNT; i++)
        g->sleeping[i] = pool[p++];
}

static int styles_decide_from_view_only(void)
{
    int compared = 0;
    for (int style = 0; style < 6; style++) {
        for (uint64_t seed = 0; seed < 100; seed++) {
            Game g;
            game_new_match(&g, seed);
            g.styles[SEAT_EAST] = g.styles[SEAT_NORTH] = g.styles[SEAT_WEST] = (CpuStyle)style;
            uint64_t mix = seed * 31 + (uint64_t)style + 1;
            for (int step = 0; step < 60 && g.phase == PHASE_PLAYING; step++) {
                int ia, ib;
                End ea, eb;
                bool oka = cpu_choose(&g, &ia, &ea);
                if (g.turn != SEAT_SOUTH) {
                    CpuView v = cpu_view_of(&g, g.turn);
                    CHECK(v.style == g.styles[g.turn]);
                    CHECK(v.style == (CpuStyle)style);
                    for (int r = 0; r < 3; r++) {
                        Game h = g;
                        shuffle_hidden(&h, g.turn, &mix);
                        bool okb = cpu_choose(&h, &ib, &eb);
                        CHECK(oka == okb);
                        if (oka) {
                            CHECK(ia == ib);
                            CHECK(ea == eb);
                        }
                        compared++;
                    }
                }
                if (oka)
                    CHECK(game_play(&g, ia, ea));
                else
                    game_pass(&g);
            }
        }
    }
    CHECK(compared > 1000);
    return 0;
}

/* ---------- S2 ---------- */

static int menu_partner_default_audi(void)
{
    CHECK(strcmp(MENU_PARTNER_LABEL, "Seu parceiro") == 0);
    Menu m;
    menu_init(&m, NULL);
    CHECK(m.partner == 2);
    menu_init(&m, "Ana");
    CHECK(m.partner == 2);
    CHECK(strcmp(BOTS[m.partner].name, "Audi") == 0);
    return 0;
}

static int menu_card_click_selects(void)
{
    Menu m;
    menu_init(&m, "Ana");
    for (int i = 0; i < 6; i++) {
        RectF r = menu_card_rect(i);
        CHECK(menu_click(&m, cx(r), cy(r)) == MENU_NONE);
        CHECK(menu_pick_partner(&m, cx(r), cy(r)));
        CHECK(m.partner == i);
        CHECK(menu_pick_partner(&m, cx(r), cy(r))); /* de novo: continua selecionado */
        CHECK(m.partner == i);
    }
    const int order[] = {5, 0, 3};
    for (int k = 0; k < 3; k++) {
        RectF r = menu_card_rect(order[k]);
        CHECK(menu_pick_partner(&m, cx(r), cy(r)));
        CHECK(m.partner == order[k]);
    }
    RectF t = menu_title_rect();
    CHECK(!menu_pick_partner(&m, cx(t), cy(t)));
    CHECK(menu_click(&m, cx(t), cy(t)) == MENU_NONE);
    CHECK(m.partner == 3);
    CHECK(!menu_pick_partner(&m, 20, 700));
    CHECK(menu_click(&m, 20, 700) == MENU_NONE);
    CHECK(m.partner == 3);
    return 0;
}

static bool rect_eq(RectF a, float x, float y, float w, float h)
{
    return a.x == x && a.y == y && a.w == w && a.h == h;
}

static int menu_partner_layout(void)
{
    CHECK(rect_eq(menu_play_rect(), 540, 360, 200, 55));
    CHECK(rect_eq(menu_quit_rect(), 540, 440, 200, 55));
    RectF r[12];
    int n = 0;
    r[n++] = menu_title_rect();
    r[n++] = menu_label_rect();
    r[n++] = menu_field_rect();
    r[n++] = menu_partner_label_rect();
    for (int i = 0; i < 6; i++)
        r[n++] = menu_card_rect(i);
    r[n++] = menu_play_rect();
    r[n++] = menu_quit_rect();
    for (int i = 0; i < 12; i++) {
        CHECK(r[i].w > 0 && r[i].h > 0);
        CHECK(r[i].x >= 0 && r[i].y >= 0 && r[i].x + r[i].w <= SCREEN_W && r[i].y + r[i].h <= SCREEN_H);
        for (int j = i + 1; j < 12; j++)
            CHECK(!rect_overlaps(r[i], r[j]));
    }
    /* título, rótulo do nome, campo, rótulo do parceiro de cima para baixo */
    for (int i = 1; i < 4; i++)
        CHECK(r[i].y >= r[i - 1].y + r[i - 1].h);
    /* cartões numa faixa só, abaixo do rótulo do parceiro e acima de Jogar */
    for (int i = 4; i < 10; i++) {
        CHECK(r[i].y == r[4].y && r[i].h == r[4].h);
        CHECK(r[i].y >= r[3].y + r[3].h);
        CHECK(r[i].y + r[i].h <= r[10].y);
        if (i > 4)
            CHECK(r[i].x >= r[i - 1].x + r[i - 1].w); /* na ordem da esquerda para a direita */
    }
    CHECK(r[11].y >= r[10].y + r[10].h);
    return 0;
}

static int bots_seating_by_seed(void)
{
    for (int p = 0; p < BOT_COUNT; p++) {
        bool seen[BOT_COUNT] = {false};
        for (uint64_t seed = 0; seed < 100; seed++) {
            Controller a, b, plain;
            ctl_init(&a, seed);
            ctl_set_bots(&a, p, seed);
            CHECK(bots_ok(&a, p));
            ctl_init(&b, seed);
            ctl_set_bots(&b, p, seed);
            CHECK(a.bot[SEAT_EAST] == b.bot[SEAT_EAST] && a.bot[SEAT_WEST] == b.bot[SEAT_WEST]);
            ctl_init(&plain, seed);
            CHECK(memcmp(a.game.hands, plain.game.hands, sizeof a.game.hands) == 0);
            CHECK(memcmp(a.game.sleeping, plain.game.sleeping, sizeof a.game.sleeping) == 0);
            seen[a.bot[SEAT_EAST]] = seen[a.bot[SEAT_WEST]] = true;
        }
        for (int i = 0; i < BOT_COUNT; i++)
            CHECK(seen[i] == (i != p));
    }
    return 0;
}

static int new_match_keeps_partner_redraws(void)
{
    Controller c;
    ctl_init(&c, 3);
    ctl_set_bots(&c, 1, 3);
    CHECK(bots_ok(&c, 1));
    int pairs[20][2], distinct = 0;
    RectF btn = layout_new_match_button();
    for (int k = 0; k < 20; k++) {
        c.game.phase = PHASE_MATCH_OVER;
        ctl_click(&c, cx(btn), cy(btn));
        CHECK(c.game.phase == PHASE_PLAYING);
        CHECK(bots_ok(&c, 1));
        bool found = false;
        for (int j = 0; j < distinct; j++)
            if (pairs[j][0] == c.bot[SEAT_EAST] && pairs[j][1] == c.bot[SEAT_WEST])
                found = true;
        if (!found) {
            pairs[distinct][0] = c.bot[SEAT_EAST];
            pairs[distinct][1] = c.bot[SEAT_WEST];
            distinct++;
        }
    }
    CHECK(distinct >= 2);
    return 0;
}

/* ---------- S3 ---------- */

static int bot_names_on_table(void)
{
    Controller c;
    ctl_init(&c, 11);
    ctl_set_bots(&c, 4, 11);
    View v;
    ctl_view(&c, &v);
    CHECK(v.seat_name[SEAT_SOUTH][0] == '\0');
    const Seat cpus[] = {SEAT_NORTH, SEAT_EAST, SEAT_WEST};
    for (int k = 0; k < 3; k++)
        CHECK(strcmp(v.seat_name[cpus[k]], BOTS[c.bot[cpus[k]]].name) == 0);

    RectF others[3];
    for (int k = 0; k < 3; k++)
        others[k] = layout_bot_name(cpus[k]);
    RectF score = {0, 0, 300, 50};
    RectF sleeping[SLEEPING_COUNT];
    layout_sleeping(sleeping);
    for (int k = 0; k < 3; k++) {
        RectF r = others[k];
        CHECK(r.w > 0 && r.h > 0);
        CHECK(r.x >= 0 && r.y >= 0 && r.x + r.w <= SCREEN_W && r.y + r.h <= SCREEN_H);
        CHECK(!rect_overlaps(r, layout_table_area()));
        CHECK(!rect_overlaps(r, score));
        CHECK(!rect_overlaps(r, layout_message()));
        for (int i = 0; i < SLEEPING_COUNT; i++)
            CHECK(!rect_overlaps(r, sleeping[i]));
        for (int j = 0; j < 3; j++)
            if (j != k)
                CHECK(!rect_overlaps(r, others[j]));
        for (int s = 0; s < SEAT_COUNT; s++)
            for (int count = 1; count <= HAND_SIZE; count++) {
                RectF row[HAND_SIZE];
                if (s == SEAT_SOUTH)
                    layout_south_hand(count, row);
                else
                    layout_cpu_row((Seat)s, count, row);
                for (int i = 0; i < count; i++)
                    CHECK(!rect_overlaps(r, row[i]));
            }
    }
    RectF row[HAND_SIZE];
    layout_cpu_row(SEAT_NORTH, HAND_SIZE, row);
    RectF n = others[0];
    CHECK(n.y + n.h <= 40);
    CHECK(n.y + n.h <= row[0].y);
    CHECK(cx(n) == 640);
    const Seat sides[] = {SEAT_EAST, SEAT_WEST};
    for (int k = 0; k < 2; k++) {
        layout_cpu_row(sides[k], HAND_SIZE, row);
        RectF r = layout_bot_name(sides[k]);
        CHECK(r.y + r.h <= row[0].y);
        CHECK(cx(r) == cx(row[0]));
    }
    return 0;
}

/* Vez do `who`, que não tem jogada; o seguinte tem jogada (para não trocar a mensagem). */
static void no_move_turn(Controller *c, Seat who)
{
    Game *g = &c->game;
    set_board_ends(g, 1, 1);
    for (int s = 0; s < SEAT_COUNT; s++) {
        Tile with[] = {T(1, 2), T(3, 3), T(4, 5)};
        Tile without[] = {T(2, 3), T(4, 4), T(5, 6)};
        set_hand(g, (Seat)s, (Seat)s == who ? without : with, 3);
    }
    memset(g->lacks, 0, sizeof g->lacks);
    g->turn = who;
    c->turn_ms = 0;
    c->message_ms = 0;
    c->anim = ANIM_NONE;
}

static int bot_pass_message(void)
{
    Controller c;
    uint64_t seed = 0;
    for (;; seed++) {
        ctl_init(&c, seed);
        ctl_set_bots(&c, 0, seed);
        if (c.bot[SEAT_EAST] == 3)
            break;
        CHECK(seed < 1000);
    }
    no_move_turn(&c, SEAT_EAST);
    View v;
    ctl_update(&c, 800);
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Seu Leo passou") == 0);
    ctl_update(&c, 1499);
    ctl_view(&c, &v);
    CHECK(v.message_visible);
    CHECK(strcmp(v.message, "Seu Leo passou") == 0);
    ctl_update(&c, 1);
    ctl_view(&c, &v);
    CHECK(!v.message_visible);
    return 0;
}

static int no_bots_keeps_messages(void)
{
    const Seat seats[] = {SEAT_EAST, SEAT_NORTH, SEAT_WEST};
    const char *texts[] = {"Leste passou", "Parceiro passou", "Oeste passou"};
    for (int k = 0; k < 3; k++) {
        Controller c;
        ctl_init(&c, 5);
        no_move_turn(&c, seats[k]);
        View v;
        ctl_update(&c, 800);
        ctl_view(&c, &v);
        CHECK(v.message_visible);
        CHECK(strcmp(v.message, texts[k]) == 0);
        for (int s = 0; s < SEAT_COUNT; s++)
            CHECK(v.seat_name[s][0] == '\0');
    }
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"bots_table", bots_table},
        {"style_seu_paulo_always_partner", style_seu_paulo_always_partner},
        {"style_neca_partner_when_not_more", style_neca_partner_when_not_more},
        {"style_audi_is_current_rule", style_audi_is_current_rule},
        {"style_seu_leo_blocks_first", style_seu_leo_blocks_first},
        {"style_guede_never_partner", style_guede_never_partner},
        {"style_manezinho_tiebreak_only", style_manezinho_tiebreak_only},
        {"styles_decide_from_view_only", styles_decide_from_view_only},
        {"menu_partner_default_audi", menu_partner_default_audi},
        {"menu_card_click_selects", menu_card_click_selects},
        {"menu_partner_layout", menu_partner_layout},
        {"bots_seating_by_seed", bots_seating_by_seed},
        {"new_match_keeps_partner_redraws", new_match_keeps_partner_redraws},
        {"bot_names_on_table", bot_names_on_table},
        {"bot_pass_message", bot_pass_message},
        {"no_bots_keeps_messages", no_bots_keeps_messages},
    };
    return RUN_TESTS(cases);
}
