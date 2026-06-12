/**
 * Cloudflare Edge Handler — C++ Implementation
 *
 * This file implements the core API logic that runs on Cloudflare's
 * edge network as a WebAssembly module.
 *
 * Compilation flow:
 *   C++ (this file) → Emscripten → edge_handler.wasm
 *   Cloudflare Worker (worker.js) loads edge_handler.wasm and calls:
 *     - edgeInit()   on startup (once per isolate)
 *     - edgeHandle() on every incoming HTTP request
 */

#include "api_handler.h"
#include "router.h"
#include "rate_limiter.h"
#include "json_builder.h"
#include <emscripten/emscripten.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>

// ─────────────────────────────────────────────
// Global state (lives in WASM linear memory)
// ─────────────────────────────────────────────
static Router*      gRouter      = nullptr;
static RateLimiter* gRateLimiter = nullptr;
static int          gRequestCount= 0;
static float        gUptimeStart = 0.0f;

// ─────────────────────────────────────────────
// Utility: FNV-1a hash (fast, no stdlib)
// ─────────────────────────────────────────────
static uint32_t fnv1a(const std::string& s) {
    uint32_t h = 2166136261u;
    for (unsigned char c : s) {
        h ^= c;
        h *= 16777619u;
    }
    return h;
}

// ─────────────────────────────────────────────
// Utility: parse query param
// ─────────────────────────────────────────────
static std::string getQueryParam(const std::string& query, const std::string& key) {
    // Looks for "key=value" in a query string
    std::string search = key + "=";
    auto pos = query.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    auto end = query.find('&', pos);
    return query.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

// ─────────────────────────────────────────────
// Route Handlers (each returns a Response)
// ─────────────────────────────────────────────

// GET /api/hello
static Response handleHello(const Request& req) {
    std::string name = getQueryParam(req.query, "name");
    if (name.empty()) name = "World";
    return {200,
        JsonBuilder()
            .add("message", "Hello, " + name + "! 👋 From C++ on the Edge.")
            .add("edge_location", "Simulated CF Edge Node")
            .add("runtime", "WebAssembly (C++ via Emscripten)")
            .build()
    };
}

// GET /api/stats
static Response handleStats(const Request& req) {
    (void)req;
    return {200,
        JsonBuilder()
            .add("total_requests", gRequestCount)
            .add("runtime", "WebAssembly")
            .add("language", "C++20")
            .add("compiled_with", "Emscripten")
            .add("rate_limit_max", gRateLimiter->maxTokens())
            .add("status", "healthy")
            .build()
    };
}

// POST /api/echo
static Response handleEcho(const Request& req) {
    return {200,
        JsonBuilder()
            .add("method", req.method)
            .add("path", req.path)
            .add("client_ip", req.clientIP)
            .add("body_length", (int)req.body.size())
            .add("echo", req.body.empty() ? "(empty)" : req.body)
            .build()
    };
}

// GET /api/rate-limit
static Response handleRateLimit(const Request& req) {
    int remaining = gRateLimiter->remaining(req.clientIP);
    bool limited  = remaining == 0;
    return {limited ? 429 : 200,
        JsonBuilder()
            .add("client_ip",   req.clientIP)
            .add("remaining",   remaining)
            .add("limit",       gRateLimiter->maxTokens())
            .add("rate_limited", limited)
            .add("policy", "10 requests per 10 seconds (token bucket)")
            .build()
    };
}

// GET /api/hash?value=...
static Response handleHash(const Request& req) {
    std::string val = getQueryParam(req.query, "value");
    if (val.empty()) {
        return {400, JsonBuilder().add("error", "Missing ?value= query param").build()};
    }
    uint32_t hash = fnv1a(val);
    std::ostringstream hex;
    hex << std::hex << hash;
    return {200,
        JsonBuilder()
            .add("input",     val)
            .add("algorithm", "FNV-1a 32-bit")
            .add("hash_dec",  (int)hash)
            .add("hash_hex",  "0x" + hex.str())
            .add("computed_in", "C++ WASM")
            .build()
    };
}

// GET /api/fibonacci?n=...
static Response handleFibonacci(const Request& req) {
    std::string nStr = getQueryParam(req.query, "n");
    int n = nStr.empty() ? 10 : atoi(nStr.c_str());
    if (n < 0 || n > 50) {
        return {400, JsonBuilder().add("error", "n must be between 0 and 50").build()};
    }
    long long a = 0, b = 1;
    for (int i = 0; i < n; i++) { long long t = a + b; a = b; b = t; }
    return {200,
        JsonBuilder()
            .add("n",        n)
            .add("result",   (int)a)
            .add("computed_in", "C++ WASM")
            .build()
    };
}

// ─────────────────────────────────────────────
// Exported WASM Functions (called by worker.js)
// ─────────────────────────────────────────────
extern "C" {

EMSCRIPTEN_KEEPALIVE
void edgeInit() {
    if (gRouter) return; // already initialized
    gRouter      = new Router();
    gRateLimiter = new RateLimiter(10, 1.0f); // 10 tokens, 1/sec refill
    gRequestCount= 0;

    // Register all routes
    gRouter->add("GET",  "/api/hello",     handleHello);
    gRouter->add("GET",  "/api/stats",     handleStats);
    gRouter->add("POST", "/api/echo",      handleEcho);
    gRouter->add("GET",  "/api/rate-limit",handleRateLimit);
    gRouter->add("GET",  "/api/hash",      handleHash);
    gRouter->add("GET",  "/api/fibonacci", handleFibonacci);
}

EMSCRIPTEN_KEEPALIVE
const char* edgeHandle(
    const char* method,
    const char* path,
    const char* query,
    const char* body,
    const char* clientIP)
{
    if (!gRouter) edgeInit();
    gRequestCount++;

    // Build request struct
    Request req;
    req.method   = method   ? method   : "GET";
    req.path     = path     ? path     : "/";
    req.query    = query    ? query    : "";
    req.body     = body     ? body     : "";
    req.clientIP = clientIP ? clientIP : "0.0.0.0";

    // Apply rate limiting
    float now = (float)gRequestCount; // simplified clock (real: use emscripten_get_now())
    bool  ok  = gRateLimiter->allow(req.clientIP, now);
    if (!ok) {
        std::string resp = JsonBuilder()
            .add("error",   "Rate limit exceeded")
            .add("client_ip", req.clientIP)
            .add("retry_after", 1)
            .build();
        char* out = (char*)malloc(resp.size() + 1);
        strcpy(out, resp.c_str());
        return out;
    }

    // Dispatch to router
    Response res = gRouter->dispatch(req);

    // Return heap-allocated string (JS must call edgeFreeString)
    char* out = (char*)malloc(res.body.size() + 1);
    strcpy(out, res.body.c_str());
    return out;
}

EMSCRIPTEN_KEEPALIVE
void edgeFreeString(const char* ptr) {
    free((void*)ptr);
}

EMSCRIPTEN_KEEPALIVE
int edgeGetRequestCount() {
    return gRequestCount;
}

EMSCRIPTEN_KEEPALIVE
int edgeGetRateRemaining(const char* clientIP) {
    if (!gRateLimiter || !clientIP) return 0;
    return gRateLimiter->remaining(std::string(clientIP));
}

} // extern "C"
