# Session 2: Toolchain

## 1. LLVM to WASM
LLVM (Low Level Virtual Machine) is a powerful, industry-standard compiler infrastructure. Many modern languages (including C, C++, Rust, and Swift) utilize LLVM as their compiler backend. WebAssembly is officially supported as a first-class compilation target in LLVM.
- When compiling code, the frontend language compiler translates the source code into LLVM Intermediate Representation (IR).
- The LLVM backend then processes this IR, applies generic optimizations, and translates it into the WebAssembly binary format (`.wasm`).

## 2. Emscripten (C++)
Emscripten is the premier toolchain for compiling C and C++ projects into WebAssembly.
- **How it works**: It invokes Clang (the C++ frontend for LLVM) to compile code to WASM. Crucially, it generates the necessary JavaScript "glue code" required to load and interact with the WASM module in a browser.
- **POSIX Emulation**: Emscripten simulates a standard C/POSIX environment. If your C++ code interacts with the filesystem, uses threads (`pthreads`), or renders graphics (OpenGL), Emscripten automatically maps these calls to the browser's File API, WebWorkers, and WebGL respectively.

### Example (Emscripten)
```cpp
// math.cpp
extern "C" {
    int square(int n) {
        return n * n;
    }
}
```
Compile command: 
```bash
emcc math.cpp -s EXPORTED_FUNCTIONS="['_square']" -o math.js
```

## 3. Rust to WASM (wasm-pack)
Rust offers seamless, first-class support for WebAssembly. The standard tool for packaging Rust for the web is `wasm-pack`.
- `wasm-pack` manages the compilation of Rust code to WASM and utilizes the `wasm-bindgen` tool under the hood.
- `wasm-bindgen` is revolutionary because it handles the complex serialization and deserialization of data types between Rust and JavaScript. Instead of only passing integers, you can pass strings, arrays, and complex objects directly.

### Example (wasm-pack)
```rust
// src/lib.rs
use wasm_bindgen::prelude::*;

#[wasm_bindgen]
pub fn greet(name: &str) -> String {
    format!("Hello, {}!", name)
}
```
Compile command: 
```bash
wasm-pack build --target web
```

### References
- [Emscripten Documentation](https://emscripten.org/docs/index.html)
- [Rust and WebAssembly Book](https://rustwasm.github.io/docs/book/)
