//! The gRPC server.
//!

use crate::{log, rpc::kv_store::*, SERVER_ADDR};
use anyhow::Result;
use tonic::{transport::Server, Request, Response, Status};
use std::collections::HashMap;
use tokio::sync::RwLock;
use std::sync::Arc;

pub struct KvStore { kv_store: Arc<RwLock<HashMap<Vec<u8>, Vec<u8>>>>,}

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
    async fn echo(
        &self,
        req: Request<EchoRequest>,
    ) -> Result<Response<EchoReply>, Status> {
        log::info!("Received echo request.");
        Ok(Response::new(EchoReply { 
            message: req.into_inner().message,
        }))
    }

     async fn put(
        &self,
        req: Request<PutRequest>,
    ) -> Result<Response<PutReply>, Status> {
        log::info!("Received put request.");
        let mut kv_store = self.kv_store.write().await;
        let req = req.into_inner();
        kv_store.insert(req.key.clone(), req.value.clone());
        Ok(Response::new(PutReply {}))
    }

    async fn get(
        &self,
        req: Request<GetRequest>,
    ) -> Result<Response<GetReply>, Status> {
        log::info!("Received get request.");
        let kv_store = self.kv_store.read().await;
        match kv_store.get(&req.into_inner().key) {
            Some(value) => {
                Ok(Response::new(GetReply { value: value.clone() }))
            }
            None => {
                Err(tonic::Status::new(tonic::Code::NotFound, "Key does not exist."))
            }
        }
    }
}

pub async fn start() -> Result<()> {
    //let svc = kv_store_server::KvStoreServer::new(KvStore {});
    let svc = kv_store_server::KvStoreServer::new(KvStore {kv_store: Arc::new(RwLock::new(HashMap::new())),});
    log::info!("Starting KV store server.");
    Server::builder()
        .add_service(svc)
        .serve(SERVER_ADDR.parse().unwrap())
        .await?;
    Ok(())
}

