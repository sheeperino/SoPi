// #include "../../external/raygui.h"
#include "draw.h"
#include "state.h"
#define NOB_IMPLEMENTATION
#include "../../nob.h"
#include "raylib.h"
#include "../sorting.h"
#include "rlgl.h"

// TODO: maybe preview quality option
int main() {
  InitWindow(1280, 720, "sopi gui");
  SetTargetFPS(60);
  SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  draw_init_raygui();

  int sidewidth = 250;

  MIN = 50; MAX = 255;
  sort_direction = UP;

  State S = {
    .app_state = STATE_NO_IMAGE,
    .rfact = 1.0,
    .img_area_w = GetScreenWidth() - sidewidth,
    .img_area_h = GetScreenHeight(),
    .sidewidth = sidewidth,
    .min = 50, .max = 255,
    .min_changed = true, .max_changed = true,
    .sort_dir_drop = sort_direction,
    .t_sort_by = BY_VALUE, .t_sort_by_drop = S.t_sort_by,
    0,
  };
  state_dialog_init(&S);

  // if (!state_image_load(&S, "img/coros2.jpg")) return 1;

  float last_resized = GetTime();
  bool needs_resize = false;
  while (!WindowShouldClose()) {
    state_main_update(&S);
    state_handle_resize(&S);
    state_handle_file_drops(&S);

    BeginDrawing();
      draw_main_gui(&S);
    EndDrawing();
  }

  // image_write(S.resized_data, S.width, S.height, "test.png");
  state_free(&S);
  CloseWindow();
  return 0;
}
