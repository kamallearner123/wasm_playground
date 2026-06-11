# WASM Practicals — Hands-On Lab Guide

> **Lab Prerequisites**: `g++`, `emscripten`, `wasmtime`, `python3`

---

## Example 01 — Compiling C++ for the Target (Native) Machine

### 🎯 Objective
Understand how C++ code is compiled and linked into a native binary for your host machine, before contrasting it with WASM compilation.

### Step 1: Write the C++ Source
**File: `lab01/calculator.cpp`**
```cpp
#include <iostream>
#include <string>

int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
long long factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    std::cout << "=== Native Calculator ===" << std::endl;
    std::cout << "add(10, 5)       = " << add(10, 5) << std::endl;
    std::cout << "subtract(10, 5)  = " << subtract(10, 5) << std::endl;
    std::cout << "factorial(10)    = " << factorial(10) << std::endl;
    return 0;
}
```

### Step 2: Compile and Run
```bash
mkdir -p lab01
g++ -O2 -o lab01/calculator lab01/calculator.cpp
./lab01/calculator
```

**Expected Output:**
```
=== Native Calculator ===
add(10, 5)       = 15
subtract(10, 5)  = 5
factorial(10)    = 3628800
```

### Step 3: Inspect the Binary
```bash
# Check the target architecture
file lab01/calculator

# Check which system libraries it links to
ldd lab01/calculator
```
The binary is tied to your OS and CPU architecture (e.g., `ELF 64-bit LSB executable, x86-64`). It **cannot run on ARM** or inside a browser.

### 🔍 Key Insight
| Property | Native Binary |
|---|---|
| Portability | x86-64 Linux only |
| File Access | Full, unrestricted |
| Browser Support | ❌ None |
| Security Sandbox | ❌ None |

---

## Example 02 — Compiling C++ to WebAssembly with Emscripten

### 🎯 Objective
Compile the same logic to WASM, expose functions to JavaScript, and explore the browser sandbox security model.

### Step 1: Write the C++ Logic
**File: `lab02/calculator.cpp`**
```cpp
#include <emscripten/emscripten.h>
#include <cmath>

extern "C" {

EMSCRIPTEN_KEEPALIVE
int add(int a, int b) { return a + b; }

EMSCRIPTEN_KEEPALIVE
int subtract(int a, int b) { return a - b; }

EMSCRIPTEN_KEEPALIVE
long long factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

EMSCRIPTEN_KEEPALIVE
double sqrt_val(double n) { return sqrt(n); }

// ⚠️ This WILL be compiled, but cannot be called from WASM context
void try_read_file() {
    FILE* f = fopen("/etc/passwd", "r"); // Blocked by sandbox
    if (!f) { printf("ACCESS DENIED: Cannot read /etc/passwd\n"); }
    else { printf("File opened (should not happen in browser)\n"); fclose(f); }
}

} // extern "C"
```

### Step 2: Compile with Emscripten
```bash
mkdir -p lab02/out

emcc lab02/calculator.cpp \
  -O2 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_add","_subtract","_factorial","_sqrt_val"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -o lab02/out/calculator.js
```
This generates two files:
- `calculator.js` — The JavaScript glue code
- `calculator.wasm` — The actual WASM binary

### Step 3: Create the HTML Frontend
**File: `lab02/out/index.html`**
```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WASM Calculator Lab</title>
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <div class="container">
    <h1>⚡ WASM Calculator</h1>
    <p class="subtitle">C++ logic running in your browser via WebAssembly</p>
    <div class="card">
      <h2>Add Two Numbers</h2>
      <input type="number" id="a" placeholder="Enter A" value="10">
      <input type="number" id="b" placeholder="Enter B" value="5">
      <button onclick="runAdd()">Calculate</button>
      <div class="result" id="add-result">Result: —</div>
    </div>
    <div class="card">
      <h2>Factorial</h2>
      <input type="number" id="n" placeholder="Enter N" value="10">
      <button onclick="runFactorial()">Calculate</button>
      <div class="result" id="fact-result">Result: —</div>
    </div>
    <div class="card warning">
      <h2>🔒 Security Demo: File Access</h2>
      <p>Attempt to read <code>/etc/passwd</code> from WASM.</p>
      <button onclick="runFileAccess()">Try File Access</button>
      <div class="result" id="file-result">Result: —</div>
    </div>
  </div>
  <script src="calculator.js"></script>
  <script src="app.js"></script>
</body>
</html>
```

