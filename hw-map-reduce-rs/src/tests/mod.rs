//! Basic MapReduce tests.
//!
// Feel free to modify this file;
// tests here are NOT run by the autograder.

use std::time::Duration;

use crate::client::*;
use crate::rpc::coordinator::*;
use crate::utils::start_cluster;
use crate::COORDINATOR_ADDR;
use anyhow::Result;

/// An example test that you can run locally.
/// This test starts a cluster of 4 workers,
/// submits a word count job to the coordinator,
/// and waits for the job to complete.
/// It then prints the final output from running word count.
///
/// The autograder runs more detailed tests.
/// This test likely won't pass until you implement task execution.
#[tokio::test]
#[ignore]
async fn test_wc() -> Result<()> {
    start_cluster(4).await;

    let mut client =
        coordinator_client::CoordinatorClient::connect(format!("http://{}", COORDINATOR_ADDR))
            .await?;

    let infile = todo!("Pick an input file");
    let output_dir = "/tmp/hw-map-reduce/test-wc".to_string();
    let job_id = submit_job(
        &mut client,
        vec![infile.clone()],
        output_dir.clone(),
        "wc".to_string(),
        5,
        vec![],
    )
    .await?;

    let mut res;
    while {
        res = poll_job(&mut client, job_id).await?;
        !(res.done || res.failed)
    } {
        tokio::time::sleep(Duration::from_secs(1)).await;
    }

    assert!(res.done && !res.failed);

    let s = postprocess_job(&output_dir, "wc", 5)
        .await
        .expect("failed to postprocess");

    println!("{}", s);

    Ok(())
}
