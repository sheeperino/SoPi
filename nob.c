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
bool lin = false;
bool win = false;
const char *dist_tag = NULL;

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

int clean_build() {
  if (file_exists(BUILD_DIR)) {
    cmd_append(&cmd, "rm", "-rf", BUILD_DIR);
    if (!cmd_run(&cmd)) return 0;
  }
  return 1;
}

int is_dist_valid() {
  cmd_append(&cmd, "gh", "release", "view", dist_tag); // verify provided tag is valid
  if (!cmd_run(&cmd)) return 0;
  return 1;
}

// NOTE: this doesnt directly publish the release, it assumes the release is a draft
// (note to self) for publishing: gh release edit <tag> --draft=false
int dist_assets(bool upload) {
  if (!mkdir_if_not_exists("dist/")) return 0;
  const char *linux_zip = temp_sprintf("dist/sopi-%s-linux.zip", dist_tag);

  cmd_append(&cmd, "7z", "a", linux_zip, BUILD_DIR "sopi");
  if (!cmd_run(&cmd)) return 0;
  cmd_append(&cmd, "7z", "rn", linux_zip, BUILD_DIR, temp_sprintf("sopi-%s-linux/", dist_tag));
  if (!cmd_run(&cmd)) return 0;

  const char *win_zip = temp_sprintf("dist/sopi-%s-windows.zip", dist_tag);
  cmd_append(&cmd, "7z", "a", win_zip, BUILD_DIR "sopi.exe");
  if (!cmd_run(&cmd)) return 0;
  cmd_append(&cmd, "7z", "rn", win_zip, BUILD_DIR, temp_sprintf("sopi-%s-windows/", dist_tag));
  if (!cmd_run(&cmd)) return 0;

  if (upload) {
    cmd_append(&cmd, "gh", "release", "upload", dist_tag, "dist/*", "--clobber");
    if (!cmd_run(&cmd)) return 0;
  }
  return 1;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "platform/build_linux.c", "platform/build_win.c");
  shift(argv, argc);

  // NOTE: use -- if you want to pass arguments to the program instead of to nob
  while (argc > 0) {
    const char *arg = shift(argv, argc);
    if (strcmp(arg, "--") == 0) break;
    if (strcmp(arg, "clean") == 0) {
      if (!clean_build()) return 1;
    } else if (strcmp(arg, "cli_only") == 0) {
      cli_only = true;
    } else if (strcmp(arg, "linux") == 0) {
      lin = true;
    } else if (strcmp(arg, "windows") == 0) {
      win = true;
    } else if (strcmp(arg, "all") == 0) {
      lin = true;
      win = true;
    } else if (strcmp(arg, "dist") == 0) {
      dist_tag = shift(argv, argc);
      if (!is_dist_valid()) return 1;
      if (!clean_build()) return 1;
      lin = true;
      win = true;
    }
  }

  // automatically pick a platform to build on if not specified
  if (!lin && !win) {
    printf("provided neither windows nor linux\n");
    #ifndef _WIN32
    lin = true;
    #else
    win = true;
    #endif
  }

  if (!mkdir_if_not_exists(BUILD_DIR)) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "stb/")) return 1;
  if (lin && !mkdir_if_not_exists(BUILD_DIR "stb/linux/")) return 1;
  if (win && !mkdir_if_not_exists(BUILD_DIR "stb/windows/")) return 1;
  if (!mkdir_if_not_exists(BUILD_DIR "raygui/")) return 1;
  if (lin && !mkdir_if_not_exists(BUILD_DIR "raygui/linux/")) return 1;
  if (win && !mkdir_if_not_exists(BUILD_DIR "raygui/windows/")) return 1;

  if (!cli_only) {
    if (lin && !build_raylib_linux()) return 1;
    if (win && !build_raylib_windows()) return 1;
    procs_flush(&procs);
  }

  // stb
  if (lin) {
    build_object_linux(BUILD_DIR "stb/linux/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION");
    build_object_linux(BUILD_DIR "stb/linux/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION");
    build_object_linux(BUILD_DIR "stb/linux/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION");
  }
  if (win) {
    build_object_windows(BUILD_DIR "stb/windows/stb_image.o", EXT_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION");
    build_object_windows(BUILD_DIR "stb/windows/stb_image_write.o", EXT_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION");
    build_object_windows(BUILD_DIR "stb/windows/stb_image_resize.o", EXT_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION");
  }

  if (!procs_flush(&procs)) return 1;

  if (lin) {
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
  }
  if (win) {
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
      cmd_append(&cmd, "-lwinmm", "-lgdi32", "-lole32");
      cmd_append(&cmd, "-static");
    }
    if (!cmd_run(&cmd, .async = &procs)) return 1;
  }

  if (!procs_flush(&procs)) return 1;

  // generate dist assets
  if (dist_tag != NULL) {
    if (!dist_assets(true)) return 1;
  }

  const char *exe;
  if (lin && win) {
    #ifndef _WIN32
    exe = "sopi";
    #else
    exe = "sopi.exe";
    #endif
  } else exe = (lin ? "sopi" : "sopi.exe");
  cmd_append(&cmd, temp_sprintf(BUILD_DIR "%s", exe));
  for (int i = 0; i < argc; ++i) cmd_append(&cmd, argv[i]);
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
