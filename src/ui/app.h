#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "controller.h"
#include "menu.h"

typedef struct {
    uint64_t seed;
    int max_frames;          /* 0 = até fechar a janela */
    bool force_window_fail;  /* testes: simula a janela que não abre */
    void (*on_frame)(int frame, void *user); /* testes: chamado com o quadro desenhado */
    void *user;
    void (*on_state)(const Controller *c, void *user); /* testes: estado do controlador no quadro 0 */
    bool show_menu;          /* tela inicial antes da partida; só a domino.exe liga */
    const char *name_path;   /* arquivo do nome; NULL = não lê nem grava */
    /* testes: ação da tela inicial no quadro, no lugar do teclado e do mouse */
    MenuAction (*menu_input)(int frame, Menu *m, void *user);
    /* testes: chamado uma vez quando a partida começa */
    void (*on_game_start)(const Controller *c, void *user);
    /* testes: clique do mouse na tela inicial no quadro (true = clicou em *x, *y), no lugar do mouse */
    bool (*mouse_input)(int frame, float *x, float *y, void *user);
} AppOptions;

int app_run(const AppOptions *opt);

#endif
