/**
 * Case 1: Pure C++ math module compiled to WebAssembly
 *
 * KEY CONCEPT:
 *   This .cpp file is compiled directly to a .wasm binary.
 *   The browser downloads math.wasm via fetch(), and the
 *   browser's built-in WebAssembly runtime executes it.
 *   No server-side execution. No Node.js. Pure client-side WASM.
 *
 * Build: emcc src/math.cpp -O2 -s WASM=1 --no-entry
 *        -s STANDALONE_WASM=1 ... -o math.wasm
 */

// No #include needed — pure computation only.
// Using stdlib would add OS-call imports; we want a self-contained .wasm.

extern "C" {

// ── Calculator ──────────────────────────────────
int   add(int a, int b)        { return a + b; }
int   subtract(int a, int b)   { return a - b; }
int   multiply(int a, int b)   { return a * b; }
double divide(double a, double b) { return b != 0.0 ? a / b : 0.0; }

double power(double base, int exp) {
    double result = 1.0;
    bool   neg    = exp < 0;
    if (neg) exp = -exp;
    for (int i = 0; i < exp; i++) result *= base;
    return neg ? 1.0 / result : result;
}

int modulo(int a, int b) { return b != 0 ? a % b : 0; }

// ── Fibonacci ───────────────────────────────────
// Iterative — O(n), safe for n up to 70 with long long
long long fibonacci(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long t = a + b;
        a = b;
        b = t;
    }
    return b;
}

// Recursive — demonstrates call stack in WASM (use small N)
long long fibonacci_recursive(int n) {
    if (n <= 1) return n;
    return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2);
}

// ── Bonus: Primes & Factorial ───────────────────
int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return 0;
    return 1;
}

long long factorial(int n) {
    if (n <= 0) return 1;
    long long r = 1;
    for (int i = 2; i <= n; i++) r *= i;
    return r;
}

} // extern "C"
