// Animation engine. Each animation draws one frame into the canvas given the
// time since it was selected; it must not depend on frame rate or hardware.
#pragma once

#include <Adafruit_GFX.h>

struct Animation {
    virtual ~Animation() = default;
    virtual const char *name() const = 0;
    virtual void frame(GFXcanvas16 &c, uint32_t ms) = 0;
};

Animation **animationList(int &count);
