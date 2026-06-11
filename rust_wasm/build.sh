#!/bin/bash
set -e

echo "Building Rust module to WASM..."

# We use the target 'web' to allow loading natively as an ES module in browser
wasm-pack build --target web

echo "Successfully built Rust WASM module in pkg/"
