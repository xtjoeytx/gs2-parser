use std::env;
use std::path::PathBuf;
use cmake::Config;

// Constants for common library names
const DEBUG_SUFFIX_GS2: &str = "_d";

fn main() {
    let profile = env::var("PROFILE").unwrap_or_else(|_| "release".to_string());
    let target = env::var("TARGET").expect("TARGET environment variable not set");

    // Determine the appropriate library names based on the profile and platform
    let gs2_compiler_lib = library_name("gs2compiler", &profile, DEBUG_SUFFIX_GS2, &target);

    let mut cmake_config = Config::new(".");
    cmake_config.build_target("gs2compiler").define("STATIC", "ON");

    let cpp_lib = configure_platform_specifics(&target, &mut cmake_config);

    let lib_path = cmake_config.build();

    // Add the appropriate subdirectory (e.g., Debug or Release)
    let build_subdir = if profile == "debug" {
        "build/Debug"
    } else {
        "build/Release"
    };
    
    let full_lib_path = lib_path.join(build_subdir);

    link_libraries(full_lib_path, cpp_lib, &gs2_compiler_lib);
}

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
        "c++"
    } else if target.contains("windows") {
        cmake_config.define("WIN32", "ON");
        "msvcrt"
    } else {
        println!("cargo:rustc-link-arg=-lstdc++");
        "stdc++"
    }
}

fn link_libraries(lib_path: PathBuf, cpp_lib: &str, gs2_lib: &str) {
    println!("cargo:rustc-link-lib=dylib={}", cpp_lib);
    println!("cargo:rustc-link-search=native={}", lib_path.display());
    println!("cargo:rustc-link-lib=static={}", gs2_lib);
}
