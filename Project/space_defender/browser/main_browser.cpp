/**
 * Space Defender — Browser Entry Point
 * Compiled with Emscripten to game.js + game.wasm
 *
 * Demonstrates:
 *  - emscripten_set_main_loop() for the browser game loop
 *  - EM_JS macros to call Canvas 2D drawing primitives
 *  - EMSCRIPTEN_KEEPALIVE to export C++ functions to JS
 *  - Keyboard input via Emscripten callbacks
 */

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <cstring>
#include "../include/engine.h"

// ─────────────────────────────────────────────
// Global engine instance
// ─────────────────────────────────────────────
static GameConfig    gConfig;
static GameEngine*   gEngine = nullptr;

// ─────────────────────────────────────────────
// JS Canvas drawing — defined via EM_JS
// These call real HTML5 Canvas 2D methods
// ─────────────────────────────────────────────
EM_JS(void, js_clearCanvas, (), {
    var canvas = document.getElementById('gameCanvas');
    var ctx    = canvas.getContext('2d');
    ctx.fillStyle = '#0a0a1a';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw starfield (simple static stars using random seed per clear)
    ctx.fillStyle = 'rgba(255,255,255,0.5)';
    for (var i = 0; i < 80; i++) {
        var sx = ((i * 127 + 33) % canvas.width);
        var sy = ((i * 317 + 71) % canvas.height);
        ctx.fillRect(sx, sy, 1, 1);
    }
});

EM_JS(void, js_drawRect, (float x, float y, float w, float h, const char* colorPtr), {
    var canvas = document.getElementById('gameCanvas');
    var ctx    = canvas.getContext('2d');
    var color  = UTF8ToString(colorPtr);
    ctx.fillStyle = color;
    ctx.shadowBlur = 8;
    ctx.shadowColor = color;
    ctx.fillRect(x, y, w, h);
    ctx.shadowBlur = 0;
});

EM_JS(void, js_drawText, (float x, float y, const char* textPtr, const char* colorPtr), {
    var canvas = document.getElementById('gameCanvas');
    var ctx    = canvas.getContext('2d');
    ctx.fillStyle = UTF8ToString(colorPtr);
    ctx.font = '16px monospace';
    ctx.fillText(UTF8ToString(textPtr), x, y);
});

EM_JS(void, js_updateHUD, (int score, int hp, int level, int phase), {
    document.getElementById('score').textContent  = score;
    document.getElementById('health').textContent = '❤'.repeat(Math.max(0, hp));
    document.getElementById('level').textContent  = level;
    var overlay = document.getElementById('overlay');
    var msg     = document.getElementById('overlayMsg');
    if (phase === 4) {       // GAME_OVER
        overlay.style.display = 'flex';
        msg.textContent       = '💥 GAME OVER — Press R to Restart';
    } else if (phase === 5) { // VICTORY
        overlay.style.display = 'flex';
        msg.textContent       = '🏆 VICTORY! You defeated all bosses!';
    } else {
        overlay.style.display = 'none';
    }
});

// ─────────────────────────────────────────────
// Draw callbacks wrappers
// ─────────────────────────────────────────────
static void drawRect(float x, float y, float w, float h, const char* color) {
    js_drawRect(x, y, w, h, color);
}
static void drawText(float x, float y, const char* text, const char* color) {
    js_drawText(x, y, text, color);
}

// ─────────────────────────────────────────────
// Main Loop (called by Emscripten at ~60 fps)
// ─────────────────────────────────────────────
static void mainLoop() {
    if (!gEngine) return;
    gEngine->update(gConfig.fixedDt);
    js_clearCanvas();
    gEngine->render();
    js_updateHUD(gEngine->getScore(), gEngine->getHealth(),
                 gEngine->getLevel(), (int)gEngine->getPhase());
}

// ─────────────────────────────────────────────
// Keyboard Input Callbacks
// ─────────────────────────────────────────────
static EM_BOOL onKeyDown(int, const EmscriptenKeyboardEvent* e, void*) {
    if (!gEngine) return EM_FALSE;
    std::string key = e->key;
    if (key == "ArrowLeft"  || key == "a") gEngine->inputLeft  = true;
    if (key == "ArrowRight" || key == "d") gEngine->inputRight = true;
    if (key == "ArrowUp"    || key == "w") gEngine->inputUp    = true;
    if (key == "ArrowDown"  || key == "s") gEngine->inputDown  = true;
    if (key == " ") gEngine->inputFire = true;
    if (key == "r" || key == "R") gEngine->reset();
    if (key == " " && gEngine->getPhase() == GamePhase::MENU)
        gEngine->init();
    return EM_TRUE;
}

static EM_BOOL onKeyUp(int, const EmscriptenKeyboardEvent* e, void*) {
    if (!gEngine) return EM_FALSE;
    std::string key = e->key;
    if (key == "ArrowLeft"  || key == "a") gEngine->inputLeft  = false;
    if (key == "ArrowRight" || key == "d") gEngine->inputRight = false;
    if (key == "ArrowUp"    || key == "w") gEngine->inputUp    = false;
    if (key == "ArrowDown"  || key == "s") gEngine->inputDown  = false;
    if (key == " ") gEngine->inputFire = false;
    return EM_TRUE;
}

// ─────────────────────────────────────────────
// Exported C++ → JS API (EMSCRIPTEN_KEEPALIVE)
// JS can call these directly: Module._getScore()
// ─────────────────────────────────────────────
extern "C" {

EMSCRIPTEN_KEEPALIVE
int getScore()  { return gEngine ? gEngine->getScore()  : 0; }

EMSCRIPTEN_KEEPALIVE
int getHealth() { return gEngine ? gEngine->getHealth() : 0; }

EMSCRIPTEN_KEEPALIVE
int getLevel()  { return gEngine ? gEngine->getLevel()  : 0; }

EMSCRIPTEN_KEEPALIVE
int getPhase()  { return gEngine ? (int)gEngine->getPhase() : 0; }

EMSCRIPTEN_KEEPALIVE
void resetGame() { if (gEngine) gEngine->reset(); }

EMSCRIPTEN_KEEPALIVE
void setFireInput(int v) { if (gEngine) gEngine->inputFire = (v != 0); }

} // extern "C"

// ─────────────────────────────────────────────
// main()
// ─────────────────────────────────────────────
int main() {
    gConfig.screenW = 800;
    gConfig.screenH = 600;
    gEngine = new GameEngine(gConfig);
    gEngine->setDrawCallbacks(drawRect, drawText);
    gEngine->init();

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 1, onKeyDown);
    emscripten_set_keyup_callback  (EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 1, onKeyUp);

    // Set the main loop — 0 fps = use requestAnimationFrame, 1 = simulate_infinite_loop
    emscripten_set_main_loop(mainLoop, 0, 1);
    return 0;
}
