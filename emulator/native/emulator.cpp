#include <emscripten.h>

#include "Adafruit_GFX.h"
#include "animations.h"
#include "animations_extra.h"

constexpr int UPSTREAM_SCENE_COUNT = 6;
constexpr int SCENE_COUNT = 9;

static GFXcanvas16 canvas(128, 128);
static Animation *mergedScenes[SCENE_COUNT];
static bool scenesReady = false;

static void ensureScenes() {
  if (scenesReady) return;

  int upstreamCount = 0;
  Animation **upstream = animationList(upstreamCount);
  for (int i = 0; i < UPSTREAM_SCENE_COUNT; i++) mergedScenes[i] = upstream[i];

  int extraCount = 0;
  Animation **extra = extraAnimationList(extraCount);
  for (int i = 0; i < extraCount; i++) mergedScenes[UPSTREAM_SCENE_COUNT + i] = extra[i];

  scenesReady = true;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int animation_count() {
  ensureScenes();
  return SCENE_COUNT;
}

EMSCRIPTEN_KEEPALIVE
const char *animation_name(int index) {
  ensureScenes();
  if (index < 0 || index >= SCENE_COUNT) return "";
  return mergedScenes[index]->name();
}

EMSCRIPTEN_KEEPALIVE
void render_frame(int index, uint32_t ms) {
  ensureScenes();
  if (index < 0 || index >= SCENE_COUNT) return;
  canvas.fillScreen(0x0000);
  mergedScenes[index]->frame(canvas, ms);
}

EMSCRIPTEN_KEEPALIVE
uint16_t *framebuffer() { return canvas.getBuffer(); }

} // extern "C"
