#include <string.h>

#include "domino.h"

typedef struct {
    int index;
    End end;
    uint8_t left, right; /* pontas depois da jogada */
} Move;

CpuView cpu_view_of(const Game *g, Seat s)
{
    CpuView v;
    memset(&v, 0, sizeof v);
    v.own = g->hands[s];
    v.seat = s;
    for (int i = 0; i < SEAT_COUNT; i++)
        v.counts[i] = g->hands[i].count;
    memcpy(v.lacks, g->lacks, sizeof v.lacks);
    v.empty = g->board.count == 0;
    if (!v.empty) {
        v.left = g->board.line[0].left;
        v.right = g->board.line[g->board.count - 1].right;
    }
    v.first_hand = g->first_hand;
    return v;
}

static int list_moves(const CpuView *v, Move out[HAND_SIZE * 2])
{
    int n = 0;
    for (int i = 0; i < v->own.count; i++) {
        Tile t = v->own.tiles[i];
        if (v->empty) {
            if (!v->first_hand || (t.a == 6 && t.b == 6))
                out[n++] = (Move){i, END_LEFT, t.a, t.b};
            continue;
        }
        if (t.a == v->left || t.b == v->left)
            out[n++] = (Move){i, END_LEFT, t.a == v->left ? t.b : t.a, v->right};
        if (t.a == v->right || t.b == v->right)
            out[n++] = (Move){i, END_RIGHT, v->left, t.a == v->right ? t.b : t.a};
    }
    return n;
}

/* Mantém só os movimentos em que keep() é verdadeiro, se sobrar algum. */
static int filter(Move *m, int n, bool (*keep)(const CpuView *, Seat, const Move *), const CpuView *v,
                  Seat who)
{
    int k = 0;
    Move kept[HAND_SIZE * 2];
    for (int i = 0; i < n; i++)
        if (keep(v, who, &m[i]))
            kept[k++] = m[i];
    if (k == 0)
        return n;
    memcpy(m, kept, sizeof(Move) * (size_t)k);
    return k;
}

static bool not_blocking(const CpuView *v, Seat who, const Move *m)
{
    return !(v->lacks[who][m->left] && v->lacks[who][m->right]);
}

static bool forces_pass(const CpuView *v, Seat who, const Move *m)
{
    return v->lacks[who][m->left] && v->lacks[who][m->right];
}

static int own_fits_after(const CpuView *v, const Move *m)
{
    int n = 0;
    for (int i = 0; i < v->own.count; i++) {
        Tile t = v->own.tiles[i];
        if (i != m->index && (t.a == m->left || t.b == m->left || t.a == m->right || t.b == m->right))
            n++;
    }
    return n;
}

bool cpu_decide(const CpuView *v, int *index, End *end)
{
    Move m[HAND_SIZE * 2];
    int n = list_moves(v, m);
    if (n == 0)
        return false;

    Seat partner = (Seat)((v->seat + 2) % SEAT_COUNT);
    Seat next = next_seat(v->seat);
    bool partner_mode = v->counts[partner] < v->own.count;

    if (partner_mode)
        n = filter(m, n, not_blocking, v, partner);
    n = filter(m, n, forces_pass, v, next);
    if (!partner_mode) {
        int best = -1, k = 0;
        for (int i = 0; i < n; i++)
            if (own_fits_after(v, &m[i]) > best)
                best = own_fits_after(v, &m[i]);
        for (int i = 0; i < n; i++)
            if (own_fits_after(v, &m[i]) == best)
                m[k++] = m[i];
        n = k;
    }

    /* desempate: maior soma, carroça, primeira da mão, ponta esquerda (m já vem nessa ordem) */
    int best = 0;
    for (int i = 1; i < n; i++) {
        Tile t = v->own.tiles[m[i].index], bt = v->own.tiles[m[best].index];
        int ds = tile_sum(t) - tile_sum(bt);
        if (ds > 0 || (ds == 0 && tile_is_double(t) && !tile_is_double(bt)))
            best = i;
    }
    *index = m[best].index;
    *end = m[best].end;
    return true;
}

bool cpu_choose(const Game *g, int *index, End *end)
{
    CpuView v = cpu_view_of(g, g->turn);
    return cpu_decide(&v, index, end);
}
