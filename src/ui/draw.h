#ifndef DRAW_H
#define DRAW_H

#include "raylib.h"

#include "controller.h"

#define TABLE_COLOR CLITERAL(Color){20, 100, 50, 255}

void draw_frame(const Controller *c, Font font);

#endif
