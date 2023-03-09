// p is 21888242871839275222246405745257275088548364400416034343698204186575808495617
// need 254 bits = 32 bytes

extern crate num_bigint;
extern crate num_traits;
use num_bigint::BigUint;

const A: u32 = 168700;
const D: u32 = 168696;
pub const P_STR: &[u8; 77] = b"21888242871839275222246405745257275088548364400416034343698204186575808495617";
pub const S_STR: &[u8; 68] = b"81540058820840996586704275553141814055101440848469862132140264610111";

/// p = s * 2^e + 1, s is odd
pub fn p () -> BigUint {
  BigUint::parse_bytes(P_STR, 10).unwrap()
}

pub fn s () -> BigUint {
  BigUint::parse_bytes(S_STR, 10).unwrap()
}

pub fn e () -> u16 {
  28u16
}

pub fn exists (x: &BigUint, y: &BigUint) -> bool {
  let p = p();
  let two = BigUint::new([2u32].to_vec());
  let xx = x.modpow(&two, &p);
  let yy = y.modpow(&two, &p);
  (&xx * A + &yy) % &p == (&xx * &yy * D + 1u8) % &p 
}

/// axx + yy = 1 + dxxyy
/// yy = (axx-1)/(dxx-1)
pub fn get_yy (x: &BigUint) -> BigUint {
  let p = p();
  let two = BigUint::new([2u32].to_vec());
  let xx = x.modpow(&two, &p);
  (&xx * A - 1u8) * (&xx * D - 1u8).modpow(&(&p - 2u8), &p) % &p
}