//! A MapReduce worker.
//!
// Do not modify this file.

use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Arc;

use crate::codec::LengthDelimitedReader;
use crate::codec::LengthDelimitedWriter;
use crate::ihash;
use crate::log;
use crate::*;
use anyhow::Context;
use tokio::io::AsyncReadExt;
use tokio::io::AsyncWriteExt;
use tokio::sync::Mutex;
use tokio::time::Duration;
use tonic::transport::{Channel, Server};
use tonic::Code;
use tonic::Request;
use tonic::Response;
use tonic::Status;
use std::path::Path;

use crate::rpc::coordinator::*;
use crate::rpc::worker::*;

use anyhow::{bail, Result};
use itertools::Itertools;

pub mod args;

pub struct WorkerState {
    data: HashMap<JobId, HashMap<(TaskNumber, TaskNumber), Bytes>>,
    // Add your own fields
}

#[derive(Clone)]
pub struct Worker {
    id: u32,
    client: coordinator_client::CoordinatorClient<Channel>,
    inner: Arc<Mutex<WorkerState>>,
    // Add your own fields
}

impl Worker {
    pub async fn new() -> Result<Self> {
        // Connect to the coordinator
        let mut client =
            coordinator_client::CoordinatorClient::connect(format!("http://{}", COORDINATOR_ADDR))
                .await?;
        let res = client.register(RegisterRequest {}).await?;
        Ok(Self {
            id: res.get_ref().worker_id,
            client,
            inner: Arc::new(Mutex::new(WorkerState {
                data: HashMap::new(),
            })),
        })
    }

    async fn read_map_from(&self, worker: WorkerId, req: ReadMapRequest) -> Result<ReadMapReply> {
        let mut map_worker = connect(worker).await?;
        let res = map_worker.read_map(req).await?.into_inner();
        Ok(res)
    }

    // Returns an `Option<String>`. If `Some(s)`, there was a retryable error;
    // s is the error message. If `None`, reduce completed succesfully.
    // If `Err(_)`, an non-retryable error occurred, and the job should be aborted.
    async fn reduce(
        &mut self,
        reduce_fn: ReduceFn,
        job_id: u32,
        task: u32,
        map_tasks: Vec<MapTaskAssignment>,
        output_dir: &str,
        aux: Bytes,
    ) -> Result<Option<String>> {
        log::info!(
            "Worker {} received reduce task {} for job id {}",
            self.id,
            task,
            job_id
        );

        let mut intermediate = Vec::new();

        for map_task in map_tasks.into_iter() {
            let res = match self
                .read_map_from(
                    map_task.worker_id,
                    ReadMapRequest {
                        job_id,
                        map_task: map_task.task,
                        reduce_task: task,
                    },
                )
                .await
            {
                Ok(res) => res,
                Err(e) => return Ok(Some(e.to_string())),
            };

            log::info!(
                "Worker {} reducing map task {} for job id: {}",
                self.id,
                map_task.task,
                job_id
            );

            let buf = Bytes::from(res.result);
            let mut reader = LengthDelimitedReader::new(buf);

            while let Some(key) = reader.next() {
                let value = match reader.next() {
                    Some(x) => x,
                    None => bail!("Found key without a value"),
                };

                intermediate.push(KeyValue { key, value });
            }
        }

        intermediate.sort_unstable_by_key(KeyValue::key);

        let mut writer = LengthDelimitedWriter::new();

        for (key, group) in &intermediate.into_iter().group_by(KeyValue::key) {
            let iter = group.map(KeyValue::into_value);
            let output = reduce_fn(key.clone(), Box::new(iter), aux.clone())?;

            writer.send(key);
            writer.send(output);
        }

        let buf = writer.finish().freeze();
        let output_dir_path = Path::new(output_dir);
        let filename = output_dir_path.join(format!("mr-out-{}", task));
        let mut out_file = tokio::fs::File::create(&filename).await?;
        out_file.write_all(&buf).await?;

        Ok(None)
    }

    async fn map(
        &mut self,
        map_fn: MapFn,
        job_id: u32,
        task: u32,
        n_reduce: u32,
        file: String,
        aux: Bytes,
    ) -> Result<Option<String>> {
        let mut state = self.inner.lock().await;

        log::info!("Worker {} received map task {}", self.id, task);

        let mut writers = Vec::with_capacity(n_reduce as usize);
        for _ in 0..n_reduce {
            let writer = LengthDelimitedWriter::new();
            writers.push(writer);
        }

        let mut content = Vec::new();

        {
            let mut file = tokio::fs::File::open(&file)
                .await
                .with_context(|| format!("Failed to open input file {}", &file))?;
            file.read_to_end(&mut content).await?;
        }

        let content = Bytes::from(content);

        let kv = KeyValue {
            key: Bytes::from(file),
            value: content,
        };

        for item in map_fn(kv, aux)? {
            let KeyValue { key, value } = item?;
            let reduce_idx = ihash(&key) % n_reduce;

            let writer = writers.get_mut(reduce_idx as usize).unwrap();
            writer.send(key);
            writer.send(value);
        }

        let job_entry = state.data.entry(job_id).or_insert_with(HashMap::new);

        for (i, writer) in writers.into_iter().enumerate() {
            job_entry.insert((task as TaskNumber, i), writer.finish().freeze());
        }

        Ok(None)
    }

