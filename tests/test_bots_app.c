/* app_run com a tela inicial: a escolha do parceiro chega à partida (bots-parceiros). */
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "app.h"
#include "bots.h"
#include "harness.h"
#include "namefile.h"

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
        menu_press(m, r.x + r.w / 2, r.y + r.h / 2);
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

/* Sem menu_input: o teclado e o mouse passam pelo caminho real da app_run; o gancho mouse_input
   só diz onde foi o clique. Quadro 0: cartão de Seu Paulo; quadro 1: Jogar. */
static bool mouse(int frame, float *x, float *y, void *user)
{
    (void)user;
    RectF r = frame == 0 ? menu_card_rect(0) : menu_play_rect();
    *x = r.x + r.w / 2;
    *y = r.y + r.h / 2;
    return frame <= 1;
}

static int app_menu_card_click_real_path(void)
{
    CHECK(name_save("tmp_bots_app/jogador.txt", "Ana"));
    Script s = {0};
    AppOptions opt = {0};
    opt.seed = 5;
    opt.max_frames = 4;
    opt.show_menu = true;
    opt.name_path = "tmp_bots_app/jogador.txt";
    opt.mouse_input = mouse;
    opt.on_game_start = started;
    opt.user = &s;
    CHECK(app_run(&opt) == 0);
    CHECK(s.started);
    CHECK(s.bot[SEAT_NORTH] == 0);
    CHECK(s.styles[SEAT_NORTH] == STYLE_SEU_PAULO);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"app_menu_partner_starts_game", app_menu_partner_starts_game},
        {"app_menu_card_click_real_path", app_menu_card_click_real_path},
    };
    return RUN_TESTS(cases);
}
