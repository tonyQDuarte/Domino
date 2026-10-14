/* app_run com a tela inicial: a escolha do parceiro chega à partida (bots-parceiros). */
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "app.h"
#include "bots.h"
#include "harness.h"

typedef struct {
    bool started;
    int bot[SEAT_COUNT];
    CpuStyle styles[SEAT_COUNT];
} Script;

/* Quadro 0: digita o nome e clica no cartão de Seu Paulo; quadro 1: Jogar. */
static MenuAction input(int frame, Menu *m, void *user)
{
    (void)user;
    if (frame == 0) {
        for (const char *p = "Ana"; *p; p++)
            menu_type(m, (unsigned char)*p);
        RectF r = menu_card_rect(0);
        menu_pick_partner(m, r.x + r.w / 2, r.y + r.h / 2);
    }
    return frame == 1 ? MENU_PLAY : MENU_NONE;
}

static void started(const Controller *c, void *user)
{
    Script *s = user;
    s->started = true;
    for (int i = 0; i < SEAT_COUNT; i++) {
        s->bot[i] = c->bot[i];
        s->styles[i] = c->game.styles[i];
    }
}

static int run(Script *s, uint64_t seed)
{
    AppOptions opt = {0};
    opt.seed = seed;
    opt.max_frames = 4;
    opt.show_menu = true;
    opt.menu_input = input;
    opt.on_game_start = started;
    opt.user = s;
    return app_run(&opt);
}

static int app_menu_partner_starts_game(void)
{
    Script a = {0}, b = {0};
    CHECK(run(&a, 5) == 0);
    CHECK(run(&b, 5) == 0);
    CHECK(a.started && b.started);
    CHECK(strcmp(BOTS[a.bot[SEAT_NORTH]].name, "Seu Paulo") == 0);
    int e = a.bot[SEAT_EAST], w = a.bot[SEAT_WEST];
    CHECK(e >= 0 && e < BOT_COUNT && w >= 0 && w < BOT_COUNT);
    CHECK(e != w && e != 0 && w != 0);
    CHECK(a.styles[SEAT_NORTH] == STYLE_SEU_PAULO);
    CHECK(a.styles[SEAT_EAST] == BOTS[e].style && a.styles[SEAT_WEST] == BOTS[w].style);
    CHECK(b.bot[SEAT_NORTH] == 0 && b.bot[SEAT_EAST] == e && b.bot[SEAT_WEST] == w);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"app_menu_partner_starts_game", app_menu_partner_starts_game},
    };
    return RUN_TESTS(cases);
}
