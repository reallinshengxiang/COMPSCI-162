//! A fault-tolerant MapReduce system.
//!
//! Users can specify map and reduce tasks, and then distribute
//! those tasks to workers running on a MapReduce cluster.
//!
//! Copyright CS 162 staff.
//!
// Do not modify this file.

#![forbid(unsafe_code)]

use bytes::Bytes;
use std::hash::Hasher;

pub mod app;

#[cfg(feature = "autograder")]
pub mod autograder;

pub mod client;
pub mod codec;
pub mod coordinator;
pub mod log;
pub mod task;
pub mod utils;
pub mod worker;

/// A type alias for task numbers.
///
/// Although task numbers are [`usize`]s, you may assume
/// they are never more than [`u32::MAX`], and therefore
/// can be safely converted to [`u32`]s.
pub type TaskNumber = usize;

/// A type alias for worker IDs.
pub type WorkerId = u32;

/// A type alias for job IDs.
pub type JobId = u32;

#[cfg(test)]
mod tests;

/// Automatically generated utilities for RPCs.
pub mod rpc {
    /// Includes RPC definitions for the coordinator.
    pub mod coordinator {
        include!("proto/coordinator.rs");
    }
    /// Includes RPC definitions for the worker.
    pub mod worker {
        include!("proto/worker.rs");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Coordinator constants
/////////////////////////////////////////////////////////////////////////////

/// The (hardcoded) address at which the coordinator should listen.
pub const COORDINATOR_ADDR: &str = "127.0.0.1:10162";

/// Start assigning worker IDs from this number.
pub const INITIAL_WORKER_ID: WorkerId = 0;

/// Start assigning job IDs from this number.
pub const INITIAL_JOB_ID: JobId = 0;

/// If `TASK_TIMEOUT_SECS` have passed since a worker's last heartbeat,
/// the worker's tasks should be reassigned to other workers.
pub const TASK_TIMEOUT_SECS: u64 = 7;

/// The maximum allowable time it can take for the coordinator to start up.
///
/// The autograder will assume that after `COORDINATOR_STARTUP_MS` milliseconds
/// have passed from starting the coordinator, the coordinator is fully initialized
/// and ready to service requests.
pub const COORDINATOR_STARTUP_MS: u64 = 1000;

/////////////////////////////////////////////////////////////////////////////
// Worker constants
/////////////////////////////////////////////////////////////////////////////

/// The initial buffer size workers should use for temporary buffers.
pub const BUF_SIZE: usize = 4096;

/// If instructed to wait, workers should wait `WAIT_TIME_MS` milliseconds
/// before requesting another task from the coordinator.
pub const WAIT_TIME_MS: u64 = 1000;

/// How often workers should send heartbeats to the coordinator.
pub const HEARTBEAT_INTERVAL_MS: u64 = 2000;

/// The port that worker 0 should use.
///
/// Worker 1 should listen on port `INITIAL_WORKER_PORT + 1`,
/// worker 2 should listen on port `INITIAL_WORKER_PORT + 2`, and so on.
pub const INITIAL_WORKER_PORT: u16 = 10163;

/// How long MapReduce clients should between RPC calls to `PollJob`.
pub const POLL_JOB_TIMEOUT_MS: u64 = 250;

/// The maximum allowable time it can take for a worker to start up.
///
/// The autograder will assume that after `WORKER_STARTUP_MS` milliseconds
/// have passed from starting a worker, the worker is fully initialized
/// and ready to receive tasks from the coordinator.
pub const WORKER_STARTUP_MS: u64 = 500;

/////////////////////////////////////////////////////////////////////////////
// MapReduce application types
/////////////////////////////////////////////////////////////////////////////

/// The output of an application map function.
///
/// There are 2 layers of [`anyhow::Result`]s here. The outer layer
/// accounts for errors that arise while creating the iterator.
/// The inner layer accounts for errors that occur during iteration.
///
/// This accomodates both batch (all keys emitted at once) and lazy
/// (keys only emitted when the iterator is consumed) map operations.
pub type MapOutput = anyhow::Result<Box<dyn Iterator<Item = anyhow::Result<KeyValue>>>>;

/// A map function takes a key-value pair and auxiliary arguments.
///
/// It returns an iterator that yields new key-value pairs.
pub type MapFn = fn(kv: KeyValue, aux: Bytes) -> MapOutput;

/// A reduce function takes in a key, an iterator over values for that key,
/// and an auxiliary argument. It returns an [`anyhow::Result`]
/// containing a single output value.
pub type ReduceFn = fn(
    key: Bytes,
    values: Box<dyn Iterator<Item = Bytes> + '_>,
    aux: Bytes,
) -> anyhow::Result<Bytes>;

/// The process output function takes in an iterator over all key-value pairs,
/// and formats them into a string for easy viewing and autograding.
///
/// Not a part of the traditional MapReduce framework.
pub type ProcessOutputFn = fn(kvs: Box<dyn Iterator<Item = KeyValue>>) -> anyhow::Result<String>;

/// A map reduce application.
#[derive(Copy, Clone)]
pub struct App {
    pub map_fn: MapFn,
    pub reduce_fn: ReduceFn,
    pub process_output_fn: ProcessOutputFn,
}

/////////////////////////////////////////////////////////////////////////////
// Key-value pairs
/////////////////////////////////////////////////////////////////////////////

/// A single key-value pair.
#[derive(Clone, Eq, PartialEq, Hash)]
pub struct KeyValue {
    /// The key.
    pub key: Bytes,

    /// The value.
    pub value: Bytes,
}

impl KeyValue {
    /// Construct a new key-value pair from the given key and value.
    pub fn new(key: Bytes, value: Bytes) -> Self {
        Self { key, value }
    }

    /// Get the key of this key-value pair.
    ///
    /// This method is cheap, since [`Bytes`] are cheaply cloneable.
    #[inline]
    pub fn key(&self) -> Bytes {
        self.key.clone()
    }

    /// Get the value of this key-value pair.
    ///
    /// This method is cheap, since [`Bytes`] are cheaply cloneable.
    #[inline]
    pub fn value(&self) -> Bytes {
        self.value.clone()
    }

    /// Consumes the key-value pair and returns the key.
    #[inline]
    pub fn into_key(self) -> Bytes {
        self.key
    }

    /// Consumes the key-value pair and returns the value.
    #[inline]
    pub fn into_value(self) -> Bytes {
        self.value
    }
}

/// Hashes an intermediate key. Compute a reduce bucket for a given key
/// by calculating `ihash(key) % n_reduce`.
pub fn ihash(key: &[u8]) -> u32 {
    let mut hasher = fnv::FnvHasher::with_key(0);
    hasher.write(key);
    let value = hasher.finish() & 0x7fffffff;
    u32::try_from(value).expect("Failed to compute ihash of value")
}
