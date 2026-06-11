# Session 4: Performance & Security

## 1. Startup Latency
One of the most profound advantages WebAssembly holds over traditional containerization (like Docker) or virtual machines (like the JVM) is its near-instantaneous startup latency.
- **JIT and AOT Compilation**: Runtimes such as Wasmtime can utilize Ahead-of-Time (AOT) compilation. This means the WASM binary is converted directly into optimized native machine code before execution begins.
- **Microsecond Cold Starts**: When instantiated, the runtime simply maps the pre-compiled functions and allocates a block of memory. This operation occurs in microseconds, making WASM the perfect candidate for highly responsive, event-driven serverless architectures.

## 2. Sandboxing Benefits
WebAssembly implements a rigorous, hardware-enforced sandbox boundary that guarantees isolation.
- **Default Deny**: By design, a WebAssembly module is entirely isolated. It has zero capability to perform I/O, network requests, or system calls unless explicitly permitted via WASI.
- **Memory Isolation**: A module operates exclusively within its own linear memory space. Even if a buffer overflow occurs within the WASM module, the vulnerability is trapped inside the sandbox and cannot corrupt the host operating system or other running modules.
- **Supply Chain Security**: Integrating third-party WASM libraries is inherently safer. If a malicious package is imported, it cannot harvest environment variables, access file systems, or open unauthorized network sockets.

## 3. Use Cases in Production
WebAssembly is no longer an experimental technology; it powers mission-critical applications today.
- **Figma**: Transpiled their extensive C++ rendering engine to WebAssembly. This migration resulted in a 3x speed improvement over their previous `asm.js` implementation, allowing complex graphics to render fluidly in the browser.
- **Envoy Proxy (Proxy-Wasm)**: Envoy supports WASM-based filters. This allows developers to write custom authentication or networking logic in Rust or C++, compile it to WASM, and inject it safely into the proxy without needing to recompile the Envoy binary itself.
- **Shopify Functions**: Shopify uses WebAssembly to execute custom backend logic written by merchants. WASM's speed ensures that these custom scripts execute synchronously during checkout without adding measurable latency.

### References
- [Figma's Journey to WebAssembly](https://www.figma.com/blog/webassembly-cut-figmas-load-time-by-3x/)
- [WebAssembly for Proxies (Proxy-Wasm)](https://github.com/proxy-wasm/spec)
