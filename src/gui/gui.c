#include "draw.h"
#include "state.h"
#include "../../nob.h"
#include "raylib.h"
#include "../image.h"
#include "../sorting.h"
#include "rlgl.h"

int sopi_gui() {
  InitWindow(1280, 720, "sopi gui");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);
  SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  draw_init_raygui();

  int sidewidth = 250;
  State S = {
    .app_state = STATE_NO_IMAGE,
    .rfact = 1.0,
    .img_area_w = GetScreenWidth() - sidewidth,
    .img_area_h = GetScreenHeight(),
    .sidewidth = sidewidth,
    .min_changed = true, .max_changed = true,
    .sort_dir_drop = sort_direction,
    .t_sort_by = BY_VALUE, .t_sort_by_drop = S.t_sort_by,
    0,
  };
  S.thresholds = get_threshold_bounds(S.t_sort_by);
  if (Data) {
    if (!state_image_load_from_memory(&S, Data, X, Y)) return 1;
  }
  S.cam = (Camera2D){{S.img_area_w/2.0, S.img_area_h/2.0}, {S.img_area_w/2.0, S.img_area_h/2.0}, 0, 1.0};
  state_dialog_init(&S);

  bool quit = false;
  while (!WindowShouldClose() && !quit) {
    state_main_update(&S);
    state_handle_keybindings(&S, &quit);
    state_handle_pan_and_zoom(&S);
    state_handle_resize(&S);
    state_handle_file_drops(&S);

    BeginDrawing();
      draw_main_gui(&S);
    EndDrawing();
  }

  state_free(&S);
  CloseWindow();
  return 0;
}
