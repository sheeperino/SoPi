#include "../../nob.h"
#include "../image.h"
#include "state.h"
#include "string.h"
#include <raylib.h>
#include <stdio.h>

static float last_resized;
static bool needs_resize;

#define update_state_entry_comp(state, old, new, type)\
  do {\
    if (*(type *)(old) != *(type *)(new)) {\
      *(type *)(old) = *(type *)(new);\
      state_image_update((state));\
      return;\
    }\
  } while (0)

#define update_state_entry_change(state, changed)\
  do {\
    if ((changed)) {\
      (changed) = false;\
      state_image_update((state));\
      return;\
    }\
  } while (0)

void state_image_update(State *s) {
  image_sort(s->resized_img.data, s->resized_img.width, s->resized_img.height,
             s->gay, s->mask_only, s->no_mask, s->inv_mask, enum_to_thresh_func(s->t_sort_by));
  UpdateTexture(s->tex, s->resized_img.data);
  s->resized_img = ImageCopy(s->orig_resized_img);
}

void state_dialog_init(State *s) {
  s->dialog = InitGuiWindowFileDialog(GetWorkingDirectory());
  s->dialog.supportDrag = false;
  s->dialog.windowBounds = (Rectangle){0, 0, s->img_area_w, s->img_area_h};
  strcpy(s->dialog.filterExt, "DIR;.png;.bmp;.tga;.gif;.jpg;.jpeg;.psd;.hdr;.qoi;.dds;.pkm;.ktx;.pvr;.astc");
}

void state_main_update(State *s) {
  if (s->app_state == STATE_MAIN) {
    update_state_entry_comp(s, &sort_direction, &s->sort_dir_drop, int);
    if (s->t_sort_by != s->t_sort_by_drop) s->thresholds = get_threshold_bounds(s->t_sort_by_drop);
    update_state_entry_comp(s, &s->t_sort_by, &s->t_sort_by_drop, int);
    update_state_entry_change(s, s->min_changed);
    update_state_entry_change(s, s->max_changed);
    update_state_entry_change(s, s->gay_changed);
    update_state_entry_change(s, s->mask_only_changed);
    update_state_entry_change(s, s->no_mask_changed);
    update_state_entry_change(s, s->inv_mask_changed);
  } else if (s->app_state == STATE_PICK_IMAGE) {
    s->dialog.windowActive = true;
    if (s->dialog.SelectFilePressed) {
      state_image_load(s, nob_temp_sprintf("%s/%s", s->dialog.dirPathText, s->dialog.fileNameText));
      s->dialog.windowActive = false;
      s->dialog.SelectFilePressed = false;
    } else if (s->dialog.CancelFilePressed) {
      s->dialog.windowActive = false;
      s->dialog.CancelFilePressed = false;
      if (s->orig_img.data) s->app_state = STATE_MAIN;
      else s->app_state = STATE_NO_IMAGE;
    }
  } else if (s->app_state == STATE_SAVE_IMAGE) {
    s->dialog.windowActive = true;
    s->dialog.saveFileMode = true;
    if (s->dialog.SelectFilePressed) {
      const char *save_path = nob_temp_sprintf("%s/%s", s->dialog.dirPathText, s->dialog.fileNameText);
      printf("path = %s\n", save_path);
      if (!state_image_write(s, save_path)) printf("couldn't save image\n");
      s->dialog.windowActive = false;
      s->dialog.SelectFilePressed = false;
    } else if (s->dialog.CancelFilePressed) {
      s->dialog.windowActive = false;
      s->dialog.saveFileMode = false;
      s->dialog.CancelFilePressed = false;
      s->app_state = STATE_MAIN;
    }
  }
}

inline static float calc_resize_factor(State *s) {
  return (s->width_ratio > s->height_ratio ? s->height_ratio : s->width_ratio);
}

