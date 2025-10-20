// NOTE: important
// credit:
//   http://satyarth.me/articles/pixel-sorting/
//   https://stackoverflow.com/a/6930407 (rgb to hsv)

#define NOB_IMPLEMENTATION
#include "../nob.h"
#define FLAG_IMPLEMENTATION
#include "../flag.h"

#include "image.h"
#include "sorting.h"

#include <time.h>
#include <stdio.h>

#define IMG_DIR  "img/"
#define OUT_DIR  "out/"
#define OUTNAME  "sorted_"

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
  fprintf(stream, "Note - Interval ranges are as follows: \n[rgb, saturation, value (0-255)], [hue (0-360)]\n");
  fprintf(stream, "Options:\n");
  flag_print_options(stream);
}

int main(int argc, char **argv) {
  size_t *flag_min = flag_size("m", 150, "Minimum mask threshold");
  size_t *flag_max = flag_size("M", 250, "Maximum mask threshold");
  char **flag_thresh_fun = flag_str("t", "value", "Property used for the mask threshold [red, green, blue, alpha, hue, saturation, value]");
  float *flag_resize_factor = flag_float("r", 1.0, "Resize factor");
  bool *flag_gay = flag_bool("gay", false, "Skips pixel sorting, giving it a rainbow stripes effect");
  bool *flag_mask_only = flag_bool("mask", false, "Only generate the mask without sorting");
  bool *flag_no_mask = flag_bool("nomask", false, "Don't generate the mask. Sorts ALL pixels in a given direction");
  bool *flag_inv_mask = flag_bool("inv", false, "Invert the mask");
  char **flag_dir = flag_str("dir", "right", "Direction in which to sort pixels [up, down, right, left]");
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

  const char *FILENAME = *rest_argv;
  float resize_factor = *flag_resize_factor;
  bool gay = *flag_gay;
  bool mask_only = *flag_mask_only;
  bool no_mask = *flag_no_mask;
  bool inv_mask = *flag_inv_mask;
  MIN = *flag_min;
  MAX = *flag_max;

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

  if      (!strcmp("up",    *flag_dir) || !strcmp("U", *flag_dir)) sort_direction = UP;
  else if (!strcmp("down",  *flag_dir) || !strcmp("D", *flag_dir)) sort_direction = DOWN;
  else if (!strcmp("right", *flag_dir) || !strcmp("R", *flag_dir)) sort_direction = RIGHT;
  else if (!strcmp("left",  *flag_dir) || !strcmp("L", *flag_dir)) sort_direction = LEFT;
  else {
    usage(stderr);
    fprintf(stderr, "ERROR: Invalid direction provided\n");
    exit(1);
  }

  bool (*thresh_fun)(Color);
  if      (!strcmp("red",        *flag_thresh_fun) || !strcmp("r", *flag_thresh_fun)) thresh_fun = by_red;
  else if (!strcmp("green",      *flag_thresh_fun) || !strcmp("g", *flag_thresh_fun)) thresh_fun = by_green;
  else if (!strcmp("blue",       *flag_thresh_fun) || !strcmp("b", *flag_thresh_fun)) thresh_fun = by_blue;
  else if (!strcmp("alpha",      *flag_thresh_fun) || !strcmp("a", *flag_thresh_fun)) thresh_fun = by_alpha;
  else if (!strcmp("hue",        *flag_thresh_fun) || !strcmp("h", *flag_thresh_fun)) thresh_fun = by_hue;
  else if (!strcmp("saturation", *flag_thresh_fun) || !strcmp("s", *flag_thresh_fun)) thresh_fun = by_saturation;
  else if (!strcmp("value",      *flag_thresh_fun) || !strcmp("v", *flag_thresh_fun)) thresh_fun = by_value;
  else {
    usage(stderr);
    fprintf(stderr, "ERROR: Invalid threshold property provided\n");
    exit(1);
  }

  printf("sorting direction = %s (%d)\n", *flag_dir, sort_direction);
  printf("resize factor = %f\n", resize_factor);
  printf("min threshold = %zu\nmax threshold = %zu\n", MIN, MAX);
  printf("threshold property = %s\n", *flag_thresh_fun);
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

  image_sort(gay, mask_only, no_mask, inv_mask, thresh_fun);

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
