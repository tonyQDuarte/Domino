#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "controller.h"

typedef struct {
    uint64_t seed;
    int max_frames;          /* 0 = até fechar a janela */
    bool force_window_fail;  /* testes: simula a janela que não abre */
    void (*on_frame)(int frame, void *user); /* testes: chamado com o quadro desenhado */
    void *user;
    void (*on_state)(const Controller *c, void *user); /* testes: estado do controlador no quadro 0 */
} AppOptions;

int app_run(const AppOptions *opt);

#endif
