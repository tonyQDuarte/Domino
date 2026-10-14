/* app_run com a tela inicial ligada; o gancho menu_input faz o papel do teclado e do mouse. */
#include <direct.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "app.h"
#include "harness.h"
#include "namefile.h"

typedef struct {
    const char *type;    /* texto digitado no quadro 0 */
    int act_frame;       /* quadro em que a ação é devolvida */
    MenuAction action;
    char seen_name[NAME_BUF]; /* nome no campo no quadro 0 */
    int menu_frames;
    bool started;
    bool animate, dealing;
    char player[NAME_BUF];
    int frames;
} Script;

static MenuAction input(int frame, Menu *m, void *user)
{
    Script *s = user;
    s->menu_frames++;
    if (frame == 0) {
        snprintf(s->seen_name, sizeof s->seen_name, "%s", m->name);
        if (s->type)
            for (const char *p = s->type; *p; p++)
                menu_type(m, (unsigned char)*p);
    }
    return frame == s->act_frame ? s->action : MENU_NONE;
}

static void started(const Controller *c, void *user)
{
    Script *s = user;
    s->started = true;
    s->animate = c->animate;
    s->dealing = c->anim == ANIM_DEAL;
    snprintf(s->player, sizeof s->player, "%s", c->player_name);
}

static void counted(int frame, void *user)
{
    (void)frame;
    ((Script *)user)->frames++;
}

static int run(Script *s, const char *path, int max_frames)
{
    AppOptions opt = {0};
    opt.seed = 1;
    opt.max_frames = max_frames;
    opt.show_menu = true;
    opt.name_path = path;
    opt.menu_input = input;
    opt.on_game_start = started;
    opt.on_frame = counted;
    opt.user = s;
    return app_run(&opt);
}

static int app_menu_before_game(void)
{
    Script s = {0};
    s.act_frame = -1;
    CHECK(run(&s, NULL, 3) == 0);
    CHECK(s.menu_frames == 3);
    CHECK(!s.started);
    return 0;
}

static int app_menu_play_starts_game(void)
{
    Script s = {0};
    s.type = "  Ana  ";
    s.act_frame = 1;
    s.action = MENU_PLAY;
    CHECK(run(&s, NULL, 4) == 0);
    CHECK(s.started);
    CHECK(s.animate);
    CHECK(s.dealing);
    CHECK(strcmp(s.player, "Ana") == 0);
    return 0;
}

static int app_menu_quit_returns_0(void)
{
    Script s = {0};
    s.type = "Ana";
    s.act_frame = 1;
    s.action = MENU_QUIT;
    CHECK(run(&s, NULL, 100) == 0);
    CHECK(!s.started);
    CHECK(s.menu_frames == 2);
    CHECK(s.frames <= 2);
    return 0;
}

static int app_menu_saves_name(void)
{
    _mkdir("tmp_menu_app");
    remove("tmp_menu_app/salva/jogador.txt");
    _rmdir("tmp_menu_app/salva");
    Script s = {0};
    s.type = "  Ana  ";
    s.act_frame = 1;
    s.action = MENU_PLAY;
    CHECK(run(&s, "tmp_menu_app/salva/jogador.txt", 3) == 0);
    CHECK(s.started);
    FILE *f = fopen("tmp_menu_app/salva/jogador.txt", "rb");
    CHECK(f != NULL);
    char buf[16] = {0};
    size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    CHECK(n == 3);
    CHECK(memcmp(buf, "Ana", 3) == 0);
    return 0;
}

static int app_menu_prefills_name(void)
{
    _mkdir("tmp_menu_app");
    FILE *f = fopen("tmp_menu_app/ze.txt", "wb");
    fwrite("Z\xC3\xA9", 1, 3, f);
    fclose(f);
    Script s = {0};
    s.act_frame = -1;
    CHECK(run(&s, "tmp_menu_app/ze.txt", 2) == 0);
    CHECK(strcmp(s.seen_name, "Zé") == 0);
    return 0;
}

static int app_menu_save_failure_still_plays(void)
{
    _mkdir("tmp_menu_app");
    FILE *f = fopen("tmp_menu_app/arquivo", "wb"); /* um arquivo no lugar da pasta */
    fputs("x", f);
    fclose(f);
    CHECK(!name_save("tmp_menu_app/arquivo/jogador.txt", "Ana"));
    Script s = {0};
    s.type = "Ana";
    s.act_frame = 1;
    s.action = MENU_PLAY;
    CHECK(run(&s, "tmp_menu_app/arquivo/jogador.txt", 4) == 0);
    CHECK(s.started);
    CHECK(strcmp(s.player, "Ana") == 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"app_menu_before_game", app_menu_before_game},
        {"app_menu_play_starts_game", app_menu_play_starts_game},
        {"app_menu_quit_returns_0", app_menu_quit_returns_0},
        {"app_menu_saves_name", app_menu_saves_name},
        {"app_menu_prefills_name", app_menu_prefills_name},
        {"app_menu_save_failure_still_plays", app_menu_save_failure_still_plays},
    };
    SetTraceLogLevel(LOG_WARNING);
    return RUN_TESTS(cases);
}
