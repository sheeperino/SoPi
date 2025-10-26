#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#define SRC_DIR   "src/"
#define BUILD_DIR "build/"
#define RAYLIB_SRC "./external/raylib-5.5/src/"

static const char *sources[] = {
  SRC_DIR "image.c",
  SRC_DIR "color.c",
  SRC_DIR "sorting.c",
  SRC_DIR "gui/gui.c",
  SRC_DIR "gui/state.c",
  SRC_DIR "gui/draw.c",
};

Cmd cmd = {0};
Procs procs = {0};
static const char *raylib_modules[] = {
  "rcore",
  "raudio",
  "rglfw",
  "rmodels",
  "rshapes",
  "rtext",
  "rtextures",
  "utils",
};

#include "./platform/build_linux.c"
#include "./platform/build_win.c"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  shift(argv, argc);

  if (argc > 0) {
    const char *arg = shift(argv, argc);
    if (strcmp(arg, "clean") == 0) {
      if (file_exists(BUILD_DIR)) {
        cmd_append(&cmd, "rm", "-rf", BUILD_DIR "raylib");
        if (!cmd_run(&cmd)) return 1;
      }
    }
  }

  if (!mkdir_if_not_exists(BUILD_DIR)) return 1;

  if (!build_raylib_linux()) return 1;
  if (!build_raylib_windows()) return 1;
  procs_flush(&procs);

  cmd_append(&cmd, "cc", "-Wall", "-Wextra");
  cmd_append(&cmd, "-O3", "-flto=auto", "-march=native");
  for (size_t i = 0; i < ARRAY_LEN(sources); ++i) cmd_append(&cmd, sources[i]);
  cmd_append(&cmd, BUILD_DIR "stb/linux/stb_image.o", BUILD_DIR "stb/linux/stb_image_write.o", BUILD_DIR "stb/linux/stb_image_resize.o");
  cmd_append(&cmd, "-L./build/raylib/linux", "-l:libraylib.a");
  cmd_append(&cmd, "-I" RAYLIB_SRC);
  cmd_append(&cmd, "-o", BUILD_DIR "gui");
  cmd_append(&cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");
  if (!cmd_run(&cmd, .async = &procs)) return 1;

  cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-mwindows", "-Wall", "-Wextra");
  cmd_append(&cmd, "-O3", "-flto=auto");
  for (size_t i = 0; i < ARRAY_LEN(sources); ++i) cmd_append(&cmd, sources[i]);
  cmd_append(&cmd, BUILD_DIR "stb/windows/stb_image.o", BUILD_DIR "stb/windows/stb_image_write.o", BUILD_DIR "stb/windows/stb_image_resize.o");
  cmd_append(&cmd, "-L./build/raylib/windows", "-l:libraylib.a");
  cmd_append(&cmd, "-I" RAYLIB_SRC);
  cmd_append(&cmd, "-o", BUILD_DIR "gui");
  cmd_append(&cmd, "-lwinmm", "-lgdi32", "-lole32");
  cmd_append(&cmd, "-static");
  if (!cmd_run(&cmd, .async = &procs)) return 1;

  if (!procs_flush(&procs)) return 1;

  cmd_append(&cmd, BUILD_DIR "gui");
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
