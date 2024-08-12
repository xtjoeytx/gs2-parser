use std::env;
use cmake::Config;

fn main() {
    // Determine the build profile
    let profile = env::var("PROFILE").unwrap();
    let gs2_compiler_lib = if profile == "debug" {
        "gs2compiler_d"
    } else {
        "gs2compiler"
    };

    let fmt_name = if profile == "debug" {
        "fmtd"
    } else {
        "fmt"
    };

    let mut dst = Config::new(".");
    dst.build_target("gs2compiler");
    dst.define("STATIC", "ON");

    let target = env::var("TARGET").unwrap();
    let cpp = if target.contains("apple") {
        "c++"
    } else if target.contains("windows-gnu") {
        "stdc++"
    } else {
        // We need to link to the C++ standard library like this on Linux
        println!("cargo:rustc-link-arg=-lstdc++");
        "stdc++"
    };

    println!("cargo:rustc-link-lib=dylib={}", cpp);
    
    let lib_path = dst.build();
    println!("cargo:rustc-link-search=native={}", lib_path.display());

    // Link to the C++ library

    // This is where the built library will be placed
    let lib_path = env::current_dir().unwrap().join("lib");
    println!("cargo:rustc-link-search=native={}", lib_path.display());

    // Specify the name of the library to link against
    println!("cargo:rustc-link-lib=static={}", gs2_compiler_lib);
    println!("cargo:rustc-link-lib=static={}", fmt_name);
}
