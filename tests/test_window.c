/* Roda a app_run em processo: cor da mesa e falha ao abrir a janela. */
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "app.h"
#include "harness.h"

static Color sampled;
static int frames_seen;

static void sample(int frame, void *user)
{
    (void)user;
    if (frame == 0) {
        Image img = LoadImageFromScreen();
        sampled = GetImageColor(img, 640, 360);
        UnloadImage(img);
    }
    frames_seen++;
}

static int window_table_color(void)
{
    AppOptions opt = {0};
    opt.seed = 1;
    opt.max_frames = 2;
    opt.on_frame = sample;
    CHECK(app_run(&opt) == 0);
    CHECK(frames_seen >= 1);
    if (sampled.r != 20 || sampled.g != 100 || sampled.b != 50) {
        fprintf(stderr, "cor lida: %d,%d,%d\n", sampled.r, sampled.g, sampled.b);
        CHECK(false);
    }
    return 0;
}

static int window_fail_exits_1(void)
{
    char path[] = "window_fail_stderr.txt";
    fflush(stderr);
    FILE *f = freopen(path, "w", stderr);
    CHECK(f != NULL);
    AppOptions opt = {0};
    opt.seed = 1;
    opt.force_window_fail = true;
    int code = app_run(&opt);
    fflush(stderr);
    fclose(stderr);
    FILE *r = fopen(path, "r");
    char buf[256] = {0};
    if (r) {
        fread(buf, 1, sizeof buf - 1, r);
        fclose(r);
    }
    remove(path);
    if (code != 1 || strstr(buf, "erro: não foi possível abrir a janela") == NULL) {
        printf("código %d, stderr '%s'\n", code, buf);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"window_table_color", window_table_color},
        {"window_fail_exits_1", window_fail_exits_1},
    };
    SetTraceLogLevel(LOG_WARNING);
    return RUN_TESTS(cases);
}
