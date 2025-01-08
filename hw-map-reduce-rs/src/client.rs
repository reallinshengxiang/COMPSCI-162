//! Functions for interacting with a MapReduce cluster.
//!
// Do not modify this file.

use crate::codec::LengthDelimitedWriter;
use crate::log;
use crate::rpc::coordinator::coordinator_client::CoordinatorClient;
use crate::rpc::coordinator::*;
use crate::*;
use futures::stream::StreamExt;
use std::time::Duration;
use tokio::fs;
use tokio_util::codec::{Framed, LengthDelimitedCodec};
use tonic::transport::Channel;
use tonic::Request;
use std::path::Path;

use anyhow::Result;

pub fn serialize_args(args: Vec<String>) -> Vec<u8> {
    let mut writer = LengthDelimitedWriter::new();

    for arg in args {
        writer.send(arg.into());
    }

    let buf = writer.finish().freeze();
    buf.as_ref().into()
}

pub async fn submit_job(
    client: &mut coordinator_client::CoordinatorClient<Channel>,
    files: Vec<String>,
    output_dir: String,
    app: String,
    n_reduce: u32,
    args: Vec<u8>,
) -> Result<u32> {
    fs::create_dir_all(&output_dir).await?;
    let path = fs::canonicalize(&output_dir).await?;
    let res = client
        .submit_job(SubmitJobRequest {
            files,
            output_dir: path
                .into_os_string()
                .into_string()
                .map_err(|_| anyhow::anyhow!("Failed to convert path to string"))?,
            app,
            n_reduce,
            args,
        })
        .await?;
    Ok(res.get_ref().job_id)
}

pub async fn poll_job(
    client: &mut coordinator_client::CoordinatorClient<Channel>,
    job_id: JobId,
) -> Result<PollJobReply> {
    let done = client
        .poll_job(Request::new(PollJobRequest { job_id }))
        .await?;
    Ok(done.into_inner())
}

pub async fn postprocess_job(output_dir: &str, app: &str, n_reduce: u32) -> Result<String> {
    let mut intermediate = Vec::new();
    for i in 0..n_reduce {
        let output_dir_path = Path::new(output_dir);
        let filename = output_dir_path.join(format!("mr-out-{}", i));
        log::debug!("Postprocessing file {}", filename.display());

        let file = tokio::fs::File::open(&filename).await?;
        let mut transport = Framed::new(file, LengthDelimitedCodec::new());
        while let Some(key) = transport.next().await {
            let key = key?;

            let value = match transport.next().await {
                Some(x) => x?,
                None => break,
            };

            intermediate.push(KeyValue {
                key: key.freeze(),
                value: value.freeze(),
            });
        }
    }

    let app = crate::app::named(app)?;
    let s = (app.process_output_fn)(Box::new(intermediate.into_iter()))?;
    Ok(s)
}

pub async fn connect() -> Result<CoordinatorClient<Channel>> {
    let client =
        coordinator_client::CoordinatorClient::connect(format!("http://{}", COORDINATOR_ADDR))
            .await?;
    Ok(client)
}

pub async fn wait_job(
    client: &mut coordinator_client::CoordinatorClient<Channel>,
    job_id: JobId,
) -> Result<PollJobReply> {
    let mut res;
    while {
        res = poll_job(client, job_id).await?;
        !(res.done || res.failed)
    } {
        tokio::time::sleep(Duration::from_millis(POLL_JOB_TIMEOUT_MS)).await;
    }

    Ok(res)
}
