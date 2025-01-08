//! The gRPC server.
//!
// Do not modify this file.

use anyhow::Result;
use lab_grpc::{log::init_logger, server::start};

#[tokio::main]
async fn main() -> Result<()> {
    init_logger();
    start().await
}
