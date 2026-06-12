#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"

echo "════════════════════════════════════════"
echo "  Log Analyser — WASM Build"
echo "════════════════════════════════════════"

if ! command -v emcc &>/dev/null; then echo "❌ emcc not found"; exit 1; fi
echo "✅ $(emcc --version | head -1)"

emcc "$SCRIPT_DIR/src/analyser.cpp" \
    -std=c++20 \
    -O2 \
    -fno-exceptions \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_analyserInit","_analyserFeedLine","_analyserGetStats","_analyserFree","_analyserGetTotal","_analyserGetErrors","_analyserGetWarnings","_analyserGetInfo","_analyserGetFatal","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["UTF8ToString","stringToUTF8","lengthBytesUTF8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='createAnalyserModule' \
    -s ENVIRONMENT='web' \
    -o "$BUILD_DIR/analyser.js"

echo "✅ analyser.js + analyser.wasm → $BUILD_DIR/"
echo ""
echo "  Run: cd $SCRIPT_DIR && python3 -m http.server 8084"
echo "  Open: http://localhost:8084"
echo "════════════════════════════════════════"
