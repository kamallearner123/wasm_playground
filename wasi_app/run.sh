#!/bin/bash
set -e

echo "Building Rust module to WASI..."

# Add the WASI target to rustup
rustup target add wasm32-wasip1

# Build the WASI binary
cargo build --target wasm32-wasip1

echo "Successfully built Rust WASI module."

echo "Creating a test file for WASI to read..."
echo "This file was read from the host filesystem by a WASM module running in a secure sandbox!" > wasi_test.txt

echo "Running WASI module with wasmtime..."
# We map the current directory to the WASI sandbox using --dir .
# We pass an environment variable using --env
~/.wasmtime/bin/wasmtime run --env WASI_USER="WASM Trainee" --dir . target/wasm32-wasip1/debug/wasi_app.wasm
