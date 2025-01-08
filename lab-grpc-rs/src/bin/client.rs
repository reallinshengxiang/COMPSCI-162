//! The gRPC server.
//!
// Do not modify this file.

use anyhow::Result;
use clap::{Parser, Subcommand};
use lab_grpc::{
    client::{echo, example, get, put},
    log::init_logger,
};

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
pub struct Args {
    #[clap(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    Example {
        #[clap(value_parser)]
        input: u32,
    },
    Echo {
        #[clap(value_parser)]
        msg: String,
    },
    Put {
        #[clap(value_parser)]
        key: String,

        #[clap(value_parser)]
        value: String,
    },
    Get {
        #[clap(value_parser)]
        key: String,
    },
}

#[tokio::main]
async fn main() -> Result<()> {
    init_logger();
    let args = Args::parse();

    match args.command {
        Commands::Example { input } => {
            let output = example(input).await?;
            println!("{}", output);
        }
        Commands::Echo { msg } => {
            let msg = echo(msg).await?;
            println!("{}", msg);
        }
        Commands::Put { key, value } => {
            put(key.as_bytes().into(), value.as_bytes().into()).await?;
        }
        Commands::Get { key } => {
            let value = get(key.as_bytes().into()).await?;
            println!("{}", String::from_utf8(value).unwrap());
        }
    }

    Ok(())
}