### Step 4: Create the CSS
**File: `lab02/out/style.css`**
```css
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap');

* { box-sizing: border-box; margin: 0; padding: 0; }

body {
  font-family: 'Inter', sans-serif;
  background: #0f0f1a;
  color: #e0e0f0;
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding: 2rem;
}

.container { max-width: 700px; width: 100%; }

h1 { font-size: 2.2rem; color: #a78bfa; margin-bottom: 0.25rem; }
.subtitle { color: #6b7280; margin-bottom: 2rem; }

.card {
  background: #1a1a2e;
  border: 1px solid #2d2d4a;
  border-radius: 12px;
  padding: 1.5rem;
  margin-bottom: 1.5rem;
}

.card h2 { font-size: 1.1rem; margin-bottom: 1rem; color: #c4b5fd; }

input {
  background: #0f0f1a;
  border: 1px solid #3d3d6a;
  border-radius: 8px;
  color: #e0e0f0;
  padding: 0.5rem 0.75rem;
  font-size: 1rem;
  width: 120px;
  margin-right: 0.5rem;
  margin-bottom: 0.75rem;
}

button {
  background: #7c3aed;
  color: white;
  border: none;
  border-radius: 8px;
  padding: 0.5rem 1.25rem;
  font-size: 1rem;
  cursor: pointer;
  transition: background 0.2s;
}
button:hover { background: #6d28d9; }

.result {
  margin-top: 0.75rem;
  padding: 0.6rem 1rem;
  background: #0f0f1a;
  border-radius: 8px;
  font-weight: 600;
  color: #34d399;
  border-left: 3px solid #34d399;
}

.warning { border-color: #f59e0b; }
.warning h2 { color: #fbbf24; }
.warning .result { color: #f87171; border-color: #f87171; }
```

### Step 5: Create the JavaScript Bridge
**File: `lab02/out/app.js`**
```javascript
// Wait for the WASM module to be ready
Module.onRuntimeInitialized = () => {
  console.log("✅ WASM Module Initialized");

  // Use cwrap to bind C++ functions to JS
  window._add       = Module.cwrap('add',       'number', ['number', 'number']);
  window._subtract  = Module.cwrap('subtract',  'number', ['number', 'number']);
  window._factorial = Module.cwrap('factorial', 'number', ['number']);
  window._sqrt_val  = Module.cwrap('sqrt_val',  'number', ['number']);
};

function runAdd() {
  const a = parseInt(document.getElementById('a').value);
  const b = parseInt(document.getElementById('b').value);
  const result = _add(a, b);
  document.getElementById('add-result').textContent = `Result: ${a} + ${b} = ${result}`;
}

function runFactorial() {
  const n = parseInt(document.getElementById('n').value);
  const result = _factorial(n);
  document.getElementById('fact-result').textContent = `Result: ${n}! = ${result}`;
}

function runFileAccess() {
  // In a browser, WASM has NO access to the host filesystem.
  // It can only access Emscripten's virtual in-memory filesystem.
  document.getElementById('file-result').textContent =
    "❌ BLOCKED: WASM in browser has NO access to host filesystem (/etc/passwd, etc). " +
    "This is the security sandbox in action.";
}
```

### Step 6: Run It
```bash
cd lab02/out
python3 -m http.server 8080
# Open: http://localhost:8080
```

### 🔒 Security & Limitations Summary

| Feature | WASM in Browser |
|---|---|
| Read host filesystem | ❌ Blocked |
| Open network sockets | ❌ Blocked |
| Spawn processes | ❌ Blocked |
| Access hardware directly | ❌ Blocked |
| Execute in browser | ✅ Yes |
| Memory isolation | ✅ Strict linear memory |
| Call JS functions | ✅ Via imports |