int state_image_load(State *s, const char *path) {
  if (!image_load(path)) return 0;

  Image orig_img = {0};
  orig_img.data = Data;
  orig_img.width = X;
  orig_img.height = Y;
  orig_img.mipmaps = 1;
  orig_img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  s->orig_img = orig_img;
  if (!IsImageValid(orig_img)) return 0;

  s->width_ratio = s->img_area_w/(float)X;
  s->height_ratio = s->img_area_h/(float)Y;

  s->orig_resized_img = ImageCopy(s->orig_img);
  s->rfact = calc_resize_factor(s);
  // dont resize when images are smaller than window size
  if (s->rfact < 1.0)
    image_resize_fact(s->orig_resized_img.data, s->rfact, &s->orig_resized_img.width, &s->orig_resized_img.height);

  s->resized_img = ImageCopy(s->orig_resized_img);
  s->tex = LoadTextureFromImage(s->resized_img);

  s->app_state = STATE_MAIN;
  state_image_update(s);
  return 1;
}

int state_image_write(State *s, const char *path) {
  int size = s->orig_img.width*s->orig_img.height*CHANNELS;
  uint8_t *orig_but_sorted = malloc(size);
  memcpy(orig_but_sorted, s->orig_img.data, size);

  image_sort(orig_but_sorted, s->orig_img.width, s->orig_img.height,
             s->gay, s->mask_only, s->no_mask, s->inv_mask, enum_to_thresh_func(s->t_sort_by));
  int ret = image_write(orig_but_sorted, s->orig_img.width, s->orig_img.height, path);
  free(orig_but_sorted);

  s->app_state = STATE_MAIN;
  return ret;
}

static void state_image_free(State *s) {
  image_free();
  if (s->orig_resized_img.data) UnloadImage(s->orig_resized_img);
  if (s->resized_img.data) UnloadImage(s->resized_img);
  UnloadTexture(s->tex);
}

void state_handle_file_drops(State *s) {
  if (IsFileDropped()) {
    FilePathList paths = LoadDroppedFiles();

    state_free(s);
    state_image_load(s, paths.paths[0]);

    UnloadDroppedFiles(paths);
    s->app_state = STATE_MAIN;
  }
}

void state_handle_resize(State *s) {
  float time = GetTime();
  if (IsWindowResized()) {
    last_resized = time;
    needs_resize = true;
    s->img_area_w = GetScreenWidth() - s->sidewidth;
    s->img_area_h = GetScreenHeight();
    s->width_ratio = s->img_area_w/(float)X;
    s->height_ratio = s->img_area_h/(float)Y;
    s->dialog.windowBounds = (Rectangle){0, 0, s->img_area_w, s->img_area_h};
  }
  if (s->app_state == STATE_MAIN && needs_resize && time - last_resized > 0.05) {
    needs_resize = false;

    float new_fact = (s->width_ratio > s->height_ratio ? s->height_ratio : s->width_ratio);
    if (s->app_state == STATE_MAIN && (s->rfact < 1.0 || new_fact < 1.0) && s->rfact >= 0.0 && new_fact >= 0.0) {
      if (s->rfact < 1.0 && new_fact >= 1.0) s->rfact = 1.0; // temp
      else s->rfact = new_fact;

      s->orig_resized_img = ImageCopy(s->orig_img);
      image_resize_fact(s->orig_resized_img.data, s->rfact, &s->orig_resized_img.width, &s->orig_resized_img.height);

      s->resized_img = ImageCopy(s->orig_resized_img);
      image_sort(s->resized_img.data, s->resized_img.width, s->resized_img.height,
                 s->gay, s->mask_only, s->no_mask, s->inv_mask, enum_to_thresh_func(s->t_sort_by));

      UnloadTexture(s->tex);
      s->tex = LoadTextureFromImage(s->resized_img);

      printf("[%f] resized_window, took: %f\n", GetTime(), GetTime() - last_resized);
      printf("new width = %d, new height = %d\n", s->resized_img.width, s->resized_img.height);
    }
    state_image_update(s);
    s->rfact = (new_fact > 0.0 ? new_fact : 0.0); // clamp
  }
}

void state_free(State *s) {
  state_image_free(s);
}

