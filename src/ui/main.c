#include <stdio.h>
#include <time.h>

#include "app.h"
#include "cli.h"

int main(int argc, char **argv)
{
    AppOptions opt = {0};
    bool has_seed;
    if (cli_parse(argc, argv, &opt.seed, &has_seed) != 0) {
        fprintf(stderr, "%s\n", CLI_USAGE);
        return 2;
    }
    if (!has_seed)
        opt.seed = (uint64_t)time(NULL);
    return app_run(&opt);
}
