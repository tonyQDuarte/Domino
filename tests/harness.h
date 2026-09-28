#ifndef HARNESS_H
#define HARNESS_H

#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            fprintf(stderr, "%s:%d: falhou: %s\n", __FILE__, __LINE__, #cond); \
            return 1;                                                      \
        }                                                                  \
    } while (0)

typedef struct {
    const char *name;
    int (*fn)(void);
} TestCase;

static int run_named(const TestCase *cases, int n, int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s <teste>\n", argv[0]);
        return 2;
    }
    for (int i = 0; i < n; i++)
        if (strcmp(cases[i].name, argv[1]) == 0)
            return cases[i].fn();
    fprintf(stderr, "teste desconhecido: %s\n", argv[1]);
    return 2;
}

#define RUN_TESTS(cases) \
    run_named(cases, (int)(sizeof(cases) / sizeof(cases[0])), argc, argv)

#endif
