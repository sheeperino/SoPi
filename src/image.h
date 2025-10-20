#ifndef IMAGE_H_
#define IMAGE_H_

#include "color.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STBI_MAX_DIMENSIONS (1 << 27)
#define CHANNELS 4

// globals
extern uint8_t *data;
extern bool *mask;
extern size_t MIN, MAX;
extern int x, y, n;

void image_mask(uint8_t *img, bool *mask, bool f(Color));
int image_load(const char *path);
void image_sort(bool gay, bool mask_only, bool no_mask);
int image_resize(float resize_factor);
int image_write(const char *path, float resize_factor);
void image_free();

#endif // IMAGE_H_
