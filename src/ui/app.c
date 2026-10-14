#include <math.h>
#include <stdio.h>

#include "raylib.h"

#include "app.h"
#include "draw.h"
#include "namefile.h"

/* Fonte do sistema com acentos; sem ela, a fonte padrão da raylib. */
static Font load_font(void)
{
    static const char *paths[] = {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"};
    int codepoints[224];
    for (int i = 0; i < 224; i++)
        codepoints[i] = 32 + i;
    for (int i = 0; i < 2; i++)
        if (FileExists(paths[i])) {
            Font f = LoadFontEx(paths[i], 48, codepoints, 224);
            if (f.texture.id != 0) {
                SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
                return f;
            }
        }
    return GetFontDefault();
}

int app_run(const AppOptions *opt)
{
    if (!opt->force_window_fail) {
        SetTraceLogLevel(LOG_WARNING);
        InitWindow(SCREEN_W, SCREEN_H, "Dominó");
    }
    if (opt->force_window_fail || !IsWindowReady()) {
        fprintf(stderr, "erro: não foi possível abrir a janela\n");
        return 1;
    }
    SetTargetFPS(60);
    Font font = load_font();

    Menu menu;
    char saved[NAME_BUF] = "";
    if (opt->show_menu && opt->name_path)
        name_load(opt->name_path, saved);
    menu_init(&menu, saved);

    Controller c;
    bool in_menu = opt->show_menu;
    bool just_started = false;
    if (!in_menu) {
        ctl_init(&c, opt->seed);
        ctl_set_animations(&c, true);
        just_started = true;
    }
    for (int frame = 0; !WindowShouldClose(); frame++) {
        if (opt->max_frames > 0 && frame >= opt->max_frames)
            break;

        if (in_menu) {
            MenuAction action = MENU_NONE;
            if (opt->menu_input) {
                action = opt->menu_input(frame, &menu, opt->user);
            } else {
                for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
                    menu_type(&menu, ch);
                if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))
                    menu_backspace(&menu);
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
                    action = menu_enter(&menu);
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 m = GetMousePosition();
                    menu_pick_partner(&menu, m.x, m.y);
                    MenuAction clicked = menu_click(&menu, m.x, m.y);
                    if (clicked != MENU_NONE)
                        action = clicked;
                }
            }
            if (action == MENU_QUIT)
                break;
            if (action == MENU_PLAY) {
                char name[NAME_BUF];
                menu_trimmed_name(&menu, name);
                if (opt->name_path)
                    name_save(opt->name_path, name); /* se falhar, joga mesmo assim */
                ctl_init(&c, opt->seed);
                ctl_set_player_name(&c, name);
                ctl_set_bots(&c, menu.partner, opt->seed);
                ctl_set_animations(&c, true);
                in_menu = false;
                just_started = true;
            }
        }

        if (!in_menu) {
            if (just_started) {
                if (opt->on_game_start)
                    opt->on_game_start(&c, opt->user);
            } else {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 m = GetMousePosition();
                    ctl_click(&c, m.x, m.y);
                }
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
                    ctl_enter(&c);
            }
            ctl_update(&c, just_started ? 0 : GetFrameTime() * 1000.0);
            just_started = false;

            if (frame == 0 && opt->on_state)
                opt->on_state(&c, opt->user);
        }

        BeginDrawing();
        if (in_menu)
            draw_menu(&menu, font, fmod(GetTime(), 1.0) < 0.5);
        else
            draw_frame(&c, font);
        if (opt->on_frame)
            opt->on_frame(frame, opt->user);
        EndDrawing();
    }
    if (font.texture.id != GetFontDefault().texture.id)
        UnloadFont(font);
    CloseWindow();
    return 0;
}
