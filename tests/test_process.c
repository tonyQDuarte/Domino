/* Executa o domino.exe de verdade (caminho em argv[2]) e observa janela, stderr e código de saída. */
#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"

static const char *exe_path;

typedef struct {
    PROCESS_INFORMATION pi;
    HANDLE err_read;
} Child;

static bool spawn(Child *c, const char *args)
{
    SECURITY_ATTRIBUTES sa = {sizeof sa, NULL, TRUE};
    HANDLE err_write;
    if (!CreatePipe(&c->err_read, &err_write, &sa, 0))
        return false;
    SetHandleInformation(c->err_read, HANDLE_FLAG_INHERIT, 0);

    char cmd[1024];
    snprintf(cmd, sizeof cmd, "\"%s\" %s", exe_path, args);
    STARTUPINFOA si;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = err_write;
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &c->pi);
    CloseHandle(err_write);
    return ok;
}

static void read_stderr(Child *c, char *buf, size_t size)
{
    size_t len = 0;
    DWORD got;
    while (len + 1 < size && ReadFile(c->err_read, buf + len, (DWORD)(size - 1 - len), &got, NULL) &&
           got > 0)
        len += got;
    buf[len] = '\0';
}

static DWORD finish(Child *c, DWORD timeout_ms)
{
    DWORD code = 0xFFFFFFFF;
    if (WaitForSingleObject(c->pi.hProcess, timeout_ms) != WAIT_OBJECT_0)
        TerminateProcess(c->pi.hProcess, 99);
    else
        GetExitCodeProcess(c->pi.hProcess, &code);
    CloseHandle(c->pi.hProcess);
    CloseHandle(c->pi.hThread);
    CloseHandle(c->err_read);
    return code;
}

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

/* Janela visível do processo, esperando até 10 s. */
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

static int cli_bad_args_exit_2(void)
{
    const char *cases[] = {"--seed x", "--seed -1", "--seed", "--foo"};
    for (int i = 0; i < 4; i++) {
        Child c;
        CHECK(spawn(&c, cases[i]));
        char err[512];
        read_stderr(&c, err, sizeof err);
        DWORD code = finish(&c, 5000);
        if (code != 2 || strstr(err, "uso: domino [--seed <n>]") == NULL) {
            fprintf(stderr, "caso '%s': código %lu, stderr '%s'\n", cases[i], code, err);
            CHECK(false);
        }
    }
    return 0;
}

static int cli_window_title_and_size(void)
{
    Child c;
    CHECK(spawn(&c, "--seed 1"));
    HWND w = wait_window(c.pi.dwProcessId);
    CHECK(w != NULL);
    wchar_t title[64];
    GetWindowTextW(w, title, 64);
    RECT r;
    GetClientRect(w, &r);
    PostMessageW(w, WM_CLOSE, 0, 0);
    finish(&c, 10000);
    CHECK(wcscmp(title, L"Dominó") == 0);
    CHECK(r.right - r.left == 1280);
    CHECK(r.bottom - r.top == 720);
    return 0;
}

static int cli_close_exits_0(void)
{
    Child c;
    CHECK(spawn(&c, "--seed 1"));
    HWND w = wait_window(c.pi.dwProcessId);
    CHECK(w != NULL);
    Sleep(300);
    PostMessageW(w, WM_CLOSE, 0, 0);
    DWORD code = finish(&c, 10000);
    CHECK(code == 0);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"cli_bad_args_exit_2", cli_bad_args_exit_2},
        {"cli_window_title_and_size", cli_window_title_and_size},
        {"cli_close_exits_0", cli_close_exits_0},
    };
    if (argc < 3) {
        fprintf(stderr, "uso: %s <teste> <domino.exe>\n", argv[0]);
        return 2;
    }
    exe_path = argv[2];
    return RUN_TESTS(cases);
}
