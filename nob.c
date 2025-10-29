#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR   "src/"
#define EXT_DIR   "external/"
#define RAYLIB_SRC "./external/raylib-5.5/src/"

Cmd cmd = {0};
Procs procs = {0};
bool cli_only = false;

static const char *cli_sources[] = {
  SRC_DIR "sopi.c", SRC_DIR "image.c", SRC_DIR "color.c", SRC_DIR "sorting.c",
};
static const char *gui_sources[] = {
  SRC_DIR "gui/gui.c", SRC_DIR "gui/state.c", SRC_DIR "gui/draw.c",
};

static const char *raylib_modules[] = {
  "rcore", "raudio", "rglfw", "rmodels",
  "rshapes", "rtext", "rtextures", "utils",
};

#include "./platform/build_linux.c"
#include "./platform/build_win.c"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  shift(argv, argc);

  // NOTE: use -- if you want to pass arguments to the program instead of to nob
  for (int i = 0; i < argc; ++i) {
    const char *arg = shift(argv, argc);
    if (strcmp(arg, "--") == 0) break;
    if (strcmp(arg, "clean") == 0) {
      if (file_exists(BUILD_DIR)) {
        cmd_append(&cmd, "rm", "-rf", BUILD_DIR);
        if (!cmd_run(&cmd)) return 1;
      }
    } else if (strcmp(arg, "cli_only") == 0) {
      cli_only = true;
    }
  }

  if (!mkdir_if_not_exists(BUILD_DIR)) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/linux/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/windows/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "raygui/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "raygui/linux/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "raygui/windows/")) return 1;

  if (!cli_only) {
    if (!build_raylib_linux()) return 1;
    if (!build_raylib_windows()) return 1;
    procs_flush(&procs);
  }

  // stb
  build_object_linux(BUILD_DIR "stb/linux/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION");
  build_object_linux(BUILD_DIR "stb/linux/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION");
  build_object_linux(BUILD_DIR "stb/linux/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION");

  build_object_windows(BUILD_DIR "stb/windows/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION");
  build_object_windows(BUILD_DIR "stb/windows/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION");
  build_object_windows(BUILD_DIR "stb/windows/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION");

  if (!procs_flush(&procs)) return 1;

  // linux build
  cmd_append(&cmd, "cc", "-Wextra", "-Wall", "-ggdb");
  cmd_append(&cmd, "-o", BUILD_DIR "sopi");
  cmd_append(&cmd, "-lm", "-O3", "-march=native", "-flto=auto");
  for (size_t i = 0; i < ARRAY_LEN(cli_sources); ++i) cmd_append(&cmd, cli_sources[i]);
  cmd_append(&cmd, BUILD_DIR "stb/linux/stb_image.o", BUILD_DIR "stb/linux/stb_image_write.o", BUILD_DIR "stb/linux/stb_image_resize.o");
  if (cli_only) cmd_append(&cmd, "-DCLI_ONLY");
  if (!cli_only) {
    for (size_t i = 0; i < ARRAY_LEN(gui_sources); ++i) cmd_append(&cmd, gui_sources[i]);
    cmd_append(&cmd, "-L./build/raylib/linux", "-l:libraylib.a");
    cmd_append(&cmd, "-I" RAYLIB_SRC);
    cmd_append(&cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");
  }
  if (!cmd_run(&cmd, .async = &procs)) return 1;

  // windows build
  cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-mwindows", "-Wall", "-Wextra");
  cmd_append(&cmd, "-o", BUILD_DIR "sopi");
  cmd_append(&cmd, "-lm", "-O3", "-flto=auto");
  for (size_t i = 0; i < ARRAY_LEN(cli_sources); ++i) cmd_append(&cmd, cli_sources[i]);
  cmd_append(&cmd, BUILD_DIR "stb/windows/stb_image.o", BUILD_DIR "stb/windows/stb_image_write.o", BUILD_DIR "stb/windows/stb_image_resize.o");
  if (cli_only) cmd_append(&cmd, "-DCLI_ONLY");
  if (!cli_only) {
    for (size_t i = 0; i < ARRAY_LEN(gui_sources); ++i) cmd_append(&cmd, gui_sources[i]);
    cmd_append(&cmd, "-L./build/raylib/windows", "-l:libraylib.a");
    cmd_append(&cmd, "-I" RAYLIB_SRC);
    cmd_append(&cmd, "-o", BUILD_DIR "gui");
    cmd_append(&cmd, "-lwinmm", "-lgdi32", "-lole32");
    cmd_append(&cmd, "-static");
  }
  if (!cmd_run(&cmd, .async = &procs)) return 1;

  if (!procs_flush(&procs)) return 1;

  cmd_append(&cmd, BUILD_DIR "sopi");
  for (int i = 0; i < argc; ++i) cmd_append(&cmd, argv[i]);
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
