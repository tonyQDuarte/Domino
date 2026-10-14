#ifndef BOTS_H
#define BOTS_H

#include "domino.h"

#define BOT_COUNT 6
#define BOT_DEFAULT 2 /* Audi: o estilo de antes desta feature */

/* Um bot: nome, texto do estilo no cartão da tela inicial e o estilo de jogo. */
typedef struct {
    const char *name;
    const char *style_text;
    CpuStyle style;
} Bot;

/* Do mais colaborativo para o mais egoísta. */
extern const Bot BOTS[BOT_COUNT];

#endif
