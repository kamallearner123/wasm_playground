#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "════════════════════════════════════════"
echo "  Case 1: C++ → Pure WASM (no JS glue)"
echo "════════════════════════════════════════"

if ! command -v emcc &>/dev/null; then echo "❌ emcc not found"; exit 1; fi
echo "✅ $(emcc --version | head -1)"

# Build a STANDALONE .wasm — no Emscripten glue JS file.
# The browser loads it directly using WebAssembly.instantiateStreaming(fetch('math.wasm'))
emcc "$SCRIPT_DIR/src/math.cpp" \
    -std=c++20 \
    -O2 \
    -s WASM=1 \
    -s STANDALONE_WASM=1 \
    --no-entry \
    -s EXPORTED_FUNCTIONS='["_add","_subtract","_multiply","_divide","_power","_modulo","_fibonacci","_fibonacci_recursive","_is_prime","_factorial"]' \
    -o "$SCRIPT_DIR/math.wasm"

echo ""
echo "✅ math.wasm built: $(du -h "$SCRIPT_DIR/math.wasm" | cut -f1)"
echo ""
echo "════════════════════════════════════════"
echo "  Run:"
echo "    cd $SCRIPT_DIR"
echo "    python3 -m http.server 8083"
echo "    Open: http://localhost:8083"
echo "════════════════════════════════════════"
