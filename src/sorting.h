#ifndef SORTING_H_
#define SORTING_H_

#include "color.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum { UP, DOWN, RIGHT, LEFT, SORT_DIR_INVALID } SortDirection;

extern int MIN, MAX;
extern SortDirection sort_direction;

// mask sort functions
bool by_hue(PsColor c);
bool by_saturation(PsColor c);
bool by_value(PsColor c);
bool by_red(PsColor c);
bool by_green(PsColor c);
bool by_blue(PsColor c);
bool by_alpha(PsColor c); // mostly really useful for transparent images

// pixel sorting functions
int sort_pixels(const void *p1, const void *p2);

// intervals (strip of white mask pixels)
// should be filled with a random color which is reset when
// encountering black or when going to a new row
void sort_intervals_horiz(uint8_t *data, int x, int y, bool *mask, bool gay);
void sort_intervals_vert(uint8_t *data, int x, int y,  bool *mask, bool gay);

#endif // SORTING_H_
