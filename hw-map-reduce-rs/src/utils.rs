//! Utility functions that may be helpful for implementing
//! and testing MapReduce.
//!
// Do not modify this file.

use crate::coordinator::args::Args as CoordinatorArgs;
use crate::worker;
use crate::worker::args::Args as WorkerArgs;
use crate::{coordinator, COORDINATOR_STARTUP_MS};
use anyhow::Result;
use bytes::Bytes;
use std::time::Duration;

/// Read an entire [`Bytes`] slice into a [`String`].
///
/// Note that the entire slice will be read into the string.
/// It is the caller's responsibility to ensure the slice is
/// of the correct length. Failure to do so may result in
/// an error being returned, or the string data being incorrect.
///
/// Returns an error if the slice contains invalid UTF-8.
pub fn string_from_bytes(buf: Bytes) -> Result<String> {
    Ok(String::from_utf8(buf.as_ref().into())?)
}

/// Convert a [`String`] to [`Bytes`].
#[inline]
pub fn string_to_bytes(s: String) -> Bytes {
    Bytes::from(s)
}

/// Start a cluster with a single coordinator and
/// `n_workers` workers.
///
/// This should only be used for sanity-testing, as it
/// runs all services within the same process, and therefore
/// does not accurately reflect the autograder tests (or real life).
pub async fn start_cluster(n_workers: u32) {
    tokio::spawn(async {
        let _ = coordinator::start(CoordinatorArgs {}).await;
    });

    // Give time for the coordinator to start up
    tokio::time::sleep(Duration::from_millis(COORDINATOR_STARTUP_MS)).await;

    for _ in 0..n_workers {
        tokio::spawn(async {
            let _ = worker::start(WorkerArgs {}).await;
        });
    }
}
