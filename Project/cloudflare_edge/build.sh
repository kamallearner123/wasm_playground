#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "══════════════════════════════════════════════"
echo "  Cloudflare Edge Handler — WASM Build"
echo "══════════════════════════════════════════════"

if ! command -v emcc &> /dev/null; then
    echo "❌ emcc not found. Please source Emscripten's emsdk_env.sh"
    exit 1
fi
echo "✅ Emscripten: $(emcc --version | head -1)"

mkdir -p "$BUILD_DIR"

echo ""
echo "🔨 Compiling C++ → WASM (standalone, no HTML shell)..."

emcc \
    "$SCRIPT_DIR/src/api_handler.cpp" \
    -I "$SCRIPT_DIR/include" \
    -std=c++20 \
    -O2 \
    -fno-exceptions \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_edgeInit","_edgeHandle","_edgeFreeString","_edgeGetRequestCount","_edgeGetRateRemaining","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["UTF8ToString","stringToUTF8","lengthBytesUTF8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s NODEJS_CATCH_EXIT=0 \
    -s ENVIRONMENT='node' \
    -o "$BUILD_DIR/edge_handler.js"

echo ""
echo "══════════════════════════════════════════════"
echo "  ✅ Build complete!"
echo "══════════════════════════════════════════════"
echo "  Output: $BUILD_DIR/"
echo "  Files:  edge_handler.js + edge_handler.wasm"
echo ""
echo "  To run local simulation:"
echo "    node local_sim/server.js"
echo "    Open: http://localhost:8082"
echo ""
echo "  To deploy to real Cloudflare:"
echo "    npm install -g wrangler"
echo "    wrangler login"
echo "    wrangler publish"
echo "══════════════════════════════════════════════"
