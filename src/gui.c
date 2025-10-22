#include <raylib.h>
#include <rlgl.h>
#include "image.h"
#include "sorting.h"
#include <math.h>
#include <stdio.h>
// #include <stdlib.h>

int main() {
  InitWindow(800, 600, "sopi gui");
  image_load("img/pap.png");
  SetTargetFPS(60);
  SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  MIN = 100; MAX = 250;
  printf("MIN = %zu\n", MIN);
  sort_direction = RIGHT;

  Texture2D tex = {0};
  tex.id = rlLoadTexture(data, x, y, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
  tex.width = x;
  tex.height = y;
  tex.mipmaps = 1;
  tex.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  float prev_wr = 0, prev_hr = 0;

  // image_resize_fact(0.5);

  // printf("wr = %f, hr = %f\n", wr, hr);

  while (!WindowShouldClose()) {
    float wr = GetScreenWidth() / (float)x;
    float hr = GetScreenHeight() / (float)y;

    // if (prev_wr != wr)
    MIN = cos(GetTime()) * 50 + 100;
    image_sort(false, false, false, false, by_value);
    printf("MIN = %zu\n", MIN);
    UpdateTexture(tex, data);
    image_reset();
    BeginDrawing();
      ClearBackground(BLACK);
      DrawTextureEx(tex, (Vector2){0}, 0, 0.5, WHITE);
      DrawFPS(10, 10);
    EndDrawing();
  }

  image_free();
  UnloadTexture(tex);
  CloseWindow();
  return 0;
}
