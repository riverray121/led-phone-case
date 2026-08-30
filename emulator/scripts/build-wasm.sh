#!/usr/bin/env bash
set -euo pipefail

EMULATOR_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO_ROOT="$(cd "$EMULATOR_ROOT/.." && pwd)"
OUT="$EMULATOR_ROOT/wasm"
mkdir -p "$OUT"

EMCC="${EMCC:-em++}"
if ! command -v "$EMCC" >/dev/null 2>&1; then
  if [[ -x /opt/homebrew/bin/em++ ]]; then
    EMCC=/opt/homebrew/bin/em++
  else
    echo "em++ not found; install Emscripten (brew install emscripten)" >&2
    exit 1
  fi
fi

"$EMCC" \
  "$EMULATOR_ROOT/native/emulator.cpp" \
  "$EMULATOR_ROOT/native/Adafruit_GFX.cpp" \
  "$EMULATOR_ROOT/native/glcdfont.cpp" \
  "$REPO_ROOT/firmware/src/animations.cpp" \
  "$EMULATOR_ROOT/firmware/animation_shared.cpp" \
  "$EMULATOR_ROOT/firmware/animations_extra.cpp" \
  -I"$EMULATOR_ROOT/native" \
  -I"$REPO_ROOT/firmware/src" \
  -I"$EMULATOR_ROOT/firmware" \
  -O2 \
  -std=c++17 \
  -s WASM=1 \
  -s DEFAULT_TO_CXX=1 \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s EXPORT_NAME=createAnimationsModule \
  -s ENVIRONMENT=web \
  -s ALLOW_MEMORY_GROWTH=0 \
  -s EXPORTED_FUNCTIONS='["_animation_count","_animation_name","_render_frame","_framebuffer","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","HEAPU8"]' \
  -o "$OUT/animations.js"

echo "Built $OUT/animations.js and $OUT/animations.wasm"
