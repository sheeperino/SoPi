#include "image.h"
#include "sorting.h"
#include <string.h>
#include "../external/stb_image.h"
#include "../external/stb_image_write.h"
#include "../external/stb_image_resize2.h"

static uint8_t *orig_data;
uint8_t *Data;
bool *mask;
int X, Y, N;

void image_mask(uint8_t *img, int x, int y, bool *mask, bool invert, thresh_sort_func f) {
  for (int i = 0; i < x*y; ++i) {
    uint32_t p = ((uint32_t *)img)[i];
    PsColor c = abgr2col(p);
    mask[i] = f(c);
    if (invert) mask[i] = !mask[i];
  }
}

void image_reset() {
  // WIP
  memcpy(Data, orig_data, X*Y*CHANNELS);
}

int image_load(const char *path) {
  orig_data = stbi_load(path, &X, &Y, &N, CHANNELS);
  if (!orig_data) return 0;
  Data = malloc(X*Y*CHANNELS);
  memcpy(Data, orig_data, X*Y*CHANNELS);
  return 1;
}

// in sorting or image?
void image_sort(uint8_t *img, int x, int y, bool gay, bool mask_only, bool no_mask, bool inv_mask, thresh_sort_func threshold_f) {
  mask = malloc(x*y);
  if (!no_mask) image_mask(img, x, y, mask, inv_mask, threshold_f);
  else memset(mask, 1, x*y);

  if (mask_only) {
    for (int i = 0; i < x*y; ++i) {
      uint32_t mask_on = (gay ? ((uint32_t *)img)[i] : 0xFFFFFFFF);
      ((uint32_t *)img)[i] = (mask[i] ? mask_on : 0xFF000000);
    }
  }
  if (gay || !mask_only) {
    if (sort_direction == UP || sort_direction == DOWN)
      sort_intervals_vert(img, x, y, mask, gay);
    else
      sort_intervals_horiz(img, x, y, mask, gay);
  }
}

int image_resize(uint8_t *out, int width, int height, int *out_x, int *out_y) {
  if (out_x) *out_x = width;
  if (out_y) *out_y = height;
  stbir_resize_uint8_linear(Data, X, Y, X*CHANNELS,
                                  out, width, height, width*CHANNELS, STBIR_ABGR);
  if (out == NULL) {
    *out_x = -1;
    *out_y = -1;
    return 0;
  }
  return 1;
}

int image_resize_fact(uint8_t *out, float resize_factor, int *out_x, int *out_y) {
  if (resize_factor != 1.0)
    return image_resize(out, X*resize_factor, Y*resize_factor, out_x, out_y);
  return 1;
}

int image_write(uint8_t *data, int width, int height, const char *path) {
  return stbi_write_png(path, width, height, CHANNELS, data, width*CHANNELS);
}

void image_free() {
  free(mask);
  stbi_image_free(Data);
}
