#ifndef SORTING_H_
#define SORTING_H_

#include <stdbool.h>
#include "color.h"

// mask sort functions
bool by_value(Color c);
bool by_red(Color c);
bool by_green(Color c);
bool by_blue(Color c);

// pixel sorting functions
int sort_pixels(const void *p1, const void *p2);

// intervals (strip of white mask pixels)
// should be filled with a random color which is reset when
// encountering black or when going to a new row
void sort_intervals_horiz(uint8_t *data, bool *mask, bool gay);
void sort_intervals_vert(uint8_t *data, bool *mask, bool gay);

#endif // SORTING_H_
