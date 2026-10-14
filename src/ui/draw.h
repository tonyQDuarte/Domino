#ifndef DRAW_H
#define DRAW_H

#include "raylib.h"

#include "controller.h"
#include "menu.h"

#define TABLE_COLOR CLITERAL(Color){20, 100, 50, 255}

void draw_frame(const Controller *c, Font font);
void draw_menu(const Menu *m, Font font, bool cursor_on);

#endif
