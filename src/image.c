#include "image.h"
#include "sorting.h"
#include <string.h>

#undef STB_IMAGE_WRITE_STATIC
#include "../external/stb_image.h"
#include "../external/stb_image_write.h"
#include "../external/stb_image_resize2.h"

static uint8_t *orig_data;
uint8_t *data;
bool *mask;
int x, y, n;

void image_mask(uint8_t *img, bool *mask, bool invert, bool f(PsColor)) {
  for (int i = 0; i < x*y; ++i) {
    uint32_t p = ((uint32_t *)img)[i];
    PsColor c = abgr2col(p);
    mask[i] = f(c);
    if (invert) mask[i] = !mask[i];
  }
}

int image_load(const char *path) {
  orig_data = stbi_load(path, &x, &y, &n, CHANNELS);
  data = malloc(x*y*CHANNELS);
  memcpy(data, orig_data, x*y*CHANNELS);
  return (data ? 1 : 0);
}

// in sorting or image?
void image_sort(bool gay, bool mask_only, bool no_mask, bool inv_mask, bool threshold_f(PsColor)) {
  mask = malloc(x*y);
  if (!no_mask) image_mask(data, mask, inv_mask, threshold_f);
  else memset(mask, 1, x*y);

  if (mask_only) {
    printf("Generating mask...\n");
    for (int i = 0; i < x*y; ++i) {
      uint32_t mask_on = (gay ? ((uint32_t *)data)[i] : 0xFFFFFFFF);
      ((uint32_t *)data)[i] = (mask[i] ? mask_on : 0xFF000000);
    }
  }
  if (gay || !mask_only) {
    printf("Sorting...\n");
    if (sort_direction == UP || sort_direction == DOWN)
      sort_intervals_vert(data, mask, gay);
    else
      sort_intervals_horiz(data, mask, gay);
  }
}

int image_resize(float resize_factor) {
  if (resize_factor != 1.0) {
    data = stbir_resize_uint8_linear(
        data, x, y, x*CHANNELS, NULL, (int)(x*resize_factor),
        (int)(y*resize_factor), (int)(x*resize_factor)*CHANNELS,
        STBIR_ABGR);
  }
  return (data ? 1 : 0);
}

int image_write(const char *path, float resize_factor) {
  return stbi_write_png(path, (int)(x*resize_factor), (int)(y*resize_factor),
                        CHANNELS, data, (int)(x*resize_factor)*CHANNELS);
}

void image_free() {
  free(mask);
  stbi_image_free(data);
}
