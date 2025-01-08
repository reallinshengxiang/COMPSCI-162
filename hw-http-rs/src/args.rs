use clap::Parser;

#[derive(Parser, Debug, Clone, Eq, PartialEq)]
#[clap(author, version, about, long_about = None)]
pub struct Args {
    #[clap(short, long)]
    pub files: String,
    #[clap(short, long, default_value_t = 8000)]
    pub port: u16,
    #[clap(short, long, default_value_t = 8)]
    pub num_threads: usize,
}

impl Default for Args {
    fn default() -> Self {
        Self {
            files: "www".to_string(),
            port: 8000,
            num_threads: 8,
        }
    }
}
