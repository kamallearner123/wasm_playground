#!/bin/bash
# Exit on error
set -e

echo "Building C++ module to WASM..."

# Ensure output directory exists
mkdir -p out

# Compile to WASM using Emscripten
# -O3 for maximum optimization
# -s WASM=1 to enforce WASM output
# -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "getValue", "setValue"]'
# -s MODULARIZE=1 and -s EXPORT_ES6=1 to output an ES6 module we can import in JS
emcc math_core.cpp -O3 -s WASM=1 \
    -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
    -s EXPORTED_FUNCTIONS='["_fibonacci", "_sum_array", "_malloc", "_free"]' \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s EXPORT_NAME="createCppWasmModule" \
    -o out/math_core.js

echo "Successfully built C++ WASM module in out/"
