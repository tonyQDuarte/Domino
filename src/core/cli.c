#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

int cli_parse(int argc, char **argv, uint64_t *seed, bool *has_seed)
{
    *has_seed = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") != 0 || i + 1 >= argc)
            return 2;
        const char *s = argv[++i];
        if (*s == '\0')
            return 2;
        for (const char *p = s; *p; p++)
            if (*p < '0' || *p > '9')
                return 2;
        errno = 0;
        unsigned long long v = strtoull(s, NULL, 10);
        if (errno == ERANGE)
            return 2;
        *seed = (uint64_t)v;
        *has_seed = true;
    }
    return 0;
}
