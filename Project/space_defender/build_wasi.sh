#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/wasi"
DATA_DIR="$SCRIPT_DIR/build/wasi/data"

echo "══════════════════════════════════════════════"
echo "  Space Defender — WASI Build"
echo "══════════════════════════════════════════════"

# Check clang with wasm32-wasi support
if ! command -v clang++ &> /dev/null; then
    echo "❌ clang++ not found. Please install LLVM."
    exit 1
fi

if ! command -v wasmtime &> /dev/null; then
    echo "❌ wasmtime not found. Install from: https://wasmtime.dev"
    exit 1
fi

echo "✅ clang++: $(clang++ --version | head -1)"
echo "✅ wasmtime: $(wasmtime --version)"

mkdir -p "$BUILD_DIR" "$DATA_DIR"

echo ""
echo "🔨 Compiling to wasm32-wasi..."
clang++ \
    --target=wasm32-wasip1 \
    --sysroot="$(wasmtime compile --help 2>/dev/null | head -1 || echo '')" \
    -std=c++20 \
    -O2 \
    -I "$SCRIPT_DIR/include" \
    "$SCRIPT_DIR/wasi/main_wasi.cpp" \
    "$SCRIPT_DIR/engine/engine.cpp" \
    "$SCRIPT_DIR/ai/ai.cpp" \
    -o "$BUILD_DIR/wasi_defender.wasm" \
    2>&1 || {
    # Fallback: try with wasi-sdk if available
    WASI_SDK="${WASI_SDK_PATH:-/opt/wasi-sdk}"
    echo "Retrying with wasi-sdk at $WASI_SDK..."
    "$WASI_SDK/bin/clang++" \
        --target=wasm32-wasi \
        -std=c++20 \
        -O2 \
        -I "$SCRIPT_DIR/include" \
        "$SCRIPT_DIR/wasi/main_wasi.cpp" \
        "$SCRIPT_DIR/engine/engine.cpp" \
        "$SCRIPT_DIR/ai/ai.cpp" \
        -o "$BUILD_DIR/wasi_defender.wasm"
}

echo "✅ WASM binary: $BUILD_DIR/wasi_defender.wasm"
echo "   Size: $(du -h "$BUILD_DIR/wasi_defender.wasm" | cut -f1)"

echo ""
echo "══════════════════════════════════════════════"
echo "  ✅ Build complete! Running demo simulation..."
echo "══════════════════════════════════════════════"
echo ""

# Run a short simulation demo
wasmtime \
    --env PILOT=DemoBot \
    --dir "$DATA_DIR::data" \
    "$BUILD_DIR/wasi_defender.wasm" \
    -- --frames 120 --output data/scores.txt --seed 42

echo ""
echo "══════════════════════════════════════════════"
echo "  Score file contents:"
echo "══════════════════════════════════════════════"
cat "$DATA_DIR/scores.txt" 2>/dev/null || echo "(file not written)"
echo ""
echo "  Full run command:"
echo "    wasmtime --env PILOT=YourName --dir $DATA_DIR::data \\"
echo "      $BUILD_DIR/wasi_defender.wasm -- --frames 600 --verbose"
echo "══════════════════════════════════════════════"
