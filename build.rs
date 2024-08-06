use std::env;

use cmake::Config;

fn main() {
    // Determine the build profile
    let profile = env::var("PROFILE").unwrap();
    let lib_name = if profile == "debug" {
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
    let build = dst.build();

    // Link to the C++ library
    println!("cargo:rustc-link-lib=dylib=c++");

    // Specify the directory where the built library is located
    // let lib_path = build.join("lib");
    let lib_path = env::current_dir().unwrap().join("lib");
    println!("cargo:rustc-link-search=native={}", lib_path.display());

    // Also include libfmt at ./build/dependencies/fmtlib/libfmtd.a
    // let lib_path = build
    //     .join("build")
    //     .join("dependencies")
    //     .join("fmtlib");

    // Print the cargo instructions to link the library
    // println!("cargo:rustc-link-search=native={}", lib_path.display());


    // Specify the name of the library to link against
    println!("cargo:rustc-link-lib=static={}", lib_name);
    println!("cargo:rustc-link-lib=static={}", fmt_name);
}