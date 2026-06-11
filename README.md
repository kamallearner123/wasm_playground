# WebAssembly Playground

This repository is a playground for exploring WebAssembly (WASM) concepts, featuring examples of compiling C++ and Rust to WASM, as well as using the WebAssembly System Interface (WASI).

## Directory Structure

*   **`cpp_wasm/`**: Contains C++ code compiled to WebAssembly using Emscripten. Demonstrates how to write core logic in C++ and expose it to JavaScript.
*   **`rust_wasm/`**: Contains Rust code compiled to WebAssembly using `wasm-pack`. Shows how to build and package Rust code for the web.
*   **`wasi_app/`**: A standalone WebAssembly application built with Rust targeting the `wasm32-wasi` platform. Demonstrates running WASM outside the browser (e.g., using Wasmtime) with access to the file system and environment variables.
*   **`web_app/`**: The frontend web application that integrates and loads the WASM modules generated from the C++ and Rust code.
*   **`setup.sh`**: A utility script to set up the environment and install necessary toolchains like Emscripten and Rust targets.

## Running the Examples

1.  Make sure you have the necessary toolchains installed (Emscripten, Rust, `wasm-pack`, and `wasmtime`). You can review `setup.sh` for guidance.
2.  Build the C++ WASM module by navigating to `cpp_wasm/` and running `./build.sh`.
3.  Build the Rust WASM module by navigating to `rust_wasm/` and running `./build.sh`.
4.  Run the WASI application by navigating to `wasi_app/` and running `./run.sh`.
5.  Serve the root directory (or `web_app/`) using a local web server, for example: `python3 -m http.server 8080`. Then open `http://localhost:8080/web_app/` in your browser.
