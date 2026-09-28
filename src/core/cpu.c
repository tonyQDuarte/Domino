#include "domino.h"

bool cpu_choose(const Game *g, int *index, End *end)
{
    const Hand *h = &g->hands[g->turn];
    int best = -1;
    bool best_left = false;
    for (int i = 0; i < h->count; i++) {
        Tile t = h->tiles[i];
        bool l, r;
        game_fits(g, t, &l, &r);
        if (!l && !r)
            continue;
        if (best >= 0) {
            Tile bt = h->tiles[best];
            int ds = tile_sum(t) - tile_sum(bt);
            if (ds < 0 || (ds == 0 && (!tile_is_double(t) || tile_is_double(bt))))
                continue;
        }
        best = i;
        best_left = l;
    }
    if (best < 0)
        return false;
    *index = best;
    *end = best_left ? END_LEFT : END_RIGHT;
    return true;
}