    /// Handles a task. Returns true if the worker should request another task,
    /// and false if the worker should exit.
    async fn handle_task(&mut self, reply: GetTaskReply) -> Result<()> {
        if reply.wait {
            log::info!("Worker {} waiting {} ms", self.id, WAIT_TIME_MS);
            tokio::time::sleep(Duration::from_millis(WAIT_TIME_MS)).await;
            return Ok(());
        }

        let app = crate::app::named(&reply.app)?;

        let res = if reply.reduce {
            self.reduce(
                app.reduce_fn,
                reply.job_id,
                reply.task,
                reply.map_task_assignments,
                &reply.output_dir,
                Bytes::from(reply.args),
            )
            .await
            .with_context(|| format!("Failed to perform reduce task {}", reply.task))
        } else {
            self.map(
                app.map_fn,
                reply.job_id,
                reply.task,
                reply.n_reduce,
                reply.file.to_owned(),
                Bytes::from(reply.args),
            )
            .await
            .with_context(|| format!("Failed to perform map task {}", reply.task))
        };

        let mut fail_req = FailTaskRequest {
            task: reply.task,
            worker_id: self.id,
            job_id: reply.job_id,
            reduce: reply.reduce,
            ..Default::default()
        };

        let fail = match res {
            Err(e) => {
                fail_req.error = format!("{:#}", e);
                fail_req.retry = false;
                true
            }
            Ok(Some(msg)) => {
                fail_req.error = msg;
                fail_req.retry = true;
                true
            }
            Ok(None) => false,
        };

        if fail {
            log::error!("Worker {} failed processing task {}", self.id, reply.task);
            self.client.fail_task(fail_req).await?;
            tokio::time::sleep(Duration::from_millis(500)).await;
        } else {
            log::info!("Worker {} done processing task {}", self.id, reply.task);
            self.client
                .finish_task(FinishTaskRequest {
                    worker_id: self.id,
                    job_id: reply.job_id,
                    task: reply.task,
                    reduce: reply.reduce,
                })
                .await?;
        }

        Ok(())
    }

    pub async fn run(mut self) -> Result<()> {
        let mut client_clone = self.client.clone();
        tokio::spawn(async move {
            loop {
                log::info!("Worker {} sending hearbeat", self.id);
                let _ = client_clone
                    .heartbeat(HeartbeatRequest { worker_id: self.id })
                    .await;
                tokio::time::sleep(Duration::from_millis(HEARTBEAT_INTERVAL_MS)).await;
            }
        });

        loop {
            log::info!("Worker {} requesting task", self.id);
            let res = self
                .client
                .get_task(GetTaskRequest { worker_id: self.id })
                .await?;

            if let Err(e) = self.handle_task(res.into_inner()).await {
                log::info!(
                    "Worker {} failed to complete task, waiting {} ms: {:?}",
                    self.id,
                    WAIT_TIME_MS,
                    e,
                );
                tokio::time::sleep(Duration::from_millis(WAIT_TIME_MS)).await;
            }
        }
    }
}

impl WorkerState {
    fn get_data(
        &self,
        job_id: JobId,
        map_task: TaskNumber,
        reduce_task: TaskNumber,
    ) -> Option<Bytes> {
        let task_map = self.data.get(&job_id)?;
        let bytes = task_map.get(&(map_task, reduce_task))?;
        Some(bytes.clone())
    }
}

#[tonic::async_trait]
impl worker_server::Worker for Worker {
    async fn read_map(
        &self,
        req: Request<ReadMapRequest>,
    ) -> Result<Response<ReadMapReply>, Status> {
        let req = req.into_inner();
        let state = self.inner.lock().await;
        let data = state.get_data(
            req.job_id,
            req.map_task as TaskNumber,
            req.reduce_task as TaskNumber,
        );
        if data.is_none() {
            return Err(Status::new(
                Code::InvalidArgument,
                "worker does not have data for the given task",
            ));
        }
        let data = data.unwrap();

        Ok(Response::new(ReadMapReply {
            result: data.as_ref().into(),
        }))
    }

    async fn remove_job(
        &self,
        req: Request<RemoveJobRequest>,
    ) -> Result<Response<RemoveJobReply>, Status> {
        let req = req.into_inner();
        let mut state = self.inner.lock().await;
        state.data.remove(&req.job_id);
        Ok(Response::new(RemoveJobReply {}))
    }
}

async fn worker_server(worker: Worker) -> Result<()> {
    let addr = get_addr(worker.id);
    let svc = worker_server::WorkerServer::new(worker);
    Server::builder().add_service(svc).serve(addr).await?;

    Ok(())
}

pub async fn start(_args: args::Args) -> Result<()> {
    let worker = Worker::new().await?;

    let server = worker.clone();
    tokio::spawn(async move { worker_server(server).await });

    worker.run().await?;

    Ok(())
}

fn get_port(id: WorkerId) -> u16 {
    let port = INITIAL_WORKER_PORT as WorkerId + id;
    assert!(port <= u16::MAX as WorkerId);
    port as u16
}

fn get_addr(id: WorkerId) -> SocketAddr {
    format!("127.0.0.1:{}", get_port(id)).parse().unwrap()
}

async fn connect(id: WorkerId) -> Result<worker_client::WorkerClient<Channel>> {
    let client =
        worker_client::WorkerClient::connect(format!("http://127.0.0.1:{}", get_port(id))).await?;
    Ok(client)
}
