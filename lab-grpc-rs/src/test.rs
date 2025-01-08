//! Basic tests.
//!
// Do not modify this file.

use crate::client::{echo, example, get, put};
use anyhow::Result;

#[tokio::test]
async fn test_example() -> Result<()> {
    assert_eq!(2, example(1).await?);
    assert_eq!(11, example(10).await?);
    assert_eq!(32, example(31).await?);
    Ok(())
}

#[tokio::test]
async fn test_echo() -> Result<()> {
    let msg = "CS 162 is the best class!".to_string();
    assert_eq!(msg, echo(msg.clone()).await?);
    Ok(())
}

#[tokio::test]
async fn test_kv_store() -> Result<()> {
    let (k1, v1) = ("test_key".to_string(), "test_value".to_string());
    let (k2, v2) = (
        vec![39, 127, 254, 0, 1, 2, 78],
        vec![1, 4, 0, 244, 3, 33, 68],
    );

    assert!(get(k1.as_bytes().into()).await.is_err());

    put(k1.as_bytes().into(), v1.as_bytes().into()).await?;
    assert_eq!(v1.as_bytes().to_vec(), get(k1.as_bytes().into()).await?);

    put(k2.clone(), v2.clone()).await?;
    assert_eq!(v2, get(k2).await?);
    Ok(())
}
