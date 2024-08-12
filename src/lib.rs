extern crate libc;

use libc::{c_char, c_void};
use std::ffi::CStr;

#[repr(C)]
pub struct Gs2CompilerResult {
    pub success: bool,
    pub err_msg: *const c_char,
    pub bytecode: *mut u8,
    pub bytecode_size: u32,
}

extern {
    fn get_context() -> *mut c_void;
    fn compile_code_no_header(context: *mut c_void, code: *const c_char) -> Gs2CompilerResult;
    fn delete_context(context: *mut c_void);
}

/// Custom error type for the Gs2Context
#[derive(Debug)]
pub struct Gs2CompilerError {
    message: String,
}

impl Gs2CompilerError {
    pub fn new(message: &str) -> Self {
        Self {
            message: message.to_string(),
        }
    }
}

impl std::fmt::Display for Gs2CompilerError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.message)
    }
}

pub struct Gs2Context {
    context: *mut c_void,
}

impl Gs2Context {
    pub fn new() -> Self {
        unsafe {
            Gs2Context {
                context: get_context(),
            }
        }
    }

    pub fn compile_code(&self, code: &str) -> Result<Vec<u8>, Gs2CompilerError> {
        let c_code = std::ffi::CString::new(code).unwrap();

        unsafe {
            let response = compile_code_no_header(self.context, c_code.as_ptr());

            if response.success {
                let bytecode = std::slice::from_raw_parts(response.bytecode, response.bytecode_size as usize).to_vec();
                Ok(bytecode)
            } else {
                let err_msg = if response.err_msg.is_null() {
                    String::from("Unknown error")
                } else {
                    CStr::from_ptr(response.err_msg).to_string_lossy().into_owned()
                };
                Err(Gs2CompilerError::new(&err_msg))
            }
        }
    }
}

impl Drop for Gs2Context {
    fn drop(&mut self) {
        unsafe {
            delete_context(self.context);
        }
    }
}