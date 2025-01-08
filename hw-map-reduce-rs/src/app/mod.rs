//! Converts MapReduce application names to actual application code.
//!
//! # Example
//!
//! To get the word count application:
//! ```
//! # use anyhow::Result;
//! // This is the correct import to use if you are outside the crate:
//! use map_reduce::app;
//! // Since you will be working within the `map_reduce` crate,
//! // you should write `use crate::app;` instead.
//! # fn main() -> Result<()> {
//! let wc = app::named("wc")?;
//! # Ok(())
//! # }
//! ```
// Do not modify this file.

use crate::App;
use anyhow::{bail, Result};

pub mod grep;
pub mod vertex_degree;
pub mod wc;

/// Gets the [`App`] named `name`.
///
/// Returns [`None`] if no application with the given name was found.
pub fn try_named(name: &str) -> Option<App> {
    #[cfg(feature = "autograder")]
    if let Some(app) = crate::autograder::app::named(name) {
        return Some(app);
    }

    match name {
        "wc" => Some(App {
            map_fn: wc::map,
            reduce_fn: wc::reduce,
            process_output_fn: wc::process_output,
        }),
        "grep" => Some(App {
            map_fn: grep::map,
            reduce_fn: grep::reduce,
            process_output_fn: grep::process_output,
        }),
        "vertex-degree" => Some(App {
            map_fn: vertex_degree::map,
            reduce_fn: vertex_degree::reduce,
            process_output_fn: vertex_degree::process_output,
        }),
        _ => None,
    }
}

/// Gets the [`App`] named `name`.
///
/// Returns an [`anyhow::Error`] if no application with the given name was found.
pub fn named(name: &str) -> Result<App> {
    match try_named(name) {
        Some(app) => Ok(app),
        None => bail!("No app named `{}` found.", name),
    }
}
