#ifndef GUI_STATE_H_
#define GUI_STATE_H_

#include "raylib.h"
#include "../sorting.h"

#include "gui_window_file_dialog.h"

typedef enum {
  STATE_NO_IMAGE,
  STATE_MAIN,
  STATE_PICK_IMAGE,
  STATE_SAVE_IMAGE,
} AppState;

typedef struct {
  AppState app_state;
  GuiWindowFileDialogState dialog;
  Camera2D cam; // for panning/zoom

  float rfact, width_ratio, height_ratio;
  int img_area_w, img_area_h;
  int sidewidth;

  Image orig_img, orig_resized_img, resized_img;
  Texture2D tex;

  ThresholdBounds thresholds;
  int sort_dir_drop; bool sort_dir_drop_show;
  ThresholdSortBy t_sort_by; int t_sort_by_drop; bool t_sort_by_drop_show;

  int min; bool min_changed;
  int max; bool max_changed;
  bool gay, gay_changed;
  bool mask_only, mask_only_changed;
  bool no_mask, no_mask_changed;
  bool inv_mask, inv_mask_changed;
} State;

void state_dialog_init(State *s);

int state_image_load(State *s, const char *path);
int state_image_write(State *s, const char *path);
void state_image_update(State *s);
void state_main_update(State *state);

void state_handle_pan_and_zoom(State *s);
void state_handle_file_drops(State *s);
void state_handle_resize(State *s);

void state_free(State *s);

#endif // GUI_STATE_H_
