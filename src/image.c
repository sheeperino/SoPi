#include "image.h"
#include "sorting.h"
#include <string.h>

#undef STB_IMAGE_WRITE_STATIC
#include "../external/stb_image.h"
#include "../external/stb_image_write.h"
#include "../external/stb_image_resize2.h"

static uint8_t *orig_data;
uint8_t *Data;
bool *mask;
int X, Y, N;

void image_mask(uint8_t *img, int x, int y, bool *mask, bool invert, bool f(PsColor)) {
  for (int i = 0; i < x*y; ++i) {
    uint32_t p = ((uint32_t *)img)[i];
    PsColor c = abgr2col(p);
    mask[i] = f(c);
    if (invert) mask[i] = !mask[i];
  }
}

void image_reset() {
  // WIP
  // memcpy(data, orig_data, x*y*CHANNELS);
}

int image_load(const char *path) {
  orig_data = stbi_load(path, &X, &Y, &N, CHANNELS);
  Data = malloc(X*Y*CHANNELS);
  memcpy(Data, orig_data, X*Y*CHANNELS);
  return (Data ? 1 : 0);
}

// in sorting or image?
void image_sort(uint8_t *img, int x, int y, bool gay, bool mask_only, bool no_mask, bool inv_mask, bool threshold_f(PsColor)) {
  mask = malloc(x*y);
  if (!no_mask) image_mask(img, x, y, mask, inv_mask, threshold_f);
  else memset(mask, 1, x*y);

  if (mask_only) {
    // printf("Generating mask...\n");
    for (int i = 0; i < x*y; ++i) {
      uint32_t mask_on = (gay ? ((uint32_t *)img)[i] : 0xFFFFFFFF);
      ((uint32_t *)img)[i] = (mask[i] ? mask_on : 0xFF000000);
    }
  }
  if (gay || !mask_only) {
    // printf("Sorting...\n");
    if (sort_direction == UP || sort_direction == DOWN)
      sort_intervals_vert(img, x, y, mask, gay);
    else
      sort_intervals_horiz(img, x, y, mask, gay);
  }
}

int image_resize(uint8_t *out, int width, int height, int *out_x, int *out_y) {
  *out_x = width;
  *out_y = height;
  out = stbir_resize_uint8_linear(Data, X, Y, X*CHANNELS, NULL,
                                   width, height, width*CHANNELS, STBIR_ABGR);
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

int image_write(const char *path, float resize_factor) {
  return stbi_write_png(path, (int)(X*resize_factor), (int)(Y*resize_factor),
                        CHANNELS, Data, (int)(X*resize_factor)*CHANNELS);
}

void image_free() {
  free(mask);
  stbi_image_free(Data);
}
