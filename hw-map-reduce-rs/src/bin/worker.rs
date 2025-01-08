//! The MapReduce worker.
//!
// Do not modify this file.

use anyhow::Result;
use clap::Parser;
use map_reduce::{log::init_logger, worker};

#[tokio::main(flavor = "multi_thread")]
async fn main() -> Result<()> {
    init_logger();
    let args = worker::args::Args::parse();
    worker::start(args).await
}
