#ifndef SORTING_H_
#define SORTING_H_

#include "color.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum { UP, DOWN, RIGHT, LEFT, SORT_DIR_INVALID } SortDirection;
typedef enum {
  BY_HUE, BY_SATURATION, BY_VALUE,
  BY_RED, BY_GREEN, BY_BLUE, BY_ALPHA
} ThresholdSortBy;
typedef struct { int min, max; } ThresholdBounds;
typedef bool (*thresh_sort_func)(PsColor);

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

ThresholdBounds get_threshold_bounds(ThresholdSortBy sort_by);
thresh_sort_func enum_to_thresh_func(ThresholdSortBy sort_by);

// pixel sorting functions
int sort_pixels(const void *p1, const void *p2);

// intervals (strip of white mask pixels)
// should be filled with a random color which is reset when
// encountering black or when going to a new row
void sort_intervals_horiz(uint8_t *data, int x, int y, bool *mask, bool gay);
void sort_intervals_vert(uint8_t *data, int x, int y,  bool *mask, bool gay);

#endif // SORTING_H_
