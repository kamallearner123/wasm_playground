# Session 3: WASI & Runtime

## 1. The WASI Interface
WASI (WebAssembly System Interface) is a standardized API designed to give WebAssembly modules secure, cross-platform access to standard operating system features—such as files, networks, and system clocks.
- Standard WebAssembly is purely computational and cannot interact with the outside world. WASI bridges this gap for modules running outside the browser.
- **Capability-based security**: WASI enforces a strict security model. A WASI module cannot read arbitrary files on the host machine. Instead, the host environment must explicitly grant the module the capability to access specific resources (e.g., passing a specific directory handle).

## 2. Running WASM Outside the Browser
To execute WASI-compliant modules, a standalone WebAssembly runtime is required. These runtimes function similarly to the JVM but are heavily optimized for speed and security.
- **Wasmtime**: Developed by the Bytecode Alliance, Wasmtime is a fast, secure, and widely-used JIT runtime for WebAssembly and WASI.
- **Wasmer**: Another highly popular, lightweight WebAssembly runtime that supports universal execution across different systems.
- **WasmEdge**: A runtime specifically optimized for edge computing environments, microservices, and AI inference workloads.

### Example (Wasmtime)
Compile a standard Rust application targeting WASI:
```bash
cargo build --target wasm32-wasi
```
Execute the resulting module using Wasmtime:
```bash
wasmtime target/wasm32-wasi/debug/app.wasm
```

## 3. Edge Computing Use Cases
The combination of WASM and WASI is rapidly becoming the standard for Edge Computing (e.g., Cloudflare Workers, Fastly Compute).
- **Lightning Fast Cold Starts**: Unlike Docker containers which take seconds to spin up, WASM modules instantiate in microseconds, effectively eliminating cold start latency in serverless computing.
- **True Portability**: A WASM binary is compiled once and runs identically on an x86 server in AWS, an ARM edge node, or a developer's local machine.
- **Dense Multi-Tenancy**: Because the WASM sandbox is incredibly strict, thousands of untrusted modules from different users can securely run side-by-side within the same physical process, drastically reducing overhead.

### References
- [WASI Standard](https://wasi.dev/)
- [Wasmtime Runtime](https://wasmtime.dev/)
