# Browser emulator (128×128 TFT)

WebAssembly build of the Case A scene animations from `firmware/src/`. Lets you
preview and iterate on animations in a browser without flashing the ESP32.

Animations run as **firmware C++**, compiled to WebAssembly. JavaScript loads
the module and blits the framebuffer to a canvas at 3× scale. Brightness and
speed (0.25–8×) are UI-only multipliers.

## Animations

Six upstream scenes (Face, Fisherman, Runner, Sisyphus, Balloon, Stargazer) plus
local extras: Campfire, Owl, Juggler.

## Prerequisites

| Tool | Required for | Install |
|------|----------------|---------|
| [Emscripten](https://emscripten.org/) | Building WASM | `brew install emscripten` |
| Python 3 | HTTP server for local preview | usually preinstalled |

## Run

From this directory (`emulator/`):

```bash
./scripts/build-wasm.sh
python3 -m http.server 8080
```

Open http://localhost:8080

## Layout

```
emulator/
  firmware/     local extras + shared helpers (upstream anim in ../firmware/src)
  native/       GFX shim + Emscripten glue
  scripts/      build-wasm.sh
  wasm/         build output (gitignored)
  js/           WASM loader, render loop, UI
```

See the root README for the full phone-case project (firmware, iOS app, hardware).
