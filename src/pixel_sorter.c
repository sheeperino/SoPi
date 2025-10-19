// NOTE: important
// credit:
//   http://satyarth.me/articles/pixel-sorting/
//   https://stackoverflow.com/a/6930407 (rgb to hsv)

#define NOB_IMPLEMENTATION
#include "../nob.h"
#define FLAG_IMPLEMENTATION
#include "../flag.h"

#include "image.h"

#include <time.h>
#include <stdio.h>

#define IMG_DIR  "img/"
#define OUT_DIR  "out/"
#define OUTNAME  "sorted_"

static const char *FILENAME;
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
  bool *flag_gay = flag_bool("gay", false, "Skips pixel sorting, giving it a rainbow stripes effect");
  bool *flag_mask_only = flag_bool("mask", false, "Only generate the mask without sorting");
  char **flag_out_path = flag_str("o", NULL, "Custom output path\n        Default: ./out/sorted_<FILENAME>");
  bool *help = flag_bool("help", false, "Print this message");

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
  bool gay = *flag_gay;
  bool mask_only = *flag_mask_only;
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

  printf("resize factor = %f\n", resize_factor);
  printf("min threshold = %zu\nmax threshold = %zu\n", MIN, MAX);
  printf("input image = %s\n", img_path);
  printf("output image = %s\n", out_path);

  int result = 0;
  srand(time(0));
  printf("Loading...\n");
  if (!image_load(img_path)) {
    fprintf(stderr, "ERROR: Couldn't read image\n");
    nob_return_defer(1);
  } else {
    printf("Loaded.\n");
  }

  image_sort(gay, mask_only);

  printf("Resizing...\n");
  if (!image_resize(resize_factor)) {
    fprintf(stderr, "ERROR: Couldn't resize image\n");
    nob_return_defer(1);
  } else {
    printf("Resized.\n");
  }

  printf("Writing...\n");
  if (!image_write(out_path, resize_factor)) {
    fprintf(stderr, "ERROR: Couldn't write image\n");
    nob_return_defer(1);
  } else {
    printf("Wrote.\n");
  }

defer:
  image_free();
  return result;
}
