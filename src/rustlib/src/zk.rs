use regex::Regex;
use std::ffi::CString;
use std::process::{Command, Output};

use crate::log::log_print;

fn zk_convert_stdout(output: Output) -> Result<String, String> {
    let stdout = String::from_utf8(output.stdout).map_err(|err| err.to_string())?;
    if stdout.contains("error") || stdout.contains("failed") {
        Err(stdout)
    } else {
        Ok(stdout)
    }
}

fn zk_compile(path: &str) -> Result<String, String> {
    Command::new("zokrates")
        .current_dir(path)
        .arg("compile")
        .arg("--input")
        .arg("zk")
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))?
}

pub fn zk_setup(path: &str) -> Result<String, String> {
    Command::new("zokrates")
        .current_dir(path)
        .arg("setup")
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))?
}

fn zk_compute_witness(path: &str, params: &str) -> Result<String, String> {
    Command::new("zokrates")
        .current_dir(path)
        .arg("compute-witness")
        .arg("--arguments")
        .args(params.split(" "))
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))?
}

fn zk_generate_proof(path: &str) -> Result<String, String> {
    Command::new("zokrates")
        .current_dir(path)
        .arg("generate-proof")
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))?
}

fn zk_verify(path: &str) -> Result<String, String> {
    Command::new("zokrates")
        .current_dir(path)
        .arg("verify")
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))?
}

fn zk_proof(path: &str) -> Result<String, String> {
    let stdout = Command::new("zokrates")
        .current_dir(path)
        .arg("print-proof")
        .output()
        .map_err(|e| e.to_string())
        .map(|output| zk_convert_stdout(output))??;
    Ok(Regex::new(r"0x([0-9a-f]{64}).,.0x([0-9a-f]{64})")
        .unwrap()
        .captures_iter(&stdout)
        .map(|c| format!("{}{}", c[1].to_string(), c[2].to_string()))
        .collect::<Vec<_>>()
        .join(" "))
}

fn zk_proof_all(path: &str, params: &str) -> Result<String, String> {
    zk_compile(path)?;
    zk_compute_witness(path, params)?;
    zk_generate_proof(path)?;
    zk_verify(path)?;
    zk_proof(path)
}

pub fn zk_proof_tidy(path: &str, params: &str) -> CString {
    match zk_proof_all(path, params) {
        Ok(m) => CString::new(m).unwrap(),
        Err(e) => {
            log_print(e);
            CString::new("").unwrap()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_run() {
        let path = "/root/OurChain/ourTest/zk/ex1";
        let params = "2147483647 274876858367";
        match zk_proof_all(path, params) {
            Ok(msg) => println!("stdout: {}", msg),
            Err(e) => println!("error: {}", e),
        };
    }
}
