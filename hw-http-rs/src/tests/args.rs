use crate::args::Args;

#[test]
fn test_args_default() {
    let _ = Args::default();
}

#[test]
fn test_args_required_fields() {
    let _ = Args {
        files: "www2".to_string(),
        port: 9090,
        num_threads: 4,
        ..Default::default()
    };
}
