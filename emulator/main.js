const FRAME_MS = 33;
const WIDTH = 128;
const HEIGHT = 128;
const PIXELS = WIDTH * HEIGHT;
const SCALE = 3;
const SPEED_MIN = 0.25;
const SPEED_MAX = 8;
const SPEED_STEP = 0.25;

const screen = document.getElementById("screen");
const ctx = screen.getContext("2d");
const image = ctx.createImageData(WIDTH, HEIGHT);
const rgba = image.data;

let mod;
let current = 0;
let animMs = 0;
let lastTick = 0;
let lastFrame = 0;
let brightness = 255;
let speed = 1;
let paused = false;
let animCount = 0;
let animNames = [];

const animList = document.getElementById("anims");
const meta = document.getElementById("meta");
const brightEl = document.getElementById("brightness");
const speedEl = document.getElementById("speed");
const fpsEl = document.getElementById("fps");

let renderFps = 30;
let fpsFrames = 0;
let fpsWindowStart = 0;

function clampSpeed(s) {
  const steps = Math.round((s - SPEED_MIN) / SPEED_STEP);
  return Math.min(SPEED_MAX, Math.max(SPEED_MIN, SPEED_MIN + steps * SPEED_STEP));
}

function setSpeed(next) {
  speed = clampSpeed(next);
  speedEl.value = String(speed);
  document.getElementById("speed-val").textContent = formatSpeed(speed);
  updateFpsLabel();
}

function formatSpeed(s) {
  return (Number.isInteger(s) ? String(s) : s.toFixed(2).replace(/\.?0+$/, "")) + "×";
}

function updateFpsLabel() {
  fpsEl.textContent = `~${Math.max(0, Math.round(renderFps * speed))} fps`;
}

function unpack565(c) {
  return [(c >> 11) << 3, ((c >> 5) & 0x3f) << 2, (c & 0x1f) << 3];
}

function selectAnim(i) {
  current = (i + animCount) % animCount;
  animMs = 0;
  lastTick = performance.now();
  paused = false;
  document.getElementById("pause").textContent = "Pause";
  for (const btn of animList.querySelectorAll("button")) {
    btn.classList.toggle("active", Number(btn.dataset.i) === current);
  }
  meta.textContent = animNames[current];
}

function buildList() {
  animList.replaceChildren();
  animNames.forEach((name, i) => {
    const btn = document.createElement("button");
    btn.type = "button";
    btn.dataset.i = String(i);
    btn.innerHTML = `<span class="idx">${String(i + 1).padStart(2, "0")}</span>${name}`;
    btn.addEventListener("click", () => selectAnim(i));
    animList.appendChild(btn);
  });
}

function applyScale() {
  const px = WIDTH * SCALE;
  screen.style.width = px + "px";
  screen.style.height = px + "px";
}

function blit() {
  const ptr = mod._framebuffer();
  const pixels16 = new Uint16Array(mod.HEAPU8.buffer, ptr, PIXELS);
  const b = brightness / 255;
  let p = 0;
  for (let i = 0; i < PIXELS; i++) {
    const [r, g, bl] = unpack565(pixels16[i]);
    rgba[p++] = (r * b) | 0;
    rgba[p++] = (g * b) | 0;
    rgba[p++] = (bl * b) | 0;
    rgba[p++] = 255;
  }
  ctx.putImageData(image, 0, 0);
}

function tick(now) {
  requestAnimationFrame(tick);
  if (!mod || now - lastFrame < FRAME_MS) return;

  if (!paused) animMs += (now - lastTick) * speed;
  lastTick = now;
  lastFrame = now;
  if (paused) return;

  mod._render_frame(current, animMs | 0);
  blit();

  fpsFrames++;
  if (!fpsWindowStart) fpsWindowStart = now;
  const elapsed = now - fpsWindowStart;
  if (elapsed >= 500) {
    renderFps = Math.round((fpsFrames * 1000) / elapsed);
    fpsFrames = 0;
    fpsWindowStart = now;
    updateFpsLabel();
  }
}

brightEl.addEventListener("input", () => {
  brightness = Number(brightEl.value);
  document.getElementById("bright-val").textContent = brightness;
});

speedEl.addEventListener("input", () => setSpeed(Number(speedEl.value)));

document.getElementById("pause").addEventListener("click", () => {
  paused = !paused;
  lastTick = performance.now();
  document.getElementById("pause").textContent = paused ? "Resume" : "Pause";
});

window.addEventListener("keydown", (e) => {
  if (!mod) return;
  if (e.key === "ArrowDown" || e.key === "j") {
    e.preventDefault();
    selectAnim(current + 1);
  } else if (e.key === "ArrowUp" || e.key === "k") {
    e.preventDefault();
    selectAnim(current - 1);
  } else if (e.key === " ") {
    e.preventDefault();
    document.getElementById("pause").click();
  } else if (e.key === "[") {
    e.preventDefault();
    setSpeed(speed - SPEED_STEP);
  } else if (e.key === "]") {
    e.preventDefault();
    setSpeed(speed + SPEED_STEP);
  } else if (e.key >= "1" && e.key <= "9") {
    selectAnim(Number(e.key) - 1);
  }
});

async function boot() {
  const createAnimationsModule = (await import("../wasm/animations.js")).default;
  mod = await createAnimationsModule();
  animCount = mod._animation_count();
  animNames = [];
  for (let i = 0; i < animCount; i++) {
    animNames.push(mod.UTF8ToString(mod._animation_name(i)));
  }
  buildList();
  selectAnim(0);
  applyScale();
  requestAnimationFrame(tick);
}

boot().catch((err) => {
  console.error(err);
  meta.textContent = "WASM load failed";
});
