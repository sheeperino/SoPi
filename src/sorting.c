#include "sorting.h"
#include "image.h"
#include <stdlib.h>

#define MASK_THRESHOLD(val) (val >= MIN && val <= MAX ? 1 : 0)

size_t MIN, MAX;
SortDirection sort_direction;

bool by_value(Color c) {
  Hsv hsv = col2hsv(c);
  return MASK_THRESHOLD(hsv.v);
}
bool by_red(Color c) { return MASK_THRESHOLD(c.r); }
bool by_green(Color c) { return MASK_THRESHOLD(c.g); }
bool by_blue(Color c) { return MASK_THRESHOLD(c.b); }

int sort_pixels(const void *p1, const void *p2) {
  Color c1 = abgr2col(*(uint32_t *)p1);
  Color c2 = abgr2col(*(uint32_t *)p2);
  // TODO: can choose between different values to sort
  if (sort_direction == LEFT || sort_direction == DOWN)
    return c1.g - c2.g;
  else
    return c2.g - c1.g;
  // return *(uint32_t *)p2 - *(uint32_t *)p1;
}

void sort_intervals_horiz(uint8_t *data, bool *mask, bool gay) {
  bool new_strip = true;
  uint32_t strip_col;
  int start = 0;
  for (int i = 0; i < x*y; ++i) {
    if (mask[i] == 0 || i/x != start/x) {
      if (!new_strip && !gay)
        qsort(data + start*CHANNELS, (i - start), sizeof(data[0])*CHANNELS, sort_pixels);
      new_strip = true;
    }
    if (mask[i] == 1 && new_strip) {
      Color r = rand_color();
      strip_col = col2abgr(r);
      new_strip = false;
      start = i;
    }
    if (gay && mask[i] == 1) ((uint32_t *)data)[i] = strip_col;
  }
}

void sort_intervals_vert(uint8_t *data, bool *mask, bool gay) {
  bool new_strip = true;
  uint32_t strip_col;
  int start = 0;
  uint32_t *tmp = malloc(y*CHANNELS);
  for (int i = 0; i < x*y; ++i) {
    size_t j = x*(i%y) + i/y;
    if (mask[j] == 0 || i/y != start/y) {
      if (!new_strip && !gay) {
        qsort(tmp, i - start, sizeof(uint32_t), sort_pixels);
        for (int k = start; k < i; ++k) {
          int idx = x*(k%y) + k/y;
          ((uint32_t *)data)[idx] = tmp[k - start];
        }
      }
      new_strip = true;
    }
    if (mask[j] == 1 && new_strip) {
      Color r = rand_color();
      strip_col = col2abgr(r);
      new_strip = false;
      start = i;
    }
    if (mask[j] == 1) tmp[i - start] = ((uint32_t *)data)[j];
    if (gay && mask[j] == 1) ((uint32_t *)data)[j] = strip_col;
  }
  free(tmp);
}
