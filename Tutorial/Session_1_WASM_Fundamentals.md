# Session 1: WASM Fundamentals

## 1. WebAssembly Architecture
WebAssembly (WASM) is a binary instruction format for a stack-based virtual machine. It is designed as a portable compilation target for programming languages, enabling high-performance applications on the web for both client and server environments.

### Core Characteristics:
- **Binary Format**: The `.wasm` format is compact, allowing for fast downloads, decoding, and execution compared to parsing traditional JavaScript.
- **Stack Machine Architecture**: Operations push and pop values from a stack rather than relying on hardware-specific registers. This keeps the instruction set small and cross-platform.
- **Types**: WebAssembly is strongly typed but simple, natively supporting only 4 basic value types: `i32`, `i64`, `f32`, and `f64`.

## 2. Execution Model
WASM is not designed to replace JavaScript; rather, it works seamlessly alongside it within the JavaScript engine (like V8 in Chrome).
- **Module**: The compiled WASM binary is loaded into the browser as a Module.
- **Memory**: WASM uses a resizable `ArrayBuffer` as its linear memory heap. This is an uninterrupted block of bytes that WASM can read from and write to.
- **Instantiation**: To run WASM, the JS environment instantiates the module by providing imports (such as JavaScript functions or memory blocks). The instantiated module then returns exports (WASM functions that JavaScript can invoke).

## 3. Sandbox Environment
Security is a primary design goal for WebAssembly. It executes safely within a sandboxed environment.
- **Browser Constraints**: It strictly adheres to the browser's same-origin policy and security permissions.
- **Memory Safety**: WASM code cannot access memory outside of the specific linear memory buffer provided to it. It has no ability to read the host operating system's memory or the JavaScript engine's internal state.
- **Control Flow Integrity**: Execution in WASM is structured. It cannot jump to arbitrary memory addresses, effectively neutralizing traditional security exploits like Return-Oriented Programming (ROP).

### Example
A simple WebAssembly text format (WAT) module that adds two numbers:
```wat
(module
  (func $add (param $lhs i32) (param $rhs i32) (result i32)
    local.get $lhs
    local.get $rhs
    i32.add)
  (export "add" (func $add))
)
```

### References
- [WebAssembly Core Specification](https://webassembly.github.io/spec/core/)
- [MDN Web Docs: WebAssembly Concepts](https://developer.mozilla.org/en-US/docs/WebAssembly/Concepts)
