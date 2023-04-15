use mypoint::MyPoint;
use std::ffi::{CStr, CString};

use crate::mypoint;

pub fn encrypt(point: &CStr, scalar: &CStr) -> CString {
    let p = point.to_str().unwrap();
    let s = scalar.to_str().unwrap();
    let sk = MyPoint::from_secret(s);
    let pk = if p == "0" {
        MyPoint::b()
    } else {
        MyPoint::from_public(p)
    };
    let pk = pk.mul_scalar(&sk.s);
    CString::new(pk.compress_public()).unwrap()
}

pub fn random() -> CString {
    let random_point = MyPoint::rand();
    let s = random_point.compress_secret();
    let p = random_point.compress_public();
    CString::new(s + &p).unwrap()
}

#[cfg(test)]
mod tests {
    use mypoint::MyPoint;
    extern crate num_bigint;
    use num_bigint::{BigInt, RandBigInt};
    use num_traits::Signed;

    use crate::mypoint;

    #[test]
    fn diffie_hellman() {
        let sk1 = BigInt::from(12345);
        let sk2 = BigInt::from(54321);
        let dh = MyPoint::from(BigInt::from(12345 * 54321));
        let key1 = MyPoint::from(sk1);
        let key2 = MyPoint::from(sk2);
        let p12 = key1.mul_scalar(&key2.s);
        let p21 = key2.mul_scalar(&key1.s);

        assert_eq!(p12, p21);
        assert_eq!(p12, dh);

        let sk3 = rand::thread_rng().gen_bigint(32).abs();
        let sk4 = rand::thread_rng().gen_bigint(32).abs();
        let dh = MyPoint::from(&sk3 * &sk4);
        let key3 = MyPoint::from(sk3);
        let key4 = MyPoint::from(sk4);
        let p34 = key3.mul_scalar(&key4.s);
        let p43 = key4.mul_scalar(&key3.s);

        assert_eq!(p34, p43);
        assert_eq!(p34, dh);
    }

    #[test]
    fn import_export() {
        let sk1 = MyPoint::from(12345);
        let ex_s = sk1.compress_secret();
        let ex_p = sk1.compress_public();
        println!("sec: {}\npub: {}", &ex_s, &ex_p);

        let sk2 = MyPoint::from_secret(&ex_s[..]);
        let sk3 = MyPoint::from_public(&ex_p[..]);
        assert_eq!(sk1, sk2);
        assert_eq!(sk1, sk3);
    }
}
