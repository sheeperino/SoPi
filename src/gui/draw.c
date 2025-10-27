#include "../../nob.h"
#include "state.h"
#define RAYGUI_IMPLEMENTATION
#include "../../external/raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"
#include "draw.h"
#include "style_amber.h"

static Color bg_color;
static Color text_color;
static int default_text_size;

void draw_init_raygui() {
  GuiLoadStyleAmber();
  default_text_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
  bg_color = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
  text_color = GetColor(GuiGetStyle(LABEL, TEXT));
  GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
  GuiSetStyle(DROPDOWNBOX, TEXT_PADDING, 4);
}

// returns whether the value has changed
bool draw_slider_with_value_box(Rectangle box_rect, Rectangle slider_rect, int min, int max, int *value) {
  float ftemp = *value;
  int itemp = *value;
  bool box_active = CheckCollisionPointRec(GetMousePosition(), box_rect);

  char min_str[4], max_str[4];
  snprintf(min_str, 4, "%d", min);
  snprintf(max_str, 4, "%d", max);

  GuiSlider(slider_rect, min_str, max_str, &ftemp, min, max);
  GuiValueBox(box_rect, NULL, &itemp, min, max, box_active);

  if (itemp != *value) {
    *value = itemp;
    return true;
  }
  if ((int)ftemp != *value) {
    *value = (int)ftemp;
    return true;
  }
  return false;
}

void draw_main_gui(State *s) {
  ClearBackground(bg_color);
  if (s->app_state == STATE_MAIN) {
    GuiSetState(STATE_NORMAL);
    GuiGrid((Rectangle){0, 0, s->img_area_w, GetScreenHeight()}, NULL, 25, 1, NULL);
    draw_image(s);

    if (s->sort_dir_drop_show || s->t_sort_by_drop_show) GuiLock();
    else GuiUnlock();
  } else if (s->app_state == STATE_NO_IMAGE) {
    GuiSetStyle(DEFAULT, TEXT_SIZE, default_text_size*2);
    GuiDrawText("Open or Drag & Drop an image to get started.", (Rectangle){0, 0, s->img_area_w, s->img_area_h}, TEXT_ALIGN_CENTER, WHITE);
    GuiSetStyle(DEFAULT, TEXT_SIZE, default_text_size);
    GuiSetState(STATE_NORMAL);
  } else if (s->app_state == STATE_PICK_IMAGE) {
    GuiSetState(STATE_NORMAL);
    GuiWindowFileDialog(&s->dialog);
  } else if (s->app_state == STATE_SAVE_IMAGE) {
    GuiSetState(STATE_NORMAL);
    GuiWindowFileDialog(&s->dialog);
  }

  draw_sidebar(s, (Rectangle){GetScreenWidth() - s->sidewidth, 0, s->sidewidth, GetScreenHeight()});
}

void draw_image(State *s) {
  if (s->rfact <= 1.0)
    DrawTextureEx(s->tex, (Vector2){
                  (s->img_area_w - s->resized_img.width)/2.0,
                  (s->img_area_h - s->resized_img.height)/2.0}, 0, 1, WHITE);
  else if (s->rfact >= 0.0)
    DrawTextureEx(s->tex, (Vector2){
                  (s->img_area_w - s->rfact*s->orig_img.width)/2.0,
                  (s->img_area_h - s->rfact*s->orig_img.height)/2.0}, 0, s->rfact, WHITE);
}

void draw_sidebar(State *s, Rectangle r) {
  int sidepad = 40;
  DrawRectangle(r.x, 0, r.width, GetScreenHeight(), bg_color);

  // open and save image buttons
  if (s->app_state == STATE_PICK_IMAGE || s->app_state == STATE_SAVE_IMAGE) GuiSetState(STATE_DISABLED);
  if (GuiButton((Rectangle){r.x + 10, 250, r.width - (sidepad - 10), 25}, "Open image..."))
    s->app_state = STATE_PICK_IMAGE;

  if (s->app_state == STATE_NO_IMAGE) GuiSetState(STATE_DISABLED);
  if (GuiButton((Rectangle){r.x + 10, 280, r.width - (sidepad - 10), 25}, "Save image..."))
    s->app_state = STATE_SAVE_IMAGE;

  // thresholds
  GuiDrawText("Min:", (Rectangle){r.x + 10, 5, r.width, 25}, 0, text_color);
  s->min_changed = draw_slider_with_value_box(
    (Rectangle){r.x + sidepad, 5, 35, 25}, (Rectangle){r.x + sidepad + 55, 5, 100, 25},
    0, 255, &MIN);
  GuiDrawText("Max:", (Rectangle){r.x + 10, 35, r.width, 25}, 0, text_color);
  s->max_changed = draw_slider_with_value_box(
    (Rectangle){r.x + sidepad, 35, 35, 25}, (Rectangle){r.x + sidepad + 55, 35, 100, 25},
    0, 255, &MAX);

  // we draw these in reverse order to avoid overlap
  // -----------------------------------------------
  // bool flags
  s->gay_changed = GuiCheckBox((Rectangle){r.x + 10, 140, 15, 15}, "Highlight intervals (gay mode)", &s->gay);
  s->mask_only_changed = GuiCheckBox((Rectangle){r.x + 10, 165, 15, 15}, "Mask only", &s->mask_only);
  s->no_mask_changed = GuiCheckBox((Rectangle){r.x + 10, 190, 15, 15}, "No mask", &s->no_mask);
  s->inv_mask_changed = GuiCheckBox((Rectangle){r.x + 10, 215, 15, 15}, "Invert mask", &s->inv_mask);

  // sort by
  GuiDrawText("Sort by:", (Rectangle){r.x + 10, 105, r.width, 25}, 0, text_color);
  if (GuiDropdownBox((Rectangle){r.x + 20 + 100, 105, 75, 25},
                 "hue;saturation;value;red;green;blue;alpha", &s->t_sort_by_drop, s->t_sort_by_drop_show))
    s->t_sort_by_drop_show = !s->t_sort_by_drop_show;

  // sort dir
  GuiDrawText("Sort direction:", (Rectangle){r.x + 10, 70, r.width, 25}, 0, text_color);
  if (GuiDropdownBox((Rectangle){r.x + 20 + 100, 70, 75, 25},
                     "#117#up;#116#down;#115#right;#114#left", &s->sort_dir_drop, s->sort_dir_drop_show))
    s->sort_dir_drop_show = !s->sort_dir_drop_show;
  // -----------------------------------------------

  // info texts
  if (s->app_state == STATE_MAIN) {
    GuiDrawText(nob_temp_sprintf("Preview size: %dx%d", (int)(s->orig_img.width*s->rfact), (int)(s->orig_img.height*s->rfact)),
                (Rectangle){r.x + 10, GetScreenHeight() - 45, r.width, 25}, 0, text_color);
    GuiDrawText(nob_temp_sprintf("Image size: %dx%d", s->orig_img.width, s->orig_img.height), (Rectangle){r.x + 10, GetScreenHeight() - 25, r.width, 25}, 0, text_color);
  }

  // line separator
  DrawRectangle(r.x, 0, GuiGetStyle(DEFAULT, BORDER_WIDTH),
                GetScreenHeight(), GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));
}