> **Security Insight**: WASM runs in a "default-deny" environment. The browser's JavaScript engine controls what the WASM module can and cannot do. Even if malicious code is compiled into a WASM binary, it cannot escape the sandbox.

---

## Example 03 — WASM Outside the Browser with WASI

### 🎯 Objective
Run WASM on a server using Wasmtime, and explicitly grant access to files and environment variables using WASI's capability-based model.

### Step 1: Write the Rust WASI Application
**File: `lab03/src/main.rs`**
```rust
use std::env;
use std::fs;
use std::io::Write;

fn main() {
    println!("=== WASI Lab Demo ===");

    // 1. Reading Environment Variables
    // The host must explicitly export these; WASI can't read arbitrary env vars.
    let user = env::var("LAB_USER").unwrap_or_else(|_| "Guest".to_string());
    let mode = env::var("LAB_MODE").unwrap_or_else(|_| "default".to_string());
    println!("[ENV] User: {}, Mode: {}", user, mode);

    // 2. Reading a File (host must grant directory access via --dir flag)
    let input_path = "data/input.txt";
    match fs::read_to_string(input_path) {
        Ok(contents) => println!("[FILE READ] Content:\n{}", contents.trim()),
        Err(e) => println!("[FILE READ] Error (is --dir granted?): {}", e),
    }

    // 3. Writing a File
    let output_path = "data/output.txt";
    match fs::File::create(output_path) {
        Ok(mut file) => {
            writeln!(file, "Processed by WASI module for user: {}", user).unwrap();
            println!("[FILE WRITE] Written to {}", output_path);
        }
        Err(e) => println!("[FILE WRITE] Error: {}", e),
    }

    // 4. Reading command-line arguments
    let args: Vec<String> = env::args().collect();
    println!("[ARGS] Received {} argument(s): {:?}", args.len() - 1, &args[1..]);
}
```

**File: `lab03/Cargo.toml`**
```toml
[package]
name = "wasi-lab"
version = "0.1.0"
edition = "2021"
```

### Step 2: Compile to WASI Target
```bash
cd lab03
rustup target add wasm32-wasip1
cargo build --target wasm32-wasip1
```

### Step 3: Prepare Test Data
```bash
mkdir -p lab03/data
echo "Hello from the host filesystem! Line 1.
Line 2: WASI grants explicit access.
Line 3: No access beyond what is granted." > lab03/data/input.txt
```

### Step 4: Run — WITHOUT File Access (Default Deny)
```bash
wasmtime lab03/target/wasm32-wasip1/debug/wasi-lab.wasm
```
**Output:**
```
=== WASI Lab Demo ===
[ENV] User: Guest, Mode: default
[FILE READ] Error (is --dir granted?): failed to find a pre-opened file descriptor...
```
✅ The module cannot read files unless we explicitly grant access.

### Step 5: Run — WITH File Access (`--dir` flag)
```bash
wasmtime \
  --env LAB_USER=Kamal \
  --env LAB_MODE=production \
  --dir lab03/data::data \
  lab03/target/wasm32-wasip1/debug/wasi-lab.wasm -- arg1 arg2
```

**Explanation of flags:**
| Flag | Meaning |
|---|---|
| `--env KEY=VALUE` | Expose a specific env variable to the WASM module |
| `--dir host_path::wasm_path` | Grant the module access to a specific directory only |
| `-- arg1 arg2` | Pass CLI arguments to the WASM module |

**Output:**
```
=== WASI Lab Demo ===
[ENV] User: Kamal, Mode: production
[FILE READ] Content:
Hello from the host filesystem! Line 1.
Line 2: WASI grants explicit access.
Line 3: No access beyond what is granted.
[FILE WRITE] Written to data/output.txt
[ARGS] Received 2 argument(s): ["arg1", "arg2"]
```

### Step 6: Verify the Output File was Written
```bash
cat lab03/data/output.txt
# Processed by WASI module for user: Kamal
```

### 🔍 WASI Capability Model Summary

