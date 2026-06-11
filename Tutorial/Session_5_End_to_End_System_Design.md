# Session 5: End-to-End System Design

## 1. The C++ -> Rust -> WASM Pipeline
In advanced system architectures, you often need to harness legacy, heavily-optimized C++ libraries while building the modern orchestration and business logic in memory-safe Rust.
- **C++ Compilation**: The legacy C++ logic is compiled to an object file or directly into a WebAssembly library.
- **Rust FFI (Foreign Function Interface)**: Rust code uses FFI to define bindings and safely call the external C++ functions.
- **Unified WASM Module**: Using tools like `wasm-pack` or Cargo, the entire project (combining the Rust core and the linked C++ logic) is compiled down into a single, cohesive WebAssembly module for deployment.

## 2. Multi-Language Architecture
Because WebAssembly serves as a universal compilation target, system design is no longer constrained by a single language runtime. A modern architecture might consist of:
- **Frontend**: A UI built with React or Vue (JavaScript/TypeScript).
- **Client-Side Processing**: Heavy data processing, cryptography, or image manipulation modules written in Rust and compiled to WASM running in the user's browser.
- **Edge Backend**: Serverless API endpoints written in Go or C++ and compiled to WASM, deployed globally on Edge nodes.
This polyglot approach allows specialized teams to use the best tool for their specific domain while standardizing on a single deployment artifact.

## 3. Microservices + WASM Modules
While traditional microservices orchestrate Docker containers, the next generation of cloud architecture relies on WASM components.
- **Lightweight Alternative**: Unlike Docker, which packages an entire operating system userland, WASM packages only the compiled application logic. This reduces image sizes from hundreds of megabytes to mere kilobytes.
- **WebAssembly Component Model**: This emerging standard allows WASM modules written in entirely different languages to communicate seamlessly without the overhead of JSON serialization or network requests.
- **Kubernetes Integrations**: Orchestration tools are evolving. Solutions like `Kwasm` and `Fermyon Spin` allow Kubernetes clusters to schedule, scale, and manage WebAssembly workloads identically to traditional containers, paving the way for hybrid deployments.

### Example Architecture
1. **Client**: The browser loads `app.wasm` (UI and state management built in Rust).
2. **API Gateway**: An Envoy proxy intercepts requests, running a WASM filter (Auth/Rate limiting built in Go).
3. **Backend Service**: A serverless WASI module executed on Wasmtime processes the business logic (built in C++).

### References
- [WebAssembly Component Model](https://component-model.bytecodealliance.org/)
- [Fermyon Spin for Microservices](https://www.fermyon.com/spin)
