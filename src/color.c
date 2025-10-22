#include "color.h"
#include <math.h>
#include <stdlib.h>

PsColor rand_color() {
  PsColor c;
  c.a = 0xFF;
  c.r = rand();
  c.g = rand();
  c.b = rand();
  return c;
}

PsColor abgr2col(uint32_t abgr) {
  PsColor c;
  c.a = (abgr >> 24) & 0xFF;
  c.b = (abgr >> 16) & 0xFF;
  c.g = (abgr >>  8) & 0xFF;
  c.r =  abgr        & 0xFF;
  return c;
}

Hsv col2hsv(PsColor col) {
  Hsv hsv;
  float min, max, delta;

  min = col.r < col.g ? col.r : col.g;
  min = min < col.b ? min : col.b;

  max = col.r > col.g ? col.r : col.g;
  max = max > col.b ? max : col.b;

  hsv.v = max; // v
  delta = max - min;
  if (delta < 0.00001) {
    hsv.s = 0;
    hsv.h = 0; // undefined, maybe nan?
    return hsv;
  }
  if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
    hsv.s = (delta/max); // s
  } else {
    // if max is 0, then r = g = b = 0
    // s = 0, h is undefined
    hsv.s = 0.0;
    hsv.h = NAN; // its now undefined
    return hsv;
  }
  if (col.r >= max)                  // > is bogus, just keeps compilor happy
    hsv.h = (col.g - col.b) / delta; // between yellow & magenta
  else if (col.g >= max)
    hsv.h = 2.0 + (col.b - col.r) / delta; // between cyan & yellow
  else
    hsv.h = 4.0 + (col.r - col.g) / delta; // between magenta & cyan

  hsv.h *= 60.0; // degrees

  if (hsv.h < 0.0)
    hsv.h += 360.0;

  return hsv;
}
