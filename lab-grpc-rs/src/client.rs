//! The gRPC client.
//!

use crate::{rpc::kv_store::*, SERVER_ADDR};
use anyhow::Result;
use tonic::transport::Channel;

async fn connect() -> Result<kv_store_client::KvStoreClient<Channel>> {
    Ok(kv_store_client::KvStoreClient::connect(format!("http://{}", SERVER_ADDR)).await?)
}

// Client methods. DO NOT MODIFY THEIR SIGNATURES.
pub async fn example(input: u32) -> Result<u32> {
    let mut client = connect().await?;
    Ok(client
        .example(ExampleRequest { input })
        .await?
        .into_inner()
        .output)
}
pub async fn echo(msg: String) -> Result<String> {
    todo!("TODO: Issuing RPCs")
}
pub async fn put(key: Vec<u8>, value: Vec<u8>) -> Result<()> {
    todo!("TODO: KV store")
}
pub async fn get(key: Vec<u8>) -> Result<Vec<u8>> {
    todo!("TODO: KV store")
}
