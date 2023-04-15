use ::hex::ToHex;
use std::{
    convert::TryInto,
    hash::{Hash, Hasher},
};
extern crate ff;
use ff::*;
extern crate num_bigint;
use num_bigint::BigInt;
use num_traits::{Num, One, Zero};
extern crate babyjubjub_rs;
use babyjubjub_rs::*;

#[derive(Clone, Debug)]
pub struct MyPoint {
    pub p: Point,  // 32 bytes to represent
    pub s: BigInt, // 0 means unknown
}

impl PartialEq for MyPoint {
    fn eq(&self, other: &Self) -> bool {
        self.p.x == other.p.x && self.p.y == other.p.y
        // && self.s == other.s
    }
}

impl Eq for MyPoint {}

impl Hash for MyPoint {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.p.y.hash(state);
    }
}

impl From<Point> for MyPoint {
    fn from(p: Point) -> Self {
        MyPoint { p, s: Zero::zero() }
    }
}

impl From<PrivateKey> for MyPoint {
    fn from(pk: PrivateKey) -> Self {
        MyPoint {
            p: pk.public(),
            s: pk.scalar_key(),
        }
    }
}

impl From<i32> for MyPoint {
    fn from(sk: i32) -> Self {
        MyPoint::from(BigInt::from(sk))
    }
}

impl From<BigInt> for MyPoint {
    fn from(sk: BigInt) -> Self {
        MyPoint {
            p: MyPoint::b().p.mul_scalar(&sk),
            s: sk,
        }
    }
}

impl From<&String> for MyPoint {
    fn from(comp: &String) -> Self {
        MyPoint {
            p: match hex::decode(comp) {
                Ok(v) => match decompress_point(v.try_into().unwrap()) {
                    Ok(u) => u,
                    Err(_) => MyPoint::zero().p,
                },
                Err(_) => MyPoint::zero().p,
            },
            s: Zero::zero(),
        }
    }
}

impl MyPoint {
    /// order
    pub fn order() -> BigInt {
        BigInt::from_str_radix(
            "2736030358979909402780800718157159386076813972158567259200215660948447373041",
            10,
        )
        .unwrap()
    }
    /// base point
    pub fn b() -> Self {
        let h = "8b7d2d877a253c4b7733e1b91f05e0fcedf96bd11c2e572549b2a0f703727925";
        MyPoint {
            p: MyPoint::from_public(h).p,
            s: One::one(),
        }
    }
    /// infinity point
    pub fn zero() -> Self {
        MyPoint {
            p: Point {
                x: Fr::zero(),
                y: Fr::one(),
            },
            s: Zero::zero(),
        }
    }
    pub fn rand() -> Self {
        MyPoint::from(new_key())
    }
    pub fn mul_scalar(&self, n: &BigInt) -> Self {
        MyPoint {
            p: self.p.mul_scalar(n),
            s: Zero::zero(),
        }
    }
    /// hex format. length = 64.
    pub fn compress_public(&self) -> String {
        self.p.compress().encode_hex::<String>()
    }
    /// hex format. length = 64.
    pub fn compress_secret(&self) -> String {
        format!("{:0>64}", self.s.to_str_radix(16))
    }
    pub fn from_public(s: &str) -> Self {
        let mut decoded = [0; 32];
        match hex::decode_to_slice(s, &mut decoded) {
            Ok(_) => MyPoint::from(decompress_point(decoded).unwrap()),
            Err(_) => MyPoint::zero(),
        }
    }
    pub fn from_secret(s: &str) -> Self {
        MyPoint::from(BigInt::from_str_radix(s, 16).unwrap())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base_point() {
        let base_x = Fr::from_str(
            "5299619240641551281634865583518297030282874472190772894086521144482721001553",
        )
        .unwrap();
        let base_y = Fr::from_str(
            "16950150798460657717958625567821834550301663161624707787222815936182638968203",
        )
        .unwrap();
        assert_eq!(MyPoint::b().p.x, base_x);
        assert_eq!(MyPoint::b().p.y, base_y);
    }

    #[test]
    fn test_order() {
        let zero = MyPoint::b().mul_scalar(&MyPoint::order());
        assert_eq!(MyPoint::zero(), zero);
    }

    #[test]
    fn test_invalid_point() {
        let pt = MyPoint::from_public(&"0");
        assert_eq!(MyPoint::zero(), pt);
        let pt = MyPoint::from_public(&"hello");
        assert_eq!(MyPoint::zero(), pt);
    }
}
