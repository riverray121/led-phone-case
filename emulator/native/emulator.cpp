#include <emscripten.h>

#include "Adafruit_GFX.h"
#include "animations.h"

static GFXcanvas16 canvas(128, 128);

extern "C" {

EMSCRIPTEN_KEEPALIVE
int animation_count() {
    int count = 0;
    sceneAnimationList(count);
    return count;
}

EMSCRIPTEN_KEEPALIVE
const char *animation_name(int index) {
    int count = 0;
    Animation **scenes = sceneAnimationList(count);
    if (index < 0 || index >= count) return "";
    return scenes[index]->name();
}

EMSCRIPTEN_KEEPALIVE
void render_frame(int index, uint32_t ms) {
    int count = 0;
    Animation **scenes = sceneAnimationList(count);
    if (index < 0 || index >= count) return;
    canvas.fillScreen(0x0000);
    scenes[index]->frame(canvas, ms);
}

EMSCRIPTEN_KEEPALIVE
uint16_t *framebuffer() { return canvas.getBuffer(); }

}  // extern "C"
