#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "build/"

Cmd cmd = {0};

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", BUILD_DIR "gui", "src/image.c", "src/color.c", "src/sorting.c", "src/gui/gui.c", "-lm", "-lraylib");
  cmd_append(&cmd, BUILD_DIR "stb_image.o", BUILD_DIR "stb_image_write.o", BUILD_DIR "stb_image_resize.o");
  cmd_append(&cmd, "-O3", "-flto=auto", "-march=native");
  if (!cmd_run(&cmd)) return 1;

  cmd_append(&cmd, BUILD_DIR "gui");
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
