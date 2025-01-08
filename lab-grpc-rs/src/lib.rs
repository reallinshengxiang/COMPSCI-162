//! A basic gRPC client/server example.
//!
//! Copyright CS 162 staff.
//!
// Do not modify this file.

pub mod client;
pub mod log;
pub mod rpc;
pub mod server;
#[cfg(test)]
pub mod test;

pub const SERVER_ADDR: &str = "127.0.0.1:10162";
