#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdint.h>

#define CLI_USAGE "uso: domino [--seed <n>]"

/* Retorna 0 se os argumentos são válidos, 2 caso contrário. */
int cli_parse(int argc, char **argv, uint64_t *seed, bool *has_seed);

#endif
