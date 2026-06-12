/**
 * Local Cloudflare Edge Simulator — Node.js HTTP Server
 * Loads the Emscripten-generated edge_handler.js to run C++ WASM
 */

const http  = require('http');
const fs    = require('fs');
const path  = require('path');
const url   = require('url');

const PORT      = 8082;
const WASM_JS   = path.resolve(__dirname, '../build/edge_handler.js');

// ── Load Emscripten WASM Module ───────────────
function loadEmscriptenModule() {
    return new Promise((resolve, reject) => {
        if (!fs.existsSync(WASM_JS)) {
            return reject(new Error(`WASM not found: ${WASM_JS}\nRun ./build.sh first.`));
        }

        // Create a fresh Module config BEFORE requiring the generated JS
        const mod = {
            onRuntimeInitialized() { resolve(mod); },
            print:    (s) => {},
            printErr: (s) => console.error('[WASM]', s),
        };

        // Emscripten generated code reads Module from global in Node
        global.Module = mod;
        require(WASM_JS);
    });
}

// ── String helpers using Module's own helpers ─
function writeString(mod, str) {
    const len = mod.lengthBytesUTF8(str) + 1;
    const ptr = mod._malloc(len);
    mod.stringToUTF8(str, ptr, len);
    return ptr;
}
function readString(mod, ptr) {
    return mod.UTF8ToString(ptr);
}

// ── Start Server ──────────────────────────────
loadEmscriptenModule().then(mod => {
    // Initialize the C++ handler
    mod._edgeInit();

    console.log('✅ C++ WASM edge handler initialized');
    console.log(`🚀 Local CF Edge Simulator running at http://localhost:${PORT}`);
    console.log('\nAvailable endpoints:');
    [
        'GET  http://localhost:' + PORT + '/api/hello?name=World',
        'GET  http://localhost:' + PORT + '/api/stats',
        'POST http://localhost:' + PORT + '/api/echo',
        'GET  http://localhost:' + PORT + '/api/rate-limit',
        'GET  http://localhost:' + PORT + '/api/hash?value=hello',
        'GET  http://localhost:' + PORT + '/api/fibonacci?n=20',
    ].forEach(e => console.log(' ', e));
    console.log('\nTest UI: http://localhost:' + PORT + '/\n');

    const server = http.createServer((req, res) => {
        const parsed   = url.parse(req.url);
        const pathname = parsed.pathname;
        const query    = (parsed.search || '').replace(/^\?/, '');
        const clientIP = req.socket.remoteAddress || '127.0.0.1';

        // Test UI
        if (pathname === '/' || pathname === '/index.html') {
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(fs.readFileSync(path.join(__dirname, 'index.html')));
            return;
        }

        // API via C++ WASM
        if (pathname.startsWith('/api/')) {
            let body = '';
            req.on('data', c => body += c);
            req.on('end', () => {
                try {
                    const pMethod   = writeString(mod, req.method);
                    const pPath     = writeString(mod, pathname);
                    const pQuery    = writeString(mod, query);
                    const pBody     = writeString(mod, body);
                    const pClientIP = writeString(mod, clientIP);

                    // ── Call C++ ─────────────────────
                    const resultPtr    = mod._edgeHandle(pMethod, pPath, pQuery, pBody, pClientIP);
                    const responseJSON = readString(mod, resultPtr);

                    mod._edgeFreeString(resultPtr);
                    mod._free(pMethod);
                    mod._free(pPath);
                    mod._free(pQuery);
                    mod._free(pBody);
                    mod._free(pClientIP);

                    console.log(`  [${req.method}] ${pathname}${query?'?'+query:''}`);
                    console.log(`  → ${responseJSON.substring(0, 100)}`);

                    let status = 200;
                    try {
                        const p = JSON.parse(responseJSON);
                        if (p.error?.includes('Rate limit'))  status = 429;
                        if (p.error?.includes('Not Found'))   status = 404;
                        if (p.error?.includes('Missing'))     status = 400;
                    } catch (_) {}

                    res.writeHead(status, {
                        'Content-Type':                'application/json',
                        'X-Powered-By':                'C++ via WebAssembly',
                        'X-Request-Count':             mod._edgeGetRequestCount().toString(),
                        'Access-Control-Allow-Origin': '*',
                    });
                    res.end(responseJSON);

                } catch (err) {
                    console.error('Error:', err.message);
                    res.writeHead(500, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: err.message }));
                }
            });
        } else {
            res.writeHead(404, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Not found', path: pathname }));
        }
    });

    server.listen(PORT);
}).catch(err => {
    console.error('❌ Failed to load WASM:', err.message);
    process.exit(1);
});
