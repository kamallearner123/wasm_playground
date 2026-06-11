use std::env;
use std::fs;
use std::time::SystemTime;

fn main() {
    println!("=== WASI Environment Execution ===");
    
    // Demonstrate WASI capability: Accessing environment variables
    let user = env::var("WASI_USER").unwrap_or_else(|_| "Unknown User".to_string());
    println!("Hello, {}! Running securely outside the browser.", user);
    
    // Demonstrate WASI capability: System Time
    match SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
        Ok(n) => println!("Current Unix Timestamp: {}", n.as_secs()),
        Err(_) => println!("SystemTime before UNIX EPOCH!"),
    }

    // Demonstrate WASI capability: File System Access
    let file_path = "wasi_test.txt";
    println!("Attempting to read file: {}", file_path);
    
    match fs::read_to_string(file_path) {
        Ok(contents) => {
            println!("File contents successfully read:");
            println!("--------------------------------");
            println!("{}", contents.trim());
            println!("--------------------------------");
        }
        Err(e) => {
            println!("Failed to read file: {}. (This is expected if the directory isn't mapped to the WASI sandbox!)", e);
        }
    }
    
    println!("=== Execution Complete ===");
}
