#ifndef COLOR_H_
#define COLOR_H_

#include <stdint.h>
#define col2abgr(c) (c.a << 24 | c.b << 16 | c.g << 8 | c.r)

typedef struct {
  float h, s, v;
} Hsv;

typedef struct {
  uint8_t r, g, b, a;
} Color;

Color rand_color();
Color abgr2col(uint32_t abgr);
Hsv col2hsv(Color col);

#endif // COLOR_H_
