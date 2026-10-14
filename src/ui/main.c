#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "app.h"
#include "cli.h"
#include "namefile.h"

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

    /* tela inicial e nome salvo em %APPDATA%\Domino\jogador.txt (door 1 e 2 de tela-inicial) */
    char path[512];
    const char *appdata = getenv("APPDATA");
    opt.show_menu = true;
    if (appdata && appdata[0]) {
        name_path_from_appdata(appdata, path, sizeof path);
        opt.name_path = path;
    }
    return app_run(&opt);
}
