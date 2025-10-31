#ifndef GUI_DRAW_H_
#define GUI_DRAW_H_

#include "raylib.h"
#include "state.h"

void draw_init_raygui();
bool draw_slider_with_value_box(Rectangle box_rect, Rectangle slider_rect, int min, int max, int *value);
void draw_main_gui(State *s);
void draw_help_menu(State *s);
void draw_image(State *s);
void draw_sidebar(State *s, Rectangle r);

#endif // GUI_DRAW_H_
