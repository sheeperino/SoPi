#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR   "src/"
#define EXT_DIR   "external/"

Cmd cmd = {0};
Procs procs = {0};

// lets define these before merging nob and nob_gui to make the compiler shut up
#define RAYLIB_SRC
static const char *raylib_modules;

#include "./platform/build_linux.c"
#include "./platform/build_win.c"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  if (!mkdir_if_not_exists(BUILD_DIR)) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/linux/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/windows/")) return 1;
  shift(argv, argc);

  if (!build_object_linux(BUILD_DIR "stb/linux/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION")) return 1;
  if (!build_object_linux(BUILD_DIR "stb/linux/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION")) return 1;
  if (!build_object_linux(BUILD_DIR "stb/linux/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION")) return 1;

  if (!build_object_windows(BUILD_DIR "stb/windows/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION")) return 1;
  if (!build_object_windows(BUILD_DIR "stb/windows/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION")) return 1;
  if (!build_object_windows(BUILD_DIR "stb/windows/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION")) return 1;

  if (!procs_flush(&procs)) return 1;

	// cc -std=c99 -o sort sort.c stb_image.o stb_image_write.o -lm -O3 -march=native
  cmd_append(&cmd, "cc", "-Wextra", "-Wall"/* , "-std=c99" */, "-ggdb");
  cmd_append(&cmd, "-o", BUILD_DIR "pixel_sorter");
  cmd_append(&cmd, "-lm", "-O3", "-march=native", "-flto=auto");
  cmd_append(&cmd, SRC_DIR "pixel_sorter.c", SRC_DIR "image.c", SRC_DIR "color.c", SRC_DIR "sorting.c");
  cmd_append(&cmd, BUILD_DIR "stb/linux/stb_image.o", BUILD_DIR "stb/linux/stb_image_write.o", BUILD_DIR "stb/linux/stb_image_resize.o"); if (!cmd_run(&cmd)) return 1;

  cmd_append(&cmd, BUILD_DIR "pixel_sorter");
  for (int i = 0; i < argc; ++i) cmd_append(&cmd, argv[i]);
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