```
Host Machine (Full Access)
│
│  ── grants capabilities explicitly ──►
│
WASM Module (Default: No Access)
  ├── ENV: Only variables passed via --env
  ├── FILES: Only directories passed via --dir
  ├── NETWORK: Only sockets passed via --tcplisten (Wasmtime)
  └── ARGS: Only values passed via -- ...
```

---

## Example 04 — Complex Algorithm Lab

### 🎯 Objective
Implement a computationally intensive algorithm in C++ (Matrix Multiplication), compile it to WASM, and compare performance with a pure JavaScript implementation in the browser.

### Step 1: C++ with Emscripten
**File: `lab04/matrix.cpp`**
```cpp
#include <emscripten/emscripten.h>
#include <vector>
#include <cstdlib>

// Flat array matrix multiply: C = A * B (all N x N)
extern "C" {

EMSCRIPTEN_KEEPALIVE
void matrix_multiply(
    float* A, float* B, float* C, int N)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++)
                sum += A[i * N + k] * B[k * N + j];
            C[i * N + j] = sum;
        }
}

EMSCRIPTEN_KEEPALIVE
float* alloc_matrix(int N) {
    return (float*)malloc(N * N * sizeof(float));
}

EMSCRIPTEN_KEEPALIVE
void free_matrix(float* ptr) {
    free(ptr);
}

} // extern "C"
```

**Compile:**
```bash
mkdir -p lab04/out
emcc lab04/matrix.cpp \
  -O3 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_matrix_multiply","_alloc_matrix","_free_matrix"]' \
  -s EXPORTED_RUNTIME_METHODS='["cwrap","getValue","setValue"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -o lab04/out/matrix.js
```

### Step 2: HTML + JS Frontend with Benchmarking
**File: `lab04/out/index.html`**
```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Matrix Multiply Benchmark — WASM vs JS</title>
  <style>
    body { font-family: 'Inter', sans-serif; background: #0f0f1a; color: #e0e0f0; padding: 2rem; }
    h1 { color: #a78bfa; }
    .controls { margin: 1.5rem 0; }
    label { color: #9ca3af; margin-right: 0.5rem; }
    input { background: #1a1a2e; border: 1px solid #3d3d6a; border-radius: 6px;
            color: #e0e0f0; padding: 0.4rem 0.6rem; width: 80px; }
    button { background: #7c3aed; color: white; border: none; border-radius: 8px;
             padding: 0.6rem 1.5rem; font-size: 1rem; cursor: pointer; margin: 0.5rem 0; }
    button:hover { background: #6d28d9; }
    .result-box { background: #1a1a2e; border-radius: 12px; padding: 1.5rem; margin-top: 1.5rem; }
    .row { display: flex; justify-content: space-between; padding: 0.5rem 0;
           border-bottom: 1px solid #2d2d4a; }
    .label { color: #9ca3af; }
    .val { font-weight: 700; }
    .wasm-val { color: #34d399; }
    .js-val { color: #f59e0b; }
    .winner { margin-top: 1rem; font-size: 1.1rem; font-weight: 700; color: #a78bfa; }
  </style>
</head>
<body>
  <h1>⚡ Matrix Multiply: WASM vs JavaScript</h1>
  <p style="color:#6b7280">Measures the time to multiply two N×N matrices. C++ compiled to WASM vs pure JS.</p>
  <div class="controls">
    <label>Matrix Size (N):</label>
    <input type="number" id="size" value="256" min="64" max="512">
    <br>
    <button onclick="runBenchmark()">▶ Run Benchmark</button>
  </div>
  <div class="result-box" id="results" style="display:none">
    <div class="row"><span class="label">Matrix Size</span><span class="val" id="r-size">—</span></div>
    <div class="row"><span class="label">WASM Time</span><span class="val wasm-val" id="r-wasm">—</span></div>
    <div class="row"><span class="label">JS Time</span><span class="val js-val" id="r-js">—</span></div>
    <div class="winner" id="r-winner">—</div>
  </div>
  <script src="matrix.js"></script>
  <script src="benchmark.js"></script>
</body>
</html>
```

