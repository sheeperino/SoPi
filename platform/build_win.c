#ifdef _WIN32
#define MAYBE_PREFIXED(x) x
#else
#define MAYBE_PREFIXED(x) "x86_64-w64-mingw32-"x
#endif // _WIN32

int build_object_windows(const char *obj, const char *file, const char *impl) {
  if (needs_rebuild1(obj, file)) {
    cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-Wextra", "-Wall", "-std=c99");
    cmd_append(&cmd, "-O3", "-march=native");
    cmd_append(&cmd, "-x", "c", "-c", file, impl);
    cmd_append(&cmd, "-o", obj);
    return cmd_run(&cmd, .async = &procs);
  }
  return 1;
}

// credits: musializer
int build_raylib_windows() {
    Cmd cmd = {0};
    File_Paths object_files = {0};

    if (!mkdir_if_not_exists("./build/raylib")) return 0;

    Procs obj_procs = {0};

    const char *build_path = temp_sprintf("./build/raylib/%s", "windows");
    if (!mkdir_if_not_exists(build_path)) return 0;

    // build object files
    for (size_t i = 0; i < ARRAY_LEN(raylib_modules); ++i) {
      const char *input_path = temp_sprintf(RAYLIB_SRC "%s.c", raylib_modules[i]);
      const char *output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
      output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);

      da_append(&object_files, output_path);

      if (needs_rebuild(output_path, &input_path, 1)) {
        cmd_append(&cmd, "x86_64-w64-mingw32-gcc",
          "-ggdb", "-DPLATFORM_DESKTOP", "-fPIC",
          "-O3",
          "-fPIC",
          "-I" RAYLIB_SRC "external/glfw/include",
          "-c", input_path,
          "-o", output_path);
        if (!cmd_run(&cmd, .async = &obj_procs)) return 0;
      }
    }

  if (!procs_flush(&obj_procs)) return 0;

  // build static library
  const char *libraylib_path = temp_sprintf("%s/libraylib.a", build_path);
  if (needs_rebuild(libraylib_path, object_files.items, object_files.count)) {
    cmd_append(&cmd, MAYBE_PREFIXED("ar"));
    cmd_append(&cmd, "-crs", libraylib_path);
    for (size_t i = 0; i < ARRAY_LEN(raylib_modules); ++i) {
      const char *input_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
      cmd_append(&cmd, input_path);
    }
    if (!cmd_run(&cmd, .async = &procs)) return 0;
  }

  return 1;
}
