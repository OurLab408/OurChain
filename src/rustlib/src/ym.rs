use std::ffi::CString;
extern crate sha256;
use sha256::digest;

extern crate rand;
use rand::Rng;

// assume 45, so we accept tx of 1~2^45-1 satoshis
const COIN_LEN: usize = 6;

pub fn s0s1(set_type: u8, mut n: u64) -> [String; COIN_LEN] {
    let mut s = [0u64; COIN_LEN];
    let mut mask = 1u64;
    for i in 0..COIN_LEN {
        s[i] = if ((n & mask) == 0u64) != (set_type == 0) {
            // TODO: should check if repeated dummy values
            rand::thread_rng().gen::<u64>()
        } else if set_type == 0 {
            n | mask
        } else {
            n
        };
        n &= !mask;
        mask <<= 1;
    }
    // TODO: randomly permute
    s.map(|u| digest(digest(&u.to_le_bytes())))
}

pub fn s0(n: u64) -> CString {
    let s = s0s1(0, n).join("");
    CString::new(s).unwrap()
}

pub fn s1(n: u64) -> CString {
    let s = s0s1(1, n).join("");
    CString::new(s).unwrap()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_s0() {
        println!("{:?}", s0s1(0, 10));
    }
    #[test]
    fn test_s1() {
        println!("{:?}", s0s1(1, 15));
    }
}
