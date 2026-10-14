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

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"cli_menu_types_and_saves", cli_menu_types_and_saves},
    };
    if (argc < 3) {
        fprintf(stderr, "uso: %s <teste> <domino.exe>\n", argv[0]);
        return 2;
    }
    exe_path = argv[2];
    return RUN_TESTS(cases);
}
