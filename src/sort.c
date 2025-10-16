// NOTE: important
// credit:
//   http://satyarth.me/articles/pixel-sorting/
//   https://stackoverflow.com/a/6930407 (rgb to hsv)

#undef STB_IMAGE_WRITE_STATIC
#define NOB_IMPLEMENTATION
#include "../nob.h"
#define FLAG_IMPLEMENTATION
#include "../flag.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMG_DIR  "img/"
#define OUT_DIR  "out/"
#define OUTNAME  "sorted_"

#define STBI_MAX_DIMENSIONS (1 << 27)
#define CHANNELS 4

// these used to be defines lol
static size_t MIN;
static size_t MAX;
static const char *FILENAME;
static int x, y, n;

typedef struct {
  float h, s, v;
} Hsv;

typedef struct {
  uint8_t r, g, b, a;
} Color;

Color rand_color() {
  Color c;
  c.a = 0xFF;
  c.r = rand();
  c.g = rand();
  c.b = rand();
  return c;
}

Color abgr2col(uint32_t abgr) {
  Color c;
  c.a = (abgr >> 24) & 0xFF;
  c.b = (abgr >> 16) & 0xFF;
  c.g = (abgr >>  8) & 0xFF;
  c.r =  abgr        & 0xFF;
  return c;
}

uint32_t col2abgr(Color c) {
  return c.a << 24 | c.b << 16 | c.g << 8 | c.r;
}

Hsv col2hsv(Color col) {
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
    hsv.s = (delta / max); // s
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

#define MASK_THRESHOLD(val) (val >= MIN && val <= MAX ? 1 : 0)

int by_value(Color c) {
  Hsv hsv = col2hsv(c);
  return MASK_THRESHOLD(hsv.v);
}
int by_red(Color c) { return MASK_THRESHOLD(c.r); }
int by_green(Color c) { return MASK_THRESHOLD(c.g); }
int by_blue(Color c) { return MASK_THRESHOLD(c.b); }

void image_mask(unsigned char *img, char *mask, int f(Color)) {
  for (int i = 0; i < x * y; ++i) {
    uint32_t p = ((uint32_t *)img)[i];
    Color c = abgr2col(p);
    mask[i] = f(c);
    // mask[i] = (hsv.v >= MIN && hsv.v <= MAX ? 1 : 0);
  }
}

int sort_pixels(const void *p1, const void *p2) {
  Color c1 = abgr2col(*(uint32_t *)p1);
  Color c2 = abgr2col(*(uint32_t *)p2);
  return c2.g - c1.g;
  // return *(uint32_t *)p2 - *(uint32_t *)p1;
}

inline bool is_dir_sep(char c) { return c == '/' || c == '\\'; }

// we don't care about invalid paths since they'll already be checked
const char *basename(const char *s) {
  if (!s || !*s) return NULL;
  size_t i = strlen(s) - 1;
  for (; i != 0 && !is_dir_sep(s[i]); --i);
  return s + i + (i == 0 ? 0 : 1);
}

void usage(FILE *stream) {
  fprintf(stream, "Usage: [OPTIONS] [FILE]\n");
  fprintf(stream, "Options:\n");
  flag_print_options(stream);
}

int main(int argc, char **argv) {
  size_t *flag_min = flag_size("m", 150, "Minimum threshold");
  size_t *flag_max = flag_size("M", 250, "Maximum threshold");
  float *flag_resize_factor = flag_float("r", 1.0, "Resize factor");
  bool *help = flag_bool("help", false, "Print this message");
  char **flag_out_path = flag_str("o", NULL, "Custom output path\n        Default: ./out/sorted_<FILENAME>");

  if (!flag_parse(argc, argv)) {
    usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }
  if (*help) {
    usage(stdout);
    exit(0);
  }

  int rest_argc = flag_rest_argc();
  char **rest_argv = flag_rest_argv();

  if (!rest_argc) {
    usage(stderr);
    fprintf(stderr, "ERROR: No input provided\n");
    exit(1);
  }
  if (rest_argc > 1) {
    usage(stderr);
    fprintf(stderr, "ERROR: Too many inputs provided\n");
    exit(1);
  }

  float resize_factor = *flag_resize_factor;
  MIN = *flag_min;
  MAX = *flag_max;
  FILENAME = *rest_argv;

  const char *img_path;
  if (!nob_file_exists(img_path = FILENAME)) {
    if (is_dir_sep(FILENAME[0]) || !nob_file_exists(img_path = nob_temp_sprintf(IMG_DIR "%s", FILENAME))) {
      fprintf(stderr, "ERROR: Input path is not valid\n");
      return 1;
    }
  }

  const char *out_path;
  if (*flag_out_path) out_path = *flag_out_path;
  else out_path = nob_temp_sprintf(OUT_DIR OUTNAME "%s", basename(FILENAME));

  printf("stbi_write_tga_with_rle = %d\n", stbi_write_tga_with_rle);
  printf("resize factor = %f\n", resize_factor);
  printf("min threshold = %zu\nmax threshold = %zu\n", MIN, MAX);
  printf("input image = %s\n", img_path);
  printf("output image = %s\n", out_path);

  int status = 0;
  srand(time(0));
  printf("Loading...\n");
  unsigned char *data = stbi_load(img_path, &x, &y, &n, CHANNELS);
  printf("Loaded.\n");
  char *mask = malloc(x * y);
  if (data == NULL) {
    fprintf(stderr, "ERROR: Couldn't read image\n");
    status = 1;
    goto defer;
  }

  image_mask(data, mask, by_value);

  // intervals (strip of white mask pixels)
  // should be filled with a random color which is reset when
  // encountering black or when going to a new row
  int new_strip = 1;
  uint32_t strip_col;
  int start = 0;
  for (int i = 0; i < x * y; ++i) {
    if (mask[i] == 0 || i / x != start / x) {
      if (!new_strip)
        qsort(data + start * CHANNELS, (i - start), sizeof(data[0]) * CHANNELS, sort_pixels);
      new_strip = 1;
    }
    if (mask[i] == 1 && new_strip) {
      Color r = rand_color();
      strip_col = col2abgr(r);
      new_strip = 0;
      start = i;
    }
  }
  (void)strip_col;

  // for (int i = 0; i < x * y; ++i) {
  //   ((uint32_t *)data)[i] = (mask[i] ? 0xFFFFFFFF : 0xFF000000);
  // }

  printf("Resizing...\n");
  if (resize_factor != 1.0) {
    data = stbir_resize_uint8_linear(
        data, x, y, x * CHANNELS, NULL, (int)(x * resize_factor),
        (int)(y * resize_factor), (int)(x * resize_factor) * CHANNELS,
        STBIR_ABGR);
  }
  printf("Resized.\n");

  printf("Writing...\n");
  if (!stbi_write_png(out_path, (int)(x * resize_factor), (int)(y * resize_factor),
                      CHANNELS, data, (int)(x * resize_factor) * CHANNELS)) {
    fprintf(stderr, "ERROR: Couldn't write image\n");
    status = 1;
  }
  printf("Wrote.\n");

defer:
  free(mask);
  stbi_image_free(data);
  return status;
}
