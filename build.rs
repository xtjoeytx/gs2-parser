use std::env;
use std::path::Path;
use cmake::Config;
// use std::fs;

// Constants for common library names
const DEBUG_SUFFIX_GS2: &str = "_d";
const DEBUG_SUFFIX_FMT: &str = "d";
const LIB_DIR: &str = "lib";
// const PRECOMPILED_DIR: &str = "precompiled";

fn main() {
    let profile = env::var("PROFILE").unwrap_or_else(|_| "release".to_string());
    let target = env::var("TARGET").expect("TARGET environment variable not set");
    // let target_os_dir = get_target_os_dir(&target);

    let gs2_compiler_lib = library_name("gs2compiler", &profile, DEBUG_SUFFIX_GS2, &target);
    let fmt_lib = library_name("fmt", &profile, DEBUG_SUFFIX_FMT, &target);

    // Configure the CMake build
    let mut cmake_config = Config::new(".");
    let cpp_lib = configure_platform_specifics(&target, &mut cmake_config);

    // let precompiled_lib_path = env::current_dir().unwrap().join(PRECOMPILED_DIR).join(&target_os_dir);

    // if !env::var("CARGO_FEATURE_COMPILE_FROM_SOURCE").is_ok() {
    //     if !precompiled_lib_path.exists() {
    //         panic!("Precompiled library {} not found in {} directory. Please enable the 'compile_from_source' feature.", gs2_compiler_lib, precompiled_lib_path.display());
    //     }

    //     link_precompiled_libraries(&precompiled_lib_path, cpp_lib, &gs2_compiler_lib, &fmt_lib);
    // }

    cmake_config.build_target("gs2compiler").define("STATIC", "ON");
    let lib_path = cmake_config.build();

    link_libraries(&lib_path, cpp_lib, &gs2_compiler_lib, &fmt_lib);
    // copy_to_precompiled(&precompiled_lib_path);
}

// fn link_precompiled_libraries(precompiled_lib_path: &Path, cpp_lib: &str, gs2_lib: &str, fmt_lib: &str) {
//     println!("cargo:rustc-link-search=native={}", precompiled_lib_path.display());
//     println!("cargo:rustc-link-lib=static={}", gs2_lib);
//     println!("cargo:rustc-link-lib=static={}", fmt_lib);
//     println!("cargo:rustc-link-lib=dylib={}", cpp_lib);
// }

fn library_name(base: &str, profile: &str, debug_suffix: &str, target: &str) -> String {
    let name = if profile == "debug" {
        format!("{}{}", base, debug_suffix)
    } else {
        base.to_string()
    };

    if target.contains("windows") {
        format!("lib{}", name)
    } else {
        name
    }
}

fn configure_platform_specifics(target: &str, cmake_config: &mut Config) -> &'static str {
    if target.contains("apple") {
        cmake_config.define("CMAKE_OSX_DEPLOYMENT_TARGET", "10.13");
        "c++"
    } else if target.contains("windows") {
        "msvcrt"
    } else {
        println!("cargo:rustc-link-arg=-lstdc++");
        "stdc++"
    }
}

fn link_libraries(lib_path: &Path, cpp_lib: &str, gs2_lib: &str, fmt_lib: &str) {
    let lib_dir = env::current_dir().unwrap().join(LIB_DIR);
    println!("cargo:rustc-link-lib=dylib={}", cpp_lib);
    println!("cargo:rustc-link-search=native={}", lib_path.display());
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static={}", gs2_lib);
    println!("cargo:rustc-link-lib=static={}", fmt_lib);
}

// fn get_target_os_dir(target: &str) -> &'static str {
//     match target {
//         "aarch64-unknown-linux-gnu" => "linux-arm64",
//         "x86_64-unknown-linux-gnu" => "linux-x64",
//         "aarch64-apple-darwin" => "osx-arm64",
//         "x86_64-apple-darwin" => "osx-x64",
//         "x86_64-pc-windows-gnu" => "windows-x64",
//         _ => panic!("Unsupported target: {}", target),
//     }
// }

// fn copy_to_precompiled(precompiled_lib_path: &Path) {
//     let lib_dir = env::current_dir().unwrap().join(LIB_DIR);
//     fs::create_dir_all(precompiled_lib_path).expect("Failed to create precompiled directory");

//     for entry in fs::read_dir(lib_dir).expect("Failed to read lib directory") {
//         let entry = entry.expect("Failed to read directory entry");
//         let src_path = entry.path();
//         let file_name = src_path.file_name().expect("Failed to get file name");
//         let dst_path = precompiled_lib_path.join(file_name);
//         fs::copy(&src_path, &dst_path).expect("Failed to copy file");
//     }
// }