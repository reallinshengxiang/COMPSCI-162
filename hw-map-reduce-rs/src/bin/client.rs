//! A client for interacting with a MapReduce cluster.
//!
// Do not modify this file.

use std::time::Duration;

use map_reduce::client::*;
use map_reduce::rpc::coordinator::*;
use map_reduce::{log, log::init_logger, *};

use anyhow::Result;
use clap::{Parser, Subcommand};

/// A client for interacting with a MapReduce cluster.
#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
pub struct Args {
    #[clap(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    /// Submit a job to a MapReduce cluster.
    Submit {
        /// The list of input files.
        #[clap(value_parser)]
        files: Vec<String>,

        /// The directory in which output files should be stored.
        #[clap(short, long, value_parser)]
        output_dir: String,

        /// The name of the MapReduce application.
        #[clap(short, long, value_parser)]
        app: String,

        /// The number of reduce tasks.
        #[clap(short, long, default_value = "5", value_parser)]
        num_reduce: u32,

        /// Whether or not to wait until the job is finished.
        #[clap(short, long, default_value = "false", value_parser)]
        wait: bool,

        /// Auxiliary arguments to pass to the MapReduce application.
        #[clap(value_parser, last = true)]
        args: Vec<String>,
    },

    /// Poll a job running on a MapReduce cluster.
    Poll {
        /// The job ID.
        #[clap(value_parser)]
        job_id: JobId,
    },

    /// Post-process the output of a completed MapReduce job.
    Process {
        /// The directory in which output files were stored.
        #[clap(short, long, value_parser)]
        output_dir: String,

        /// The name of the MapReduce application.
        #[clap(short, long, value_parser)]
        app: String,

        /// The number of reduce tasks.
        #[clap(short, long, default_value = "5", value_parser)]
        num_reduce: u32,
    },
}

#[tokio::main]
async fn main() -> Result<()> {
    init_logger();
    let args = Args::parse();

    match args.command {
        Commands::Submit {
            files,
            output_dir,
            app,
            num_reduce,
            wait,
            args,
        } => {
            let mut client = coordinator_client::CoordinatorClient::connect(format!(
                "http://{}",
                COORDINATOR_ADDR
            ))
            .await?;
            log::info!(
                "Submitting job with output_dir = {} and app = {}",
                &output_dir,
                &app
            );

            let job_id = match submit_job(
                &mut client,
                files,
                output_dir,
                app,
                num_reduce,
                serialize_args(args),
            )
            .await
            {
                Ok(job_id) => job_id,
                Err(e) => {
                    log::error!("Failed to submit job: {:?}", e);
                    return Err(e);
                }
            };
            log::info!("Successfully submitted job {}", job_id);

            let mut res;
            if wait {
                while {
                    res = poll_job(&mut client, job_id).await?;
                    !(res.done || res.failed)
                } {
                    tokio::time::sleep(Duration::from_millis(POLL_JOB_TIMEOUT_MS)).await;
                }

                if res.failed {
                    log::error!("Job failed.");
                    for error in res.errors {
                        log::error!("ERROR: {}", error);
                    }
                } else {
                    assert!(res.done);
                    log::info!("SUCCESS: job {} completed.", job_id);
                }
            }
        }
        Commands::Poll { job_id } => {
            let mut client = coordinator_client::CoordinatorClient::connect(format!(
                "http://{}",
                COORDINATOR_ADDR
            ))
            .await?;
            let res = poll_job(&mut client, job_id).await?;
            if res.failed {
                log::error!("Job {} failed.", job_id);
                for error in res.errors {
                    log::error!("{}", error);
                }
            } else if res.done {
                log::info!("Job {} successfully completed.", job_id);
            } else {
                log::info!("Job {} is in progress.", job_id);
            }
        }
        Commands::Process {
            output_dir,
            app,
            num_reduce,
        } => {
            let s = postprocess_job(&output_dir, &app, num_reduce).await?;
            print!("{}", s);
        }
    }

    Ok(())
}
