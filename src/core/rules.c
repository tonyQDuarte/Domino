#include <string.h>

#include "domino.h"

bool tile_is_double(Tile t) { return t.a == t.b; }

int tile_sum(Tile t) { return t.a + t.b; }

Team team_of(Seat s) { return (s == SEAT_SOUTH || s == SEAT_NORTH) ? TEAM_US : TEAM_THEM; }

Seat next_seat(Seat s) { return (Seat)((s + 1) % SEAT_COUNT); }

/* splitmix64: sequência igual em qualquer plataforma para a mesma seed. */
static uint64_t rng_next(uint64_t *state)
{
    uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static void sort_hand(Hand *h)
{
    for (int i = 1; i < h->count; i++) {
        Tile t = h->tiles[i];
        int j = i - 1;
        while (j >= 0 && tile_sum(h->tiles[j]) > tile_sum(t)) {
            h->tiles[j + 1] = h->tiles[j];
            j--;
        }
        h->tiles[j + 1] = t;
    }
}

static Seat holder_of(const Game *g, int a, int b)
{
    for (int s = 0; s < SEAT_COUNT; s++)
        for (int i = 0; i < g->hands[s].count; i++)
            if (g->hands[s].tiles[i].a == a && g->hands[s].tiles[i].b == b)
                return (Seat)s;
    return SEAT_COUNT;
}

static void start_hand(Game *g)
{
    Tile set[TILE_COUNT];
    int n = 0;
    for (int a = 0; a <= 6; a++)
        for (int b = a; b <= 6; b++)
            set[n++] = (Tile){(uint8_t)a, (uint8_t)b};
    for (int i = TILE_COUNT - 1; i > 0; i--) {
        int j = (int)(rng_next(&g->rng) % (uint64_t)(i + 1));
        Tile t = set[i];
        set[i] = set[j];
        set[j] = t;
    }
    for (int s = 0; s < SEAT_COUNT; s++) {
        g->hands[s].count = HAND_SIZE;
        for (int i = 0; i < HAND_SIZE; i++)
            g->hands[s].tiles[i] = set[s * HAND_SIZE + i];
        sort_hand(&g->hands[s]);
    }
    for (int i = 0; i < SLEEPING_COUNT; i++)
        g->sleeping[i] = set[SEAT_COUNT * HAND_SIZE + i];
    memset(&g->board, 0, sizeof g->board);
    if (g->first_hand) {
        /* abre a maior carroça distribuída: o [6|6] pode ter ficado de fora */
        int d = 6;
        while (d > 0 && holder_of(g, d, d) == SEAT_COUNT)
            d--;
        g->opening_double = (uint8_t)d;
        g->turn = holder_of(g, d, d);
    } else {
        g->turn = game_choose_opener(g, g->next_opener);
    }
    g->opener = g->turn;
    g->passes = 0;
    g->phase = PHASE_PLAYING;
    g->result = RESULT_NONE;
    g->result_points = 0;
    g->result_team = -1;
    memset(g->lacks, 0, sizeof g->lacks);
}

static bool has_double(const Hand *h)
{
    for (int i = 0; i < h->count; i++)
        if (tile_is_double(h->tiles[i]))
            return true;
    return false;
}

Seat game_choose_opener(const Game *g, Seat lead)
{
    Seat order[4] = {lead, (Seat)((lead + 2) % SEAT_COUNT), next_seat(lead),
                     (Seat)((lead + 3) % SEAT_COUNT)};
    for (int k = 0; k < 4; k++)
        if (has_double(&g->hands[order[k]]))
            return order[k];
    return lead; /* não acontece: as 7 carroças estão sempre nas mãos */
}

void game_restart_match(Game *g)
{
    g->score[TEAM_US] = g->score[TEAM_THEM] = 0;
    g->first_hand = true;
    start_hand(g);
}

void game_new_match(Game *g, uint64_t seed)
{
    memset(g, 0, sizeof *g);
    g->rng = seed;
    game_restart_match(g);
}

void game_next_hand(Game *g)
{
    g->first_hand = false;
    start_hand(g);
}

void game_fits(const Game *g, Tile t, bool *left, bool *right)
{
    const Board *b = &g->board;
    if (b->count == 0) {
        *left = g->first_hand ? (t.a == g->opening_double && t.b == g->opening_double)
                              : tile_is_double(t);
        *right = false;
        return;
    }
    int l = b->line[0].left, r = b->line[b->count - 1].right;
    *left = t.a == l || t.b == l;
    *right = t.a == r || t.b == r;
}

bool game_has_move(const Game *g, Seat s)
{
    for (int i = 0; i < g->hands[s].count; i++) {
        bool l, r;
        game_fits(g, g->hands[s].tiles[i], &l, &r);
        if (l || r)
            return true;
    }
    return false;
}

static void finish_hand(Game *g, HandResult result, int team, int points)
{
    g->result = result;
    g->result_team = team;
    g->result_points = points;
    if (team >= 0)
        g->score[team] += points;
    g->phase = (g->score[TEAM_US] >= WIN_SCORE || g->score[TEAM_THEM] >= WIN_SCORE)
                   ? PHASE_MATCH_OVER
                   : PHASE_HAND_OVER;
}

bool game_play(Game *g, int index, End end)
{
    if (g->phase != PHASE_PLAYING)
        return false;
    Hand *h = &g->hands[g->turn];
    if (index < 0 || index >= h->count)
        return false;
    Tile t = h->tiles[index];
    bool fits_left, fits_right;
    game_fits(g, t, &fits_left, &fits_right);
    Board *b = &g->board;

    if (b->count == 0) {
        if (!fits_left)
            return false;
        b->line[0] = (PlacedTile){t.a, t.b};
        b->count = 1;
        b->anchor = 0;
    } else if (end == END_LEFT) {
        if (!fits_left)
            return false;
        uint8_t v = b->line[0].left;
        memmove(&b->line[1], &b->line[0], sizeof(PlacedTile) * (size_t)b->count);
        b->line[0] = t.b == v ? (PlacedTile){t.a, t.b} : (PlacedTile){t.b, t.a};
        b->count++;
        b->anchor++;
    } else {
        if (!fits_right)
            return false;
        uint8_t v = b->line[b->count - 1].right;
        b->line[b->count] = t.a == v ? (PlacedTile){t.a, t.b} : (PlacedTile){t.b, t.a};
        b->count++;
    }

    for (int i = index; i < h->count - 1; i++)
        h->tiles[i] = h->tiles[i + 1];
    h->count--;
    g->passes = 0;

    if (h->count == 0) {
        bool dbl = tile_is_double(t), both = fits_left && fits_right;
        HandResult r = dbl ? (both ? RESULT_CRUZADA : RESULT_CARROCA)
                           : (both ? RESULT_LA_E_LO : RESULT_SIMPLES);
        int points = r == RESULT_CRUZADA ? 4 : r == RESULT_LA_E_LO ? 3 : r == RESULT_CARROCA ? 2 : 1;
        g->result_seat = g->turn;
        g->next_opener = g->turn;
        finish_hand(g, r, team_of(g->turn), points);
        return true;
    }
    g->turn = next_seat(g->turn);
    return true;
}

static int hand_points(const Hand *h)
{
    int sum = 0;
    for (int i = 0; i < h->count; i++)
        sum += tile_sum(h->tiles[i]);
    return sum;
}

void game_pass(Game *g)
{
    if (g->phase != PHASE_PLAYING)
        return;
    if (g->board.count > 0) {
        g->lacks[g->turn][g->board.line[0].left] = true;
        g->lacks[g->turn][g->board.line[g->board.count - 1].right] = true;
    }
    g->passes++;
    if (g->passes >= SEAT_COUNT) {
        int us = hand_points(&g->hands[SEAT_SOUTH]) + hand_points(&g->hands[SEAT_NORTH]);
        int them = hand_points(&g->hands[SEAT_EAST]) + hand_points(&g->hands[SEAT_WEST]);
        int team = us < them ? TEAM_US : them < us ? TEAM_THEM : -1;
        /* a procura começa na dupla que ganhou o ponto; no empate, em quem abriu */
        g->next_opener = (team < 0 || (int)team_of(g->opener) == team) ? g->opener
                                                                         : next_seat(g->opener);
        finish_hand(g, RESULT_TRANCADO, team, team >= 0 ? 1 : 0);
        return;
    }
    g->turn = next_seat(g->turn);
}
