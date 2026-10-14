#include <string.h>

#include "bots.h"
#include "menu.h"

void menu_init(Menu *m, const char *initial)
{
    memset(m, 0, sizeof *m);
    m->partner = BOT_DEFAULT;
    if (!initial)
        return;
    /* `initial` vem de name_load: já está validado e tem no máximo 16 caracteres */
    for (const unsigned char *p = (const unsigned char *)initial; *p; p++) {
        int cp = *p;
        if (cp >= 0xC0 && p[1]) {
            cp = ((cp & 0x1F) << 6) | (p[1] & 0x3F);
            p++;
        }
        menu_type(m, cp);
    }
}

bool menu_accepts(int cp) { return (cp >= 32 && cp <= 126) || (cp >= 160 && cp <= 255); }

bool menu_type(Menu *m, int cp)
{
    if (!menu_accepts(cp) || m->chars >= NAME_MAX_CHARS)
        return false;
    size_t len = strlen(m->name);
    if (cp < 0x80) {
        m->name[len++] = (char)cp;
    } else {
        m->name[len++] = (char)(0xC0 | (cp >> 6));
        m->name[len++] = (char)(0x80 | (cp & 0x3F));
    }
    m->name[len] = '\0';
    m->chars++;
    return true;
}

void menu_backspace(Menu *m)
{
    size_t len = strlen(m->name);
    if (len == 0)
        return;
    len--;
    /* byte de continuação: apaga também o byte que abre o caractere */
    while (len > 0 && ((unsigned char)m->name[len] & 0xC0) == 0x80)
        len--;
    m->name[len] = '\0';
    m->chars--;
}

bool menu_play_enabled(const Menu *m)
{
    for (const char *p = m->name; *p; p++)
        if (*p != ' ')
            return true;
    return false;
}

void menu_trimmed_name(const Menu *m, char out[NAME_BUF])
{
    const char *start = m->name;
    while (*start == ' ')
        start++;
    size_t len = strlen(start);
    while (len > 0 && start[len - 1] == ' ')
        len--;
    memcpy(out, start, len);
    out[len] = '\0';
}

bool menu_pick_partner(Menu *m, float x, float y)
{
    for (int i = 0; i < BOT_COUNT; i++)
        if (rect_contains(menu_card_rect(i), x, y)) {
            m->partner = i;
            return true;
        }
    return false;
}

MenuAction menu_click(const Menu *m, float x, float y)
{
    if (rect_contains(menu_quit_rect(), x, y))
        return MENU_QUIT;
    if (rect_contains(menu_play_rect(), x, y) && menu_play_enabled(m))
        return MENU_PLAY;
    return MENU_NONE;
}

MenuAction menu_press(Menu *m, float x, float y)
{
    menu_pick_partner(m, x, y);
    return menu_click(m, x, y);
}

MenuAction menu_enter(const Menu *m) { return menu_play_enabled(m) ? MENU_PLAY : MENU_NONE; }

RectF menu_title_rect(void) { return (RectF){440, 30, 400, 70}; }
RectF menu_label_rect(void) { return (RectF){490, 112, 300, 30}; }
RectF menu_field_rect(void) { return (RectF){490, 147, 300, 50}; }
RectF menu_partner_label_rect(void) { return (RectF){70, 214, 300, 30}; }

RectF menu_card_rect(int i)
{
    const float w = 180, gap = 12;
    float x0 = SCREEN_W / 2.0f - (BOT_COUNT * (w + gap) - gap) / 2;
    return (RectF){x0 + i * (w + gap), 250, w, 85};
}

RectF menu_play_rect(void) { return (RectF){540, 360, 200, 55}; }
RectF menu_quit_rect(void) { return (RectF){540, 440, 200, 55}; }
