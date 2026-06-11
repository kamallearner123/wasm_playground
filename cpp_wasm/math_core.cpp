#include <emscripten/emscripten.h>
#include <vector>

// Export function to C so that Emscripten can export it to JS
extern "C" {

    // Simple recursive fibonacci for demonstrating performance
    EMSCRIPTEN_KEEPALIVE
    int fibonacci(int n) {
        if (n <= 1) return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
    }

    // A function to sum elements in an array (demonstrating pointers and memory)
    EMSCRIPTEN_KEEPALIVE
    int sum_array(int* ptr, int length) {
        int sum = 0;
        for (int i = 0; i < length; i++) {
            sum += ptr[i];
        }
        return sum;
    }
}
