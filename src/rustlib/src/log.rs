use chrono::Utc;
use std::fmt::Debug;
use std::fs::OpenOptions;
use std::io::Write;

fn timestamp() -> String {
    format!(
        "[{} (UTC)]",
        Utc::now().format("%Y-%m-%d %H:%M:%S").to_string()
    )
}

pub fn log_print(input: impl Debug) {
    let filename = "debug.log";
    let mut file = OpenOptions::new()
        .append(true)
        .create(true)
        .open(filename)
        .expect(&format!("error opening {}", filename));
    let text = format!("{} {:#?}\n", timestamp(), input);
    file.write(text.as_bytes())
        .expect(&format!("error writing to {}", filename));
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::thread;
    use std::time::Duration;

    #[test]
    fn test_log() {
        log_print("hello, ");
        thread::sleep(Duration::from_secs(2));
        log_print(["world", "!"]);
    }
}
