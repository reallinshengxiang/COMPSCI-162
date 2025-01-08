//! The gRPC server.
//!

use crate::{log, rpc::kv_store::*, SERVER_ADDR};
use anyhow::Result;
use tonic::{transport::Server, Request, Response, Status};

pub struct KvStore {}

#[tonic::async_trait]
impl kv_store_server::KvStore for KvStore {
    async fn example(
        &self,
        req: Request<ExampleRequest>,
    ) -> Result<Response<ExampleReply>, Status> {
        log::info!("Received example request.");
        Ok(Response::new(ExampleReply {
            output: req.into_inner().input + 1,
        }))
    }

    // TODO: RPC implementation
}

pub async fn start() -> Result<()> {
    let svc = kv_store_server::KvStoreServer::new(KvStore {});

    log::info!("Starting KV store server.");
    Server::builder()
        .add_service(svc)
        .serve(SERVER_ADDR.parse().unwrap())
        .await?;
    Ok(())
}
