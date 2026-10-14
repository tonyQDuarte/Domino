/* Testes da feature tela-inicial: menu e arquivo do nome (sem raylib). */
#include <direct.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "controller.h"
#include "harness.h"
#include "layout.h"
#include "menu.h"
#include "namefile.h"

static void type_str(Menu *m, const char *ascii)
{
    for (const char *p = ascii; *p; p++)
        menu_type(m, (unsigned char)*p);
}

static void click_center(const Menu *m, RectF r, MenuAction *out)
{
    *out = menu_click(m, r.x + r.w / 2, r.y + r.h / 2);
}

static void write_bytes(const char *path, const char *bytes, size_t n)
{
    FILE *f = fopen(path, "wb");
    if (n)
        fwrite(bytes, 1, n, f);
    fclose(f);
}

static size_t read_bytes(const char *path, char *buf, size_t size)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return (size_t)-1;
    size_t n = fread(buf, 1, size, f);
    fclose(f);
    return n;
}

/* ---------- S1 ---------- */

static int menu_layout_and_texts(void)
{
    CHECK(strcmp(MENU_TITLE, "Dominó") == 0);
    CHECK(strcmp(MENU_LABEL, "Seu nome") == 0);
    CHECK(strcmp(MENU_PLAY_TEXT, "Jogar") == 0);
    CHECK(strcmp(MENU_QUIT_TEXT, "Sair") == 0);
    RectF r[5] = {menu_title_rect(), menu_label_rect(), menu_field_rect(), menu_play_rect(),
                  menu_quit_rect()};
    for (int i = 0; i < 5; i++) {
        CHECK(r[i].w > 0 && r[i].h > 0);
        CHECK(r[i].x >= 0 && r[i].y >= 0 && r[i].x + r[i].w <= SCREEN_W && r[i].y + r[i].h <= SCREEN_H);
        if (i > 0)
            CHECK(r[i].y >= r[i - 1].y + r[i - 1].h); /* de cima para baixo, sem sobrepor */
        for (int j = i + 1; j < 5; j++)
            CHECK(!rect_overlaps(r[i], r[j]));
    }
    return 0;
}

static int menu_type_filter(void)
{
    const int ok[] = {32, 65, 126, 160, 231, 255};
    const int bad[] = {31, 127, 159, 256, 128512};
    for (int i = 0; i < 6; i++) {
        Menu m;
        menu_init(&m, NULL);
        CHECK(menu_type(&m, ok[i]));
        CHECK(m.chars == 1);
    }
    for (int i = 0; i < 5; i++) {
        Menu m;
        menu_init(&m, NULL);
        CHECK(!menu_type(&m, bad[i]));
        CHECK(m.chars == 0);
        CHECK(m.name[0] == '\0');
    }
    Menu m;
    menu_init(&m, NULL);
    CHECK(menu_type(&m, 231));
    CHECK((unsigned char)m.name[0] == 0xC3 && (unsigned char)m.name[1] == 0xA7 && m.name[2] == '\0');

    menu_init(&m, NULL);
    for (int i = 0; i < 16; i++)
        CHECK(menu_type(&m, 'a' + (i % 26)));
    CHECK(m.chars == 16);
    CHECK(!menu_type(&m, 'z'));
    CHECK(!menu_type(&m, 233));
    CHECK(m.chars == 16);
    CHECK(strlen(m.name) == 16);
    return 0;
}

static int menu_backspace_utf8(void)
{
    Menu m;
    menu_init(&m, NULL);
    menu_type(&m, 'Z');
    menu_type(&m, 233); /* é */
    CHECK(strcmp(m.name, "Zé") == 0);
    CHECK(m.chars == 2);
    menu_backspace(&m);
    CHECK(strcmp(m.name, "Z") == 0);
    CHECK(m.chars == 1);
    menu_backspace(&m);
    CHECK(m.name[0] == '\0' && m.chars == 0);
    menu_backspace(&m);
    CHECK(m.name[0] == '\0' && m.chars == 0);
    return 0;
}

static int menu_play_disabled_when_blank(void)
{
    const char *blanks[] = {"", "   "};
    for (int i = 0; i < 2; i++) {
        Menu m;
        menu_init(&m, NULL);
        type_str(&m, blanks[i]);
        CHECK(!menu_play_enabled(&m));
        MenuAction a;
        click_center(&m, menu_play_rect(), &a);
        CHECK(a == MENU_NONE);
        CHECK(menu_enter(&m) == MENU_NONE);
    }
    return 0;
}

