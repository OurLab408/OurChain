#[no_mangle]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 {
    println!("Hello from Rust!");
    a + b
}