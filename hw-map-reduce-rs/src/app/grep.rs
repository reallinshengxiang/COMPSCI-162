//! A MapReduce-compatible implementation of `grep`.
//!
// Do not modify this file.

use crate::{
    codec::{LengthDelimitedReader, LengthDelimitedWriter},
    *,
};
use anyhow::Result;
use bytes::{Buf, BufMut, Bytes, BytesMut};
use clap::Parser;
use serde::{Deserialize, Serialize};
use std::fmt::Write;
use std::path::Path;

use crate::utils::*;

#[derive(Parser, Debug, Serialize, Deserialize)]
#[clap(no_binary_name = true)]
struct Args {
    #[clap(short, long, value_parser)]
    term: String,
}

fn deserialize_args(buf: Bytes) -> Result<Args> {
    let reader = LengthDelimitedReader::new(buf);
    let mut args: Vec<String> = Vec::new();
    for arg in reader {
        args.push(string_from_bytes(arg)?);
    }

    let args = Args::try_parse_from(args)?;
    Ok(args)
}

#[allow(clippy::needless_collect)]
pub fn map(kv: KeyValue, aux: Bytes) -> MapOutput {
    let term = deserialize_args(aux).unwrap().term;

    let s = String::from_utf8(kv.value.as_ref().into())?;
    let lines = s
        .lines()
        .enumerate()
        .filter(|(_, s)| s.contains(&term))
        .map(|(i, s)| (i + 1, s.to_string()))
        .collect::<Vec<_>>();

    let mut key_buf = BytesMut::new();
    let mut value_buf = BytesMut::new();

    let iter = lines.into_iter().map(move |(line_num, line)| {
        key_buf.put(kv.key.as_ref());
        value_buf.put_u64(line_num as u64);
        value_buf.put(line.as_bytes());

        let key = key_buf.split().freeze();
        let value = value_buf.split().freeze();

        Ok(KeyValue { key, value })
    });
    Ok(Box::new(iter))
}

pub fn reduce(
    _key: Bytes,
    values: Box<dyn Iterator<Item = Bytes> + '_>,
    _aux: Bytes,
) -> Result<Bytes> {
    let mut writer = LengthDelimitedWriter::new();

    let mut values = values.collect::<Vec<Bytes>>();
    values.sort_by_key(|value| value.clone().get_u64());
    for value in values {
        writer.send(value);
    }

    Ok(writer.finish().freeze())
}

pub fn process_output(kvs: Box<dyn Iterator<Item = KeyValue>>) -> Result<String> {
    let mut s = String::new();
    let mut kva = kvs.collect::<Vec<_>>();
    kva.sort_by_key(KeyValue::key);

    for KeyValue { key, value } in kva {
        let file_path = string_from_bytes(key)?;
        let file_name = Path::new(&file_path)
            .file_name()
            .ok_or_else(|| anyhow::anyhow!("Failed to get file name"))?
            .to_str()
            .ok_or_else(|| anyhow::anyhow!("Failed to convert OS string to str"))?;
        let reader = LengthDelimitedReader::new(value);
        s.push_str(file_name);
        s.push_str(":\n");
        for mut value in reader {
            let line_num = value.get_u64();
            let line = string_from_bytes(value)?;
            writeln!(&mut s, "\t{}: {}", line_num, line)?;
        }
    }
    Ok(s)
}
