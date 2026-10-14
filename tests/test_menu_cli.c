/* O domino.exe real (caminho em argv[2]) abre a tela inicial: digita, Enter, grava o nome. */
#include <windows.h>

#include <direct.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"

static const char *exe_path;

typedef struct {
    DWORD pid;
    HWND found;
} FindCtx;

static BOOL CALLBACK find_proc(HWND w, LPARAM lp)
{
    FindCtx *f = (FindCtx *)lp;
    DWORD pid;
    GetWindowThreadProcessId(w, &pid);
    if (pid == f->pid && IsWindowVisible(w)) {
        f->found = w;
        return FALSE;
    }
    return TRUE;
}

static HWND wait_window(DWORD pid)
{
    for (int i = 0; i < 200; i++) {
        FindCtx f = {pid, NULL};
        EnumWindows(find_proc, (LPARAM)&f);
        if (f.found)
            return f.found;
        Sleep(50);
    }
    return NULL;
}

static int cli_menu_types_and_saves(void)
{
    char appdata[MAX_PATH];
    GetFullPathNameA("tmp_menu_cli", MAX_PATH, appdata, NULL);
    _mkdir(appdata);
    char file[MAX_PATH + 32];
    snprintf(file, sizeof file, "%s\\Domino\\jogador.txt", appdata);
    remove(file);
    SetEnvironmentVariableA("APPDATA", appdata); /* o filho herda */

    char cmd[1024];
    snprintf(cmd, sizeof cmd, "\"%s\" --seed 1", exe_path);
    STARTUPINFOA si;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    PROCESS_INFORMATION pi;
    CHECK(CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi));
    HWND w = wait_window(pi.dwProcessId);
    CHECK(w != NULL);
    Sleep(300);
    const char *text = "Ana";
    for (const char *p = text; *p; p++) {
        PostMessageW(w, WM_CHAR, (WPARAM)*p, 1);
        Sleep(60);
    }
    LPARAM down = 1 | (0x1C << 16), up = down | (1u << 30) | (1u << 31);
    PostMessageW(w, WM_KEYDOWN, VK_RETURN, down);
    Sleep(200);
    PostMessageW(w, WM_KEYUP, VK_RETURN, up);
    Sleep(500);
    PostMessageW(w, WM_CLOSE, 0, 0);
    DWORD code = 0xFFFFFFFF;
    if (WaitForSingleObject(pi.hProcess, 10000) != WAIT_OBJECT_0)
        TerminateProcess(pi.hProcess, 99);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CHECK(code == 0);

    FILE *f = fopen(file, "rb");
    if (!f) {
        fprintf(stderr, "arquivo não gravado: %s\n", file);
        CHECK(false);
    }
    char buf[16] = {0};
    size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    CHECK(n == 3);
    CHECK(memcmp(buf, "Ana", 3) == 0);
    return 0;
}

/* Abre o domino.exe com APPDATA em `dir` e devolve a janela. */
static HWND launch(const char *dir, PROCESS_INFORMATION *pi)
{
    char appdata[MAX_PATH];
    GetFullPathNameA(dir, MAX_PATH, appdata, NULL);
    _mkdir(appdata);
    SetEnvironmentVariableA("APPDATA", appdata);
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "\"%s\" --seed 1", exe_path);
    STARTUPINFOA si;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi))
        return NULL;
    HWND w = wait_window(pi->dwProcessId);
    Sleep(300);
    return w;
}

static void key(HWND w, WPARAM vk, int scancode)
{
    LPARAM down = 1 | (scancode << 16), up = down | (1u << 30) | (1u << 31);
    PostMessageW(w, WM_KEYDOWN, vk, down);
    Sleep(150);
    PostMessageW(w, WM_KEYUP, vk, up);
    Sleep(150);
}

/* Clique esquerdo de verdade no centro de um retângulo da área cliente. */
static void click(HWND w, int x, int y)
{
    LPARAM pos = MAKELPARAM(x, y);
    PostMessageW(w, WM_MOUSEMOVE, 0, pos);
    Sleep(150);
    PostMessageW(w, WM_LBUTTONDOWN, MK_LBUTTON, pos);
    Sleep(150);
    PostMessageW(w, WM_LBUTTONUP, 0, pos);
    Sleep(300);
}

static DWORD finish(PROCESS_INFORMATION *pi, DWORD timeout_ms)
{
    DWORD code = 0xFFFFFFFF;
    if (WaitForSingleObject(pi->hProcess, timeout_ms) != WAIT_OBJECT_0)
        TerminateProcess(pi->hProcess, 99);
    else
        GetExitCodeProcess(pi->hProcess, &code);
    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
    return code;
}

static size_t read_file(const char *dir, char *buf, size_t size)
{
    char appdata[MAX_PATH], file[MAX_PATH + 32];
    GetFullPathNameA(dir, MAX_PATH, appdata, NULL);
    snprintf(file, sizeof file, "%s\\Domino\\jogador.txt", appdata);
    FILE *f = fopen(file, "rb");
    if (!f)
        return (size_t)-1;
    size_t n = fread(buf, 1, size, f);
    fclose(f);
    return n;
}

/* Backspace e clique reais: "Anax" + Backspace + clique em Jogar grava "Ana";
   depois, outro processo com o clique em Sair termina sozinho com 0 sem mexer no arquivo. */
static int cli_menu_backspace_and_clicks(void)
{
    char file[MAX_PATH + 32], appdata[MAX_PATH];
    GetFullPathNameA("tmp_menu_cli2", MAX_PATH, appdata, NULL);
    snprintf(file, sizeof file, "%s\\Domino\\jogador.txt", appdata);
    remove(file);

    PROCESS_INFORMATION pi;
    HWND w = launch("tmp_menu_cli2", &pi);
    CHECK(w != NULL);
    for (const char *p = "Anax"; *p; p++) {
        PostMessageW(w, WM_CHAR, (WPARAM)*p, 1);
        Sleep(60);
    }
    key(w, VK_BACK, 0x0E);
    click(w, 640, 387); /* centro de Jogar: {540, 360, 200, 55} */
    PostMessageW(w, WM_CLOSE, 0, 0);
    CHECK(finish(&pi, 10000) == 0);
    char buf[16] = {0};
    size_t n = read_file("tmp_menu_cli2", buf, sizeof buf);
    if (n != 3 || memcmp(buf, "Ana", 3) != 0) {
        fprintf(stderr, "arquivo: %d bytes '%.*s'\n", (int)n, (int)(n > 16 ? 0 : n), buf);
        CHECK(false);
    }

    w = launch("tmp_menu_cli2", &pi);
    CHECK(w != NULL);
    click(w, 640, 467); /* centro de Sair: {540, 440, 200, 55} */
    CHECK(finish(&pi, 5000) == 0); /* sem WM_CLOSE: só o clique fecha */
    n = read_file("tmp_menu_cli2", buf, sizeof buf);
    CHECK(n == 3 && memcmp(buf, "Ana", 3) == 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"cli_menu_types_and_saves", cli_menu_types_and_saves},
        {"cli_menu_backspace_and_clicks", cli_menu_backspace_and_clicks},
    };
    if (argc < 3) {
        fprintf(stderr, "uso: %s <teste> <domino.exe>\n", argv[0]);
        return 2;
    }
    exe_path = argv[2];
    return RUN_TESTS(cases);
}
