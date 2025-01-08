//! The MapReduce autograder server.
//!
// Do not modify this file.

use anyhow::Result;
use map_reduce::log;
use map_reduce::log::init_logger;

#[tokio::main(flavor = "multi_thread")]
async fn main() -> Result<()> {
    init_logger();

    #[cfg(not(feature = "autograder"))]
    {
        log::error!("The autograder is not available here.");
        log::error!("To run the autograder, please commit and push your code to GitHub.");
        Ok(())
    }

    #[cfg(feature = "autograder")]
    {
        use clap::Parser;
        log::info!("Starting autograder");
        let args = map_reduce::autograder::server::args::Args::parse();
        map_reduce::autograder::server::start(args).await
    }
}
