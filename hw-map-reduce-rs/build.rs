/// Compiles protocol buffer code using [`tonic_build`].
fn main() {
    // Ignore errors (the directory may already exist)
    let _ = std::fs::create_dir("src/proto");

    #[cfg(feature = "autograder")]
    let _ = std::fs::create_dir("src/autograder/rpc/");

    tonic_build::configure()
        .out_dir("src/proto")
        .compile(
            &["proto/coordinator.proto", "proto/worker.proto"],
            &["proto/"],
        )
        .unwrap_or_else(|e| panic!("Failed to compile protos: {:?}", e));

    #[cfg(feature = "autograder")]
    tonic_build::configure()
        .out_dir("src/autograder/rpc")
        .compile(
            &["src/autograder/proto/grader.proto"],
            &["src/autograder/proto/"],
        )
        .unwrap_or_else(|e| panic!("Failed to compile autograder protos: {:?}", e));
}
