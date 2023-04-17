pub mod bjj;
pub mod zk;
pub mod log;
pub mod mypoint;
pub mod ym;
use std::{
    ffi::{CStr, CString},
    os::raw::c_char,
};

#[no_mangle]
pub extern "C" fn get_zk_proof(folder: *const c_char, params: *const c_char) -> *mut c_char {
    let folder = unsafe { CStr::from_ptr(folder).to_str().unwrap() };
    let params = unsafe { CStr::from_ptr(params).to_str().unwrap() };
    zk::zk_proof_tidy(folder, params).into_raw()
}

#[no_mangle]
pub extern "C" fn encrypt_bjj(point: *const c_char, scalar: *const c_char) -> *mut c_char {
    let scalar = unsafe { CStr::from_ptr(scalar) };
    let point = unsafe { CStr::from_ptr(point) };
    bjj::encrypt(point, scalar).into_raw()
}

/** private key (64 * 4bits) + public key (64 * 4bits) in hex */
#[no_mangle]
pub extern "C" fn random_bjj() -> *mut c_char {
    bjj::random().into_raw()
}

/**  */
#[no_mangle]
pub extern "C" fn ym_encode_0(n: u64) -> *mut c_char {
    ym::s0(n).into_raw()
}

/**  */
#[no_mangle]
pub extern "C" fn ym_encode_1(n: u64) -> *mut c_char {
    ym::s1(n).into_raw()
}

/** call this to char* returned from rust to free the space. */
#[no_mangle]
pub extern "C" fn free_str(s: *mut c_char) {
    unsafe { CString::from_raw(s) };
}
