#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR   "src/"

Cmd cmd = {0};
Procs procs = {0};

int build_object(const char *obj, const char *file, const char *impl) {
  if (needs_rebuild1(obj, file)) {
  	cmd_append(&cmd, "cc", "-Wextra", "-Wall", "-std=c99");
  	cmd_append(&cmd, "-O3", "-flto=auto");
  	cmd_append(&cmd, "-x", "c", "-c", file, impl);
  	cmd_append(&cmd, "-o", obj);
    return cmd_run(&cmd, .async = &procs);
  }
  return 1;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  if (!mkdir_if_not_exists(BUILD_DIR)) return 1;
  shift(argv, argc);

  if (!build_object(BUILD_DIR "stb_image.o", SRC_DIR "stb_image.h", "-DSTB_IMAGE_IMPLEMENTATION")) return 1;
  if (!build_object(BUILD_DIR "stb_image_write.o", SRC_DIR "stb_image_write.h", "-DSTB_IMAGE_WRITE_IMPLEMENTATION")) return 1;
  if (!build_object(BUILD_DIR "stb_image_resize.o", SRC_DIR "stb_image_resize2.h", "-DSTB_IMAGE_RESIZE_IMPLEMENTATION")) return 1;
  if (!procs_flush(&procs)) return 1;

	// cc -std=c99 -o sort sort.c stb_image.o stb_image_write.o -lm -O3 -march=native
	cmd_append(&cmd, "cc", "-Wextra", "-Wall"/* , "-std=c99" */);
	cmd_append(&cmd, "-o", BUILD_DIR "pixel_sorter");
	cmd_append(&cmd, "-lm", "-O3", "-march=native", "-flto=auto");
	cmd_append(&cmd, SRC_DIR "sort.c", BUILD_DIR "stb_image.o", BUILD_DIR "stb_image_write.o", BUILD_DIR "stb_image_resize.o");
	if (!cmd_run(&cmd)) return 1;

	cmd_append(&cmd, BUILD_DIR "pixel_sorter");
	for (int i = 0; i < argc; ++i) cmd_append(&cmd, argv[i]);
	if (!cmd_run(&cmd)) return 1;

  return 0;
}
