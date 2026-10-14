#ifndef NAMEFILE_H
#define NAMEFILE_H

#include <stdbool.h>
#include <stddef.h>

#include "menu.h"

/* Lê o nome salvo (door 1 de tela-inicial). Retorna true e preenche `out` com até 16
   caracteres; se o arquivo não existe, está vazio ou tem caractere não aceito, `out`
   fica vazio e retorna false. */
bool name_load(const char *path, char out[NAME_BUF]);
/* Grava o nome em UTF-8, sem BOM e sem quebra de linha, criando a pasta do arquivo. */
bool name_save(const char *path, const char *name);
/* `<appdata>\Domino\jogador.txt` */
void name_path_from_appdata(const char *appdata, char *out, size_t size);

#endif
