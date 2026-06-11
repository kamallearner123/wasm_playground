#!/bin/bash
set -e

echo "====================================="
echo "Building Integrated WASM Architecture"
echo "====================================="

# Check if Emscripten is active
if ! command -v emcc &> /dev/null
then
    echo "Error: emcc not found! Please ensure Emscripten SDK is activated."
    echo "Run: source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

# Build C++ Module
echo "--- Building C++ Module ---"
cd cpp_wasm
./build.sh
cd ..

# Build Rust Module
echo "--- Building Rust Module ---"
cd rust_wasm
./build.sh
cd ..

# Build WASI App (Optional, just to ensure it compiles)
echo "--- Building WASI Module ---"
cd wasi_app
./run.sh
cd ..

echo "====================================="
echo "All modules built successfully!"
echo "To run the web app, start a local server from this root directory:"
echo "python3 -m http.server 8080"
echo "Then navigate to: http://localhost:8080/web_app/"
echo "====================================="
