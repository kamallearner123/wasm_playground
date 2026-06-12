/**
 * Cloudflare Worker — JavaScript Entry Point
 *
 * This file IS the Cloudflare Worker. It:
 *   1. Loads the compiled C++ WASM module (edge_handler.wasm)
 *   2. Listens for incoming HTTP requests via addEventListener('fetch')
 *   3. Translates the Web Request object into C++ function calls
 *   4. Returns the C++ response as an HTTP Response
 *
 * Deploy with:
 *   wrangler publish
 *
 * ── How JS ↔ WASM string passing works ─────────────
 * WebAssembly only natively passes numbers (i32, f64).
 * To pass a string from JS → C++:
 *   1. Encode the string to UTF-8 bytes
 *   2. Write the bytes into WASM linear memory (Module.HEAPU8)
 *   3. Pass the memory pointer (integer) to the C++ function
 * To receive a string from C++ → JS:
 *   1. C++ returns a char* (an integer pointer into linear memory)
 *   2. JS reads bytes from Module.HEAPU8 starting at that pointer
 *   3. Decode back to a JS string
 *   4. Call edgeFreeString() to release the C++ heap allocation
 */

// ── WASM Module Instantiation ──────────────────────
// In real Cloudflare Workers, WASM is bundled via wrangler.toml:
//   [wasm_modules]
//   EDGE_HANDLER = "edge_handler.wasm"
// Then accessed as: const instance = await EDGE_HANDLER;
// Here we load it via fetch for local compatibility.

let wasmInstance = null;
let wasmMemory   = null;

async function initWasm() {
    if (wasmInstance) return;

    const response = await fetch('/edge_handler.wasm');
    const buffer   = await response.arrayBuffer();

    const result = await WebAssembly.instantiate(buffer, {
        env: {
            // Emscripten may need these imports
            memory: new WebAssembly.Memory({ initial: 256, maximum: 512 }),
            __stack_pointer: new WebAssembly.Global({ value: 'i32', mutable: true }, 1024 * 1024),
        },
        // Emscripten generated imports
        wasi_snapshot_preview1: {
            proc_exit: (code) => { console.log(`WASM exit: ${code}`); },
        }
    });

    wasmInstance = result.instance;
    wasmMemory   = wasmInstance.exports.memory;

    // Initialize the C++ edge handler
    wasmInstance.exports.edgeInit();
    console.log('[CF Worker] WASM module initialized — C++ edge handler ready');
}

// ── String Utilities ──────────────────────────────
// Write a JS string into WASM linear memory, return pointer
function writeString(str) {
    const encoded = new TextEncoder().encode(str + '\0');
    const ptr     = wasmInstance.exports.malloc(encoded.length);
    new Uint8Array(wasmMemory.buffer).set(encoded, ptr);
    return ptr;
}

// Read a null-terminated C string from WASM linear memory
function readString(ptr) {
    const heap = new Uint8Array(wasmMemory.buffer);
    let   end  = ptr;
    while (heap[end] !== 0) end++;
    return new TextDecoder().decode(heap.subarray(ptr, end));
}

// ── Main Worker Fetch Handler ─────────────────────
addEventListener('fetch', event => {
    event.respondWith(handleRequest(event.request));
});

async function handleRequest(request) {
    // Ensure WASM is loaded (cached after first call)
    await initWasm();

    const url      = new URL(request.url);
    const method   = request.method;
    const path     = url.pathname;
    const query    = url.search.replace('?', '');
    const clientIP = request.headers.get('CF-Connecting-IP') || '127.0.0.1';
    let   body     = '';

    if (method === 'POST' || method === 'PUT') {
        body = await request.text();
    }

    // ── JS → WASM: write strings into linear memory ──
    const pMethod   = writeString(method);
    const pPath     = writeString(path);
    const pQuery    = writeString(query);
    const pBody     = writeString(body);
    const pClientIP = writeString(clientIP);

    // ── Call C++ handler ──────────────────────────
    const resultPtr = wasmInstance.exports.edgeHandle(
        pMethod, pPath, pQuery, pBody, pClientIP
    );

    // ── WASM → JS: read result string from linear memory ──
    const responseBody = readString(resultPtr);

    // Free the C++ heap allocation
    wasmInstance.exports.edgeFreeString(resultPtr);

    // Free our input string allocations
    wasmInstance.exports.free(pMethod);
    wasmInstance.exports.free(pPath);
    wasmInstance.exports.free(pQuery);
    wasmInstance.exports.free(pBody);
    wasmInstance.exports.free(pClientIP);

    // Determine HTTP status from response body
    let status = 200;
    try {
        const parsed = JSON.parse(responseBody);
        if (parsed.error && responseBody.includes('Rate limit')) status = 429;
        if (parsed.error && responseBody.includes('Not Found'))  status = 404;
        if (parsed.error && responseBody.includes('Missing'))    status = 400;
    } catch (_) {}

    // ── Return HTTP Response ─────────────────────
    return new Response(responseBody, {
        status,
        headers: {
            'Content-Type':                'application/json',
            'X-Powered-By':                'C++ via WebAssembly',
            'X-Edge-Runtime':              'Cloudflare Workers',
            'X-Request-Count':             wasmInstance.exports.edgeGetRequestCount().toString(),
            'X-Rate-Limit-Remaining':      wasmInstance.exports.edgeGetRateRemaining(pClientIP).toString(),
            'Access-Control-Allow-Origin': '*',
        }
    });
}
