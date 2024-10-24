use std::env;
use std::net::{Ipv4Addr, SocketAddrV4};

use crate::args;

use crate::http::*;
use crate::stats::*;

use clap::Parser;
use tokio::net::{TcpListener, TcpStream};

use anyhow::Result;

//use tokio::fs::File;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
//use std::path::PathBuf;
use tokio::fs::{File, read_dir, metadata};
use std::path::{PathBuf, Path};
use std::ffi::OsStr;

pub fn main() -> Result<()> {
    // Configure logging
    // You can print logs (to stderr) using
    // `log::info!`, `log::warn!`, `log::error!`, etc.
    env_logger::Builder::new()
        .filter_level(log::LevelFilter::Info)
        .init();

    // Parse command line arguments
    let args = args::Args::parse();

    // Set the current working directory
    env::set_current_dir(&args.files)?;

    // Print some info for debugging
    log::info!("HTTP server initializing ---------");
    log::info!("Port:\t\t{}", args.port);
    log::info!("Num threads:\t{}", args.num_threads);
    log::info!("Directory:\t\t{}", &args.files);
    log::info!("----------------------------------");

    // Initialize a thread pool that starts running `listen`
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .worker_threads(args.num_threads)
        .build()?
        .block_on(listen(args.port))
}

async fn listen(port: u16) -> Result<()> {
    // Hint: you should call `handle_socket` in this function.
    //todo!("TODO: Part 2")
    let listener = TcpListener::bind(SocketAddrV4::new(Ipv4Addr::UNSPECIFIED, port)).await?;
    loop {
        let (socket, addr) = listener.accept().await?;
        tokio::spawn(async move {handle_socket(socket).await});
    }
}

// Handles a single connection via `socket`.
async fn handle_socket(mut socket: TcpStream) -> Result<()> {
    //todo!("TODO: Part 3")
    let request = parse_request(&mut socket).await?;
    match request.method.as_str() {
        "GET" => {
            let path = format!(".{}", &request.path);
            let path_buf = PathBuf::from(path);
            let meta = metadata(&path_buf).await;
            if let Ok(meta) = meta {
                if meta.is_file() {
                    handle_file_request(&mut socket, path_buf).await?;
                } else if meta.is_dir() {
                    handle_directory_request(&mut socket, path_buf).await?;
                }
            } else {
                start_response(&mut socket, 404).await?;
                end_headers(&mut socket).await?;
            }
        }
        _ => {
            start_response(&mut socket, 404).await?;
            end_headers(&mut socket).await?;
        }
    }

    Ok(())
}

// You are free (and encouraged) to add other funtions to this file.
// You can also create your own modules as you see fit.
async fn handle_file_request(socket: &mut TcpStream, path_buf: PathBuf) -> Result<()> {
    let mut file = File::open(&path_buf).await?;
    let mime_type = get_mime_type(path_buf.to_str().unwrap_or(""));
    let metadata = file.metadata().await?;
    let content_length = metadata.len();

    start_response(socket, 200).await?;
    write_header(socket, "Content-Type", mime_type).await?;
    write_header(socket, "Content-Length", &content_length.to_string()).await?;
    end_headers(socket).await?;

    let mut buffer = [0; 1024];
    loop {
        let n = file.read(&mut buffer).await?;
        if n == 0 {
            break;
        }
        socket.write_all(&buffer[..n]).await?;
    }
    Ok(())
}
async fn handle_directory_request(socket: &mut TcpStream, mut path_buf: PathBuf) -> Result<()> {
    path_buf.push("index.html");
    if path_buf.is_file() {
        handle_file_request(socket, path_buf).await?;
    } else {
        path_buf.pop(); 
        start_response(socket, 200).await?;
        write_header(socket, "Content-Type", "text/html").await?;
        end_headers(socket).await?;

        let mut response = String::new();
        response.push_str("<html><body><ul>\n");
        if let Some(parent) = path_buf.parent() {
            let parent_href = format_href(parent);
            response.push_str(&format!("<li><a href=\"{}\">Parent Directory</a></li>\n", parent_href));
        }

        let mut entries = read_dir(&path_buf).await?;
        while let Some(entry) = entries.next_entry().await? {
            let file_name = entry.file_name();
            let file_name_str = file_name.to_string_lossy();
            let href = format_href(&entry.path());
            response.push_str(&format!("<li><a href=\"{}\">{}</a></li>\n", href, file_name_str));
        }

        response.push_str("</ul></body></html>\n");
        socket.write_all(response.as_bytes()).await?;
    }
    Ok(())
}


fn format_href(path: &Path) -> String {
    let path_str = path.to_string_lossy().replace("./", "");
    format!("/{}", path_str)
}

async fn write_header(socket: &mut TcpStream, key: &str, value: &str) -> Result<()> {
    let header = format!("{}: {}\r\n", key, value);
    socket.write_all(header.as_bytes()).await?;
    Ok(())
}
