//! Utilities for logging.
//!
// Do not modify this file.

#[cfg(not(test))]
pub use log::{debug, error, info, trace, warn};

#[cfg(test)]
pub use std::{
    println as error, println as warn, println as info, println as debug, println as trace,
};

#[inline]
pub fn init_logger() {
    #[cfg(not(test))]
    env_logger::builder()
        .filter(None, log::LevelFilter::Info)
        .format_timestamp(None)
        .init();
}