static int menu_play_trims_name(void)
{
    Menu m;
    menu_init(&m, NULL);
    type_str(&m, "  Ana  ");
    CHECK(menu_play_enabled(&m));
    MenuAction a;
    click_center(&m, menu_play_rect(), &a);
    CHECK(a == MENU_PLAY);
    CHECK(menu_enter(&m) == MENU_PLAY);
    char t[NAME_BUF];
    menu_trimmed_name(&m, t);
    CHECK(strcmp(t, "Ana") == 0);
    /* clique fora dos botões não faz nada */
    CHECK(menu_click(&m, 5, 5) == MENU_NONE);
    return 0;
}

static int menu_quit_click(void)
{
    for (int filled = 0; filled < 2; filled++) {
        Menu m;
        menu_init(&m, filled ? "Ana" : NULL);
        MenuAction a;
        click_center(&m, menu_quit_rect(), &a);
        CHECK(a == MENU_QUIT);
    }
    return 0;
}

static int player_name_above_south_hand(void)
{
    Controller c;
    ctl_init(&c, 1);
    View v;
    ctl_view(&c, &v);
    CHECK(v.player_name[0] == '\0');
    ctl_set_player_name(&c, "Ana");
    ctl_view(&c, &v);
    CHECK(strcmp(v.player_name, "Ana") == 0);

    RectF n = layout_player_name(), msg = layout_message();
    RectF hand[HAND_SIZE];
    layout_south_hand(HAND_SIZE, hand);
    CHECK(n.x + n.w / 2 > 639.5f && n.x + n.w / 2 < 640.5f);
    CHECK(n.y >= 560);
    CHECK(n.y + n.h <= hand[0].y);
    CHECK(hand[0].y == 605);
    CHECK(!rect_overlaps(n, msg));
    return 0;
}

/* ---------- S2 ---------- */

static int name_save_format(void)
{
    _mkdir("tmp_menu");
    remove("tmp_menu/nova/jogador.txt");
    _rmdir("tmp_menu/nova");
    CHECK(name_save("tmp_menu/nova/jogador.txt", "Zé"));
    char buf[16];
    size_t n = read_bytes("tmp_menu/nova/jogador.txt", buf, sizeof buf);
    CHECK(n == 3);
    CHECK((unsigned char)buf[0] == 0x5A && (unsigned char)buf[1] == 0xC3 && (unsigned char)buf[2] == 0xA9);
    return 0;
}

static int name_load_prefills(void)
{
    _mkdir("tmp_menu");
    write_bytes("tmp_menu/ze.txt", "Z\xC3\xA9", 3);
    char out[NAME_BUF];
    CHECK(name_load("tmp_menu/ze.txt", out));
    CHECK(strcmp(out, "Zé") == 0);
    Menu m;
    menu_init(&m, out);
    CHECK(strcmp(m.name, "Zé") == 0);
    CHECK(m.chars == 2);
    return 0;
}

static int name_load_rejects_invalid(void)
{
    _mkdir("tmp_menu");
    char out[NAME_BUF];
    remove("tmp_menu/ausente.txt");
    strcpy(out, "x");
    CHECK(!name_load("tmp_menu/ausente.txt", out));
    CHECK(out[0] == '\0');

    struct {
        const char *bytes;
        size_t n;
    } bad[] = {{"", 0}, {"Ana\n", 4}, {"A\tB", 3}, {"\x80", 1}, {"\xC4\x80", 2}};
    for (int i = 0; i < 5; i++) {
        write_bytes("tmp_menu/ruim.txt", bad[i].bytes, bad[i].n);
        strcpy(out, "x");
        if (name_load("tmp_menu/ruim.txt", out) || out[0] != '\0') {
            fprintf(stderr, "caso %d aceito\n", i);
            CHECK(false);
        }
    }

    write_bytes("tmp_menu/longo.txt", "abcdefghijklmnopqrst", 20);
    CHECK(name_load("tmp_menu/longo.txt", out));
    CHECK(strcmp(out, "abcdefghijklmnop") == 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"menu_layout_and_texts", menu_layout_and_texts},
        {"menu_type_filter", menu_type_filter},
        {"menu_backspace_utf8", menu_backspace_utf8},
        {"menu_play_disabled_when_blank", menu_play_disabled_when_blank},
        {"menu_play_trims_name", menu_play_trims_name},
        {"menu_quit_click", menu_quit_click},
        {"player_name_above_south_hand", player_name_above_south_hand},
        {"name_save_format", name_save_format},
        {"name_load_prefills", name_load_prefills},
        {"name_load_rejects_invalid", name_load_rejects_invalid},
    };
    return RUN_TESTS(cases);
}
