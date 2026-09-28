#include <stdio.h>

#include "raylib.h"

#include "app.h"
#include "draw.h"

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

    Controller c;
    ctl_init(&c, opt->seed);
    for (int frame = 0; !WindowShouldClose(); frame++) {
        if (opt->max_frames > 0 && frame >= opt->max_frames)
            break;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            ctl_click(&c, m.x, m.y);
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
            ctl_enter(&c);
        ctl_update(&c, frame == 0 ? 0 : GetFrameTime() * 1000.0);

        BeginDrawing();
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
