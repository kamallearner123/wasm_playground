// Import WASM modules
import initRust, { process_data, fast_hash, greet } from '../rust_wasm/pkg/rust_wasm.js';
import createCppWasmModule from '../cpp_wasm/out/math_core.js';

let cppModule = null;
let rustModuleLoaded = false;

// UI Elements
const cppStatus = document.getElementById('cpp-status');
const rustStatus = document.getElementById('rust-status');
const terminal = document.getElementById('pipeline-log');

function log(msg, type = 'log-info') {
    const p = document.createElement('p');
    p.className = type;
    p.textContent = `> ${msg}`;
    terminal.appendChild(p);
    terminal.scrollTop = terminal.scrollHeight;
}

// Initialize C++ Module
async function initCpp() {
    try {
        log('Initializing Emscripten C++ WASM...', 'log-cpp');
        cppModule = await createCppWasmModule({
            locateFile: (path) => `../cpp_wasm/out/${path}`
        });
        cppStatus.textContent = 'READY';
        cppStatus.className = 'status badge-ready';
        log('C++ Module Loaded Successfully', 'log-cpp');
    } catch (e) {
        cppStatus.textContent = 'ERROR';
        cppStatus.className = 'status badge-error';
        log(`C++ Init Error: ${e.message}`, 'log-error');
        console.error(e);
    }
}

// Initialize Rust Module
async function initializeRust() {
    try {
        log('Initializing Rust wasm-bindgen WASM...', 'log-rust');
        await initRust();
        rustModuleLoaded = true;
        rustStatus.textContent = 'READY';
        rustStatus.className = 'status badge-ready';
        log('Rust Module Loaded Successfully', 'log-rust');
    } catch (e) {
        rustStatus.textContent = 'ERROR';
        rustStatus.className = 'status badge-error';
        log(`Rust Init Error: ${e.message}`, 'log-error');
        console.error(e);
    }
}

// Run C++ Function
document.getElementById('btn-cpp-fib').addEventListener('click', () => {
    if (!cppModule) return alert("C++ Module not loaded yet!");
    
    const n = parseInt(document.getElementById('fib-input').value);
    
    const startTime = performance.now();
    // cwrap creates a JS wrapper for the C function
    const fibonacci = cppModule.cwrap('fibonacci', 'number', ['number']);
    const result = fibonacci(n);
    const endTime = performance.now();
    
    document.getElementById('cpp-result').textContent = result;
    document.getElementById('cpp-time').textContent = `${(endTime - startTime).toFixed(2)}ms`;
});

// Run Rust Function
document.getElementById('btn-rust-hash').addEventListener('click', () => {
    if (!rustModuleLoaded) return alert("Rust Module not loaded yet!");
    
    const text = document.getElementById('hash-input').value;
    
    const startTime = performance.now();
    // Call Rust functions exposed via wasm-bindgen directly!
    const hashResult = fast_hash(text);
    const processed = process_data(text);
    const endTime = performance.now();
    
    document.getElementById('rust-result').textContent = `Hash: ${hashResult}`;
    document.getElementById('rust-time').textContent = `${(endTime - startTime).toFixed(2)}ms`;
});

// Run Full Pipeline
document.getElementById('btn-pipeline').addEventListener('click', async () => {
    if (!cppModule || !rustModuleLoaded) {
        return log('Modules not ready for pipeline execution.', 'log-error');
    }
    
    log('--- PIPELINE START ---');
    log('Step 1: JS generating data payload...');
    const n = 35; // Compute fibonacci(35)
    
    log(`Step 2: C++ performing heavy compute (Fibonacci ${n})...`, 'log-cpp');
    const startCpp = performance.now();
    const fibonacci = cppModule.cwrap('fibonacci', 'number', ['number']);
    const cppResult = fibonacci(n);
    const timeCpp = (performance.now() - startCpp).toFixed(2);
    log(`C++ Result: ${cppResult} (Took ${timeCpp}ms)`, 'log-cpp');
    
    log('Step 3: Rust formatting and hashing result securely...', 'log-rust');
    const startRust = performance.now();
    const payloadStr = `Final_Result_${cppResult}_Validated`;
    const finalHash = fast_hash(payloadStr);
    const timeRust = (performance.now() - startRust).toFixed(2);
    log(`Rust Hash: ${finalHash} (Took ${timeRust}ms)`, 'log-rust');
    
    log(`--- PIPELINE COMPLETE (Total JS overhead included) ---`);
});

// Boot
window.addEventListener('DOMContentLoaded', () => {
    initCpp();
    initializeRust();
});
