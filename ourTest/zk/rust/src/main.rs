use std::env;
extern crate num_bigint;
use num_bigint::BigUint;
extern crate num_traits;
use num_traits::{Zero, One};
pub mod bjj;
    
fn legendre_symbol (a: BigUint) -> bool {
    let p = bjj::p();
    if a > p { panic!(); }
    a.modpow(&(&p / 2u8), &p).is_one()
}

/// find x where x^2 = a (mod p)
/// use Tonelli-Shanks algorithm
/// return 0 if no solution
/// https://asecuritysite.com/primes/modsq?aval=209&pval=1223
fn modular_sqrt (a: BigUint) -> BigUint {
    if a.is_zero() { return a; }
    if !legendre_symbol(a.clone()) { return BigUint::zero(); }
    let p = bjj::p();
    let s = bjj::s();
    let e = bjj::e();

    let two = BigUint::new([2u32].to_vec());
    let mut n = two.clone();
    while legendre_symbol(n.clone()) {
        n += 1u8;
    }

    let mut b = a.modpow(&s, &p);
    let mut g = n.modpow(&s, &p);
    let mut x = a.modpow(&((s + 1u8) / 2u8), &p);
    let mut r = e;
    loop {
        let mut t = b.clone();
        let mut m = 0;
        let mut pow = BigUint::one();
        while m < r {
            if t.is_one() { break; } 
            t = t.modpow(&two, &p);
            m += 1;
        }
        if m == 0 { 
            return x;
        }
        for _ in m..r-1 {
            pow *= &two; 
        }

        let gs = g.modpow(&pow, &p);
        g = gs.modpow(&two, &p);
        x = (x * &gs) % &p;
        b = (b * &g) % &p;
        r = m;
    }
}

/// return odd y, 0 if invalid
fn get_y (x: &BigUint) -> BigUint {
    let yy = bjj::get_yy(&x);
    let y = modular_sqrt(yy.clone());
    if y.bit(0) || y.is_zero() { y } else { bjj::p() - y }
}

fn main () {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        println!("The amount of arguments should be exactly 1.");
        return;
    }
    let mut digits = args[1].as_bytes();
    let mut radix = 10u32;
    if digits.starts_with(b"0x") {
        digits = digits.split_at(2).1;
        radix = 16u32;
    }
    let mut x = BigUint::parse_bytes(digits, radix).unwrap();
    let mut y = get_y(&x);
    while y.is_zero() {
        x += 1u8;
        y = get_y(&x);
    }
    // println!("({x}, {y}) -> {}", bjj::exists(&x, &y));
    println!("({x}, {y})");
}