**File: `lab04/out/benchmark.js`**
```javascript
let wasmReady = false;
let _matMul, _allocMatrix, _freeMatrix;

Module.onRuntimeInitialized = () => {
  _matMul     = Module.cwrap('matrix_multiply', null,     ['number','number','number','number']);
  _allocMatrix= Module.cwrap('alloc_matrix',    'number', ['number']);
  _freeMatrix = Module.cwrap('free_matrix',     null,     ['number']);
  wasmReady   = true;
  console.log("✅ WASM Matrix module ready");
};

// Pure JavaScript matrix multiply
function jsMatMul(A, B, N) {
  const C = new Float32Array(N * N);
  for (let i = 0; i < N; i++)
    for (let j = 0; j < N; j++) {
      let sum = 0;
      for (let k = 0; k < N; k++)
        sum += A[i * N + k] * B[k * N + j];
      C[i * N + j] = sum;
    }
  return C;
}

function randomMatrix(N) {
  const m = new Float32Array(N * N);
  for (let i = 0; i < m.length; i++) m[i] = Math.random();
  return m;
}

async function runBenchmark() {
  if (!wasmReady) { alert("WASM module not ready yet!"); return; }

  const N = parseInt(document.getElementById('size').value);
  const A = randomMatrix(N);
  const B = randomMatrix(N);

  // ---- WASM Benchmark ----
  const pA = _allocMatrix(N);
  const pB = _allocMatrix(N);
  const pC = _allocMatrix(N);
  Module.HEAPF32.set(A, pA >> 2);
  Module.HEAPF32.set(B, pB >> 2);

  const t0 = performance.now();
  _matMul(pA, pB, pC, N);
  const wasmTime = performance.now() - t0;

  _freeMatrix(pA); _freeMatrix(pB); _freeMatrix(pC);

  // ---- JS Benchmark ----
  const t1 = performance.now();
  jsMatMul(A, B, N);
  const jsTime = performance.now() - t1;

  // ---- Display Results ----
  document.getElementById('results').style.display = 'block';
  document.getElementById('r-size').textContent  = `${N} × ${N}`;
  document.getElementById('r-wasm').textContent  = `${wasmTime.toFixed(2)} ms`;
  document.getElementById('r-js').textContent    = `${jsTime.toFixed(2)} ms`;
  const speedup = (jsTime / wasmTime).toFixed(2);
  const winner = wasmTime < jsTime
    ? `🏆 WASM is ${speedup}x faster than JavaScript!`
    : `JS was faster this time (JIT warmup may have helped).`;
  document.getElementById('r-winner').textContent = winner;
}
```

### Step 3: Run the Benchmark
```bash
cd lab04/out
python3 -m http.server 8080
# Open http://localhost:8080
# Try N=128, 256, 512 and observe the speedup
```

### Expected Results (Approximate)
| Matrix Size | JS Time | WASM Time | Speedup |
|---|---|---|---|
| 128×128 | ~8 ms | ~3 ms | ~2.5x |
| 256×256 | ~60 ms | ~18 ms | ~3.3x |
| 512×512 | ~500 ms | ~140 ms | ~3.5x |

### 🔍 Why is WASM Faster?
1. **Predictable Types**: WASM uses static `f32` types. JS uses dynamic type inference, which adds overhead.
2. **No Garbage Collector**: WASM uses manually managed linear memory with `malloc`/`free`.
3. **Direct Compilation**: WASM compiles directly to machine code; JS goes through JIT compilation with optimization passes.

---

## Lab Summary

| Lab | Concept | Tool | Key Takeaway |
|---|---|---|---|
| 01 | Native C++ Compilation | `g++` | Platform-specific, full OS access |
| 02 | C++ → Browser WASM | `emcc` | Sandboxed, no filesystem/network |
| 03 | C++ → Server WASM (WASI) | `wasmtime` | Capability-based access control |
| 04 | Performance Benchmark | `emcc` + JS | WASM is 2–4x faster for compute-heavy tasks |
