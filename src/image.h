#ifndef IMAGE_H_
#define IMAGE_H_

#include "color.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STBI_MAX_DIMENSIONS (1 << 27)
#define CHANNELS 4

// globals
extern uint8_t *Data;
extern bool *mask;
extern size_t MIN, MAX;
extern int X, Y, N;

void image_mask(uint8_t *img, int x, int y, bool *mask, bool invert, bool f(PsColor));
void image_reset();
int image_load(const char *path);
void image_sort(uint8_t *img, int x, int y, bool gay, bool mask_only, bool no_mask, bool inv_mask, bool threshold_f(PsColor));
int image_resize(uint8_t *out, int width, int height, int *out_x, int *out_y);
int image_resize_fact(uint8_t *out, float resize_factor, int *out_x, int *out_y);
int image_write(const char *path, float resize_factor);
void image_free();

#endif // IMAGE_H_
