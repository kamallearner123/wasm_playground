# Cloudflare Edge Handler — C++ via WebAssembly ☁️

> **Educational Project**: A Cloudflare Workers project where all API logic is written in **C++**, compiled to **WebAssembly**, and executed at the edge. Includes a local Node.js simulator so you can run it without a Cloudflare account.

---

## ⚡ Quick Start

```bash
# Step 1: Compile C++ → WASM
chmod +x build.sh && ./build.sh

# Step 2: Start local edge simulator
node local_sim/server.js

# Step 3: Open the test UI
# http://localhost:8082
```

---

## 🏗️ Architecture

```
HTTP Request
     │
     ▼
┌─────────────────────────────────┐
│   Cloudflare Worker (JS)        │  ← worker/worker.js
│   addEventListener('fetch')     │
└──────────┬──────────────────────┘
           │  WebAssembly.instantiate('edge_handler.wasm')
           │  edgeHandle(method, path, query, body, ip)
           ▼
┌─────────────────────────────────┐
│   C++ WASM Module               │  ← src/api_handler.cpp
│   ├── Router (URL dispatch)     │  ← include/router.h
│   ├── RateLimiter (token bucket)│  ← include/rate_limiter.h
│   └── JsonBuilder               │  ← include/json_builder.h
└─────────────────────────────────┘
           │
           ▼
    JSON Response → HTTP Response
```

### Local Simulation
The same architecture runs locally via `local_sim/server.js` (Node.js), which uses the Node.js `WebAssembly` API to load the `.wasm` file and call the C++ functions directly.

---

## 📁 Directory Structure

```
cloudflare_edge/
├── src/
│   └── api_handler.cpp      # All C++ business logic (routes, handlers)
├── include/
│   ├── api_handler.h        # Exported WASM function declarations
│   ├── router.h             # URL router (Request/Response types)
│   ├── rate_limiter.h       # Token bucket rate limiter
│   └── json_builder.h       # Lightweight JSON string builder
├── worker/
│   ├── worker.js            # Real Cloudflare Worker JS
│   └── wrangler.toml        # CF deployment configuration
├── local_sim/
│   ├── server.js            # Local Node.js simulator (port 8082)
│   └── index.html           # Test UI (dark-themed API tester)
├── build/                   # Compiled WASM output (gitignored)
│   ├── edge_handler.js
│   └── edge_handler.wasm
├── build.sh                 # Emscripten build script
└── README.md
```

---

## 🌐 API Endpoints (Implemented in C++)

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/hello?name=X` | Returns a greeting JSON |
| `GET` | `/api/stats` | Returns request count and runtime info |
| `POST` | `/api/echo` | Echoes the request body back as JSON |
| `GET` | `/api/rate-limit` | Shows token bucket status for your IP |
| `GET` | `/api/hash?value=X` | Computes FNV-1a 32-bit hash in C++ |
| `GET` | `/api/fibonacci?n=N` | Computes fibonacci(N) in C++ (0–50) |

---

## 🔑 Key Concept: JS ↔ WASM String Passing

WebAssembly only natively passes **numbers** (`i32`, `f64`). Strings require manual memory management:

```
JavaScript Side                      WASM Linear Memory (ArrayBuffer)
─────────────────                    ────────────────────────────────
const str = "GET"
↓ encode to UTF-8
↓ malloc(4 bytes) in WASM  ───────►  [ G | E | T | \0 ]  @ ptr 1024
↓ pass ptr (integer 1024)
↓ call edgeHandle(1024, ...)
↓ C++ reads from ptr 1024  ◄───────  C++ dereferences const char* ptr
↓ C++ malloc's result string
↓ returns result ptr (integer 2048)  [ { | " | o | k | " ... } ]  @ ptr 2048
↓ JS reads from ptr 2048   ───────►  decode back to JS string
↓ call edgeFreeString(2048)          C++ free() at ptr 2048
```

This is implemented in `worker/worker.js` (`writeString`, `readString` functions).

---

## 🔒 Rate Limiting (C++ Token Bucket)

The C++ `RateLimiter` class implements a **token bucket** algorithm stored entirely in **WASM linear memory**:
- Max **10 tokens** per client IP
- Tokens refill at **1 per second**
- Exceeding the limit returns HTTP `429 Too Many Requests`
- State persists across requests within the same Cloudflare isolate lifetime

---

## 🚀 Deploy to Real Cloudflare

```bash
# Install Wrangler CLI
npm install -g wrangler

# Login to Cloudflare
wrangler login

# Edit worker/wrangler.toml — add your account_id and route

# Deploy (builds and uploads WASM + Worker JS)
cd worker && wrangler publish
```

The WASM binary is bundled with the Worker via the `[wasm_modules]` section in `wrangler.toml`. Cloudflare distributes it to **300+ edge locations worldwide** automatically.

---

## 📊 Why C++ on the Edge?

| Concern | JavaScript Worker | C++ WASM Worker |
|---|---|---|
| Compute speed | ~1x | 2–4x faster |
| Memory control | GC-managed | Manual (`malloc`/`free`) |
| Binary size | N/A | ~50–200KB |
| Cold start | <1ms | <5ms |
| Existing C++ libs | ❌ | ✅ (port via Emscripten) |
| Safety | ✅ Sandboxed | ✅ WASM sandbox |
