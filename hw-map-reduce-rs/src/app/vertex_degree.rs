//! A MapReduce-compatible application that computes the
//! degree of each vertex in a graph, given a list of edges.
//!
// Do not modify this file.

use crate::utils::string_from_bytes;
use crate::*;
use anyhow::{anyhow, Result};
use bytes::{Buf, BufMut, Bytes, BytesMut};
use std::fmt::Write;

fn parse_line(line: &str) -> Result<(u64, u64)> {
    let mut iter = line.split_whitespace().take(2);
    let a = iter
        .next()
        .ok_or_else(|| anyhow!("Invalid input file format"))?
        .parse()?;
    let b = iter
        .next()
        .ok_or_else(|| anyhow!("Invalid input file format"))?
        .parse()?;
    Ok((a, b))
}

pub fn map(kv: KeyValue, _aux: Bytes) -> MapOutput {
    let s = string_from_bytes(kv.value)?;
    let edges = s.lines().map(parse_line).collect::<Result<Vec<_>>>()?;

    let mut key_buf = BytesMut::with_capacity(8 * edges.len());
    let mut value_buf = BytesMut::with_capacity(8 * edges.len());

    let iter = edges.into_iter().flat_map(move |(a, b)| {
        key_buf.put_u64(a);
        value_buf.put_u64(1);
        let key1 = key_buf.split().freeze();
        let value1 = value_buf.split().freeze();

        key_buf.put_u64(b);
        value_buf.put_u64(1);
        let key2 = key_buf.split().freeze();
        let value2 = value_buf.split().freeze();

        [
            Ok(KeyValue {
                key: key1,
                value: value1,
            }),
            Ok(KeyValue {
                key: key2,
                value: value2,
            }),
        ]
    });
    Ok(Box::new(iter))
}

pub fn reduce(
    _key: Bytes,
    values: Box<dyn Iterator<Item = Bytes> + '_>,
    _aux: Bytes,
) -> Result<Bytes> {
    let mut count = 0u64;

    for mut value in values {
        count += value.get_u64();
    }

    let mut value = BytesMut::with_capacity(8);
    value.put_u64(count);
    Ok(value.freeze())
}

pub fn process_output(kva: Box<dyn Iterator<Item = KeyValue>>) -> Result<String> {
    let mut decoded = Vec::with_capacity(kva.size_hint().0);

    for mut kv in kva {
        let vertex = kv.key.get_u64();
        let degree = kv.value.get_u64();
        decoded.push((vertex, degree));
    }

    decoded.sort_by_key(|tup| tup.0);

    let mut s = String::new();
    for (k, v) in decoded {
        writeln!(&mut s, "{}\t{}", k, v)?;
    }

    Ok(s)
}
