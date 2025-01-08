//! A MapReduce-compatible implementation of word count.
//!
// Do not modify this file.

use crate::utils::string_from_bytes;
use crate::*;
use anyhow::Result;
use bytes::{Buf, BufMut, Bytes, BytesMut};
use std::cmp::Ordering;
use std::fmt::Write;

pub fn map(kv: KeyValue, _aux: Bytes) -> MapOutput {
    let s = String::from_utf8(kv.value.as_ref().into())?;
    let words = s
        .split(|c: char| !c.is_alphabetic())
        .filter(|s| !s.is_empty())
        .map(|word| word.to_lowercase())
        .collect::<Vec<_>>();

    let mut key_buf = BytesMut::new();
    let mut value_buf = BytesMut::with_capacity(words.len() * 8);

    let iter = words.into_iter().map(move |word| {
        key_buf.put_slice(word.into_bytes().as_ref());
        value_buf.put_u64(1);

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
        let k = string_from_bytes(kv.key)?;
        let v = kv.value.get_u64();
        decoded.push((k, v));
    }

    decoded.sort_by(|(k1, v1), (k2, v2)| {
        let res = v1.cmp(v2);
        if res == Ordering::Equal {
            return k1.cmp(k2);
        }
        res
    });

    let mut s = String::new();
    for (k, v) in decoded {
        writeln!(&mut s, "{}\t{}", v, k)?;
    }

    Ok(s)
}
