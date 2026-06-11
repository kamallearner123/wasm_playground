use wasm_bindgen::prelude::*;
use web_sys::console;

#[wasm_bindgen]
pub fn greet(name: &str) -> String {
    format!("Hello, {}! Welcome to Rust WASM.", name)
}

#[wasm_bindgen]
pub fn process_data(data: &str) -> String {
    // A simple mock data processing step
    let result = data.to_uppercase();
    console::log_1(&"Data processed by Rust WASM!".into());
    result
}

#[wasm_bindgen]
pub fn fast_hash(input: &str) -> u32 {
    // Simple fast string hashing algorithm for demonstration
    let mut hash: u32 = 5381;
    for byte in input.bytes() {
        hash = hash.wrapping_mul(33).wrapping_add(byte as u32);
    }
    hash
}
