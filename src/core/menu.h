#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

#include "layout.h"

#define NAME_MAX_CHARS 16
#define NAME_BUF 48 /* 16 caracteres de até 2 bytes em UTF-8, com folga */

#define MENU_TITLE "Dominó"
#define MENU_LABEL "Seu nome"
#define MENU_PLAY_TEXT "Jogar"
#define MENU_QUIT_TEXT "Sair"
#define MENU_PARTNER_LABEL "Seu parceiro"

typedef enum { MENU_NONE, MENU_PLAY, MENU_QUIT } MenuAction;

/* Estado da tela inicial: o nome em UTF-8, quantos caracteres ele tem e o parceiro. */
typedef struct {
    char name[NAME_BUF];
    int chars;
    int partner; /* índice em BOTS do parceiro escolhido */
} Menu;

/* Começa com `initial` (já validado por name_load) ou vazio. */
void menu_init(Menu *m, const char *initial);
/* Caracteres aceitos no nome: códigos 32-126 e 160-255. */
bool menu_accepts(int codepoint);
/* Acrescenta o caractere se for aceito e houver espaço. */
bool menu_type(Menu *m, int codepoint);
/* Apaga o último caractere inteiro. */
void menu_backspace(Menu *m);
/* O nome tem algo além de espaços. */
bool menu_play_enabled(const Menu *m);
/* O nome sem os espaços das pontas. */
void menu_trimmed_name(const Menu *m, char out[NAME_BUF]);
MenuAction menu_click(const Menu *m, float x, float y);
/* Clique num cartão escolhe esse bot como parceiro. Retorna false fora dos cartões. */
bool menu_pick_partner(Menu *m, float x, float y);
/* Clique do mouse na tela inicial: escolhe o cartão, se for um, e devolve a ação do clique. */
MenuAction menu_press(Menu *m, float x, float y);
MenuAction menu_enter(const Menu *m);

RectF menu_title_rect(void);
RectF menu_label_rect(void);
RectF menu_field_rect(void);
RectF menu_play_rect(void);
RectF menu_quit_rect(void);
RectF menu_partner_label_rect(void);
/* Cartão do bot i (0..5), na ordem de BOTS, numa faixa só. */
RectF menu_card_rect(int i);

#endif
