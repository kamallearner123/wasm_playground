#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/browser"

echo "══════════════════════════════════════════════"
echo "  Space Defender — Browser (WASM) Build"
echo "══════════════════════════════════════════════"

# Check Emscripten
if ! command -v emcc &> /dev/null; then
    echo "❌ emcc not found. Please source Emscripten's emsdk_env.sh"
    echo "   e.g.: source ~/emsdk/emsdk_env.sh"
    exit 1
fi
echo "✅ Emscripten: $(emcc --version | head -1)"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "📦 Configuring with CMake (Emscripten toolchain)..."
emcmake cmake "$SCRIPT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    2>&1 | tail -5

echo ""
echo "🔨 Building..."
emmake make -j$(nproc) 2>&1

echo ""
echo "📋 Copying HTML to build dir..."
cp "$SCRIPT_DIR/browser/index.html" "$BUILD_DIR/index.html"

echo ""
echo "══════════════════════════════════════════════"
echo "  ✅ Build complete!"
echo "══════════════════════════════════════════════"
echo "  Output: $BUILD_DIR/"
echo "  Files:  game.js  game.wasm  index.html"
echo ""
echo "  To run:"
echo "    cd $BUILD_DIR"
echo "    python3 -m http.server 8080"
echo "    Open: http://localhost:8080"
echo "══════════════════════════════════════════════"
