#include <direct.h>
#include <stdio.h>
#include <string.h>

#include "namefile.h"

#define MAX_FILE 1024

bool name_load(const char *path, char out[NAME_BUF])
{
    out[0] = '\0';
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    unsigned char buf[MAX_FILE + 1];
    size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    if (n == 0 || n > MAX_FILE)
        return false;

    Menu m;
    menu_init(&m, NULL);
    for (size_t i = 0; i < n; i++) {
        int cp = buf[i];
        if (cp >= 0x80) {
            /* só sequências de 2 bytes; os códigos aceitos vão até 255 */
            if (cp < 0xC2 || cp > 0xDF || i + 1 >= n || (buf[i + 1] & 0xC0) != 0x80)
                return false;
            cp = ((cp & 0x1F) << 6) | (buf[i + 1] & 0x3F);
            i++;
        }
        if (!menu_accepts(cp))
            return false;
        menu_type(&m, cp); /* passa de 16: o resto é ignorado */
    }
    if (m.chars == 0)
        return false;
    memcpy(out, m.name, sizeof m.name);
    return true;
}

/* Cria cada pasta do caminho até o arquivo; erros ficam para o fopen descobrir. */
static void make_parents(const char *path)
{
    char dir[512];
    size_t len = strlen(path);
    if (len >= sizeof dir)
        return;
    for (size_t i = 1; i < len; i++) {
        if (path[i] == '/' || path[i] == '\\') {
            memcpy(dir, path, i);
            dir[i] = '\0';
            if (!(i == 2 && dir[1] == ':'))
                _mkdir(dir);
        }
    }
}

bool name_save(const char *path, const char *name)
{
    make_parents(path);
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;
    size_t len = strlen(name);
    bool ok = fwrite(name, 1, len, f) == len;
    return fclose(f) == 0 && ok;
}

void name_path_from_appdata(const char *appdata, char *out, size_t size)
{
    snprintf(out, size, "%s\\Domino\\jogador.txt", appdata);
}
