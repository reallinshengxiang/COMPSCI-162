const REQUEST_BUF_SIZE: usize = 1024;

use std::{ffi::OsStr, path::Path};

use tokio::io::{AsyncReadExt, AsyncWriteExt};

use anyhow::Result;

pub struct Request {
    pub method: String,
    pub path: String,
}

pub type StatusCode = u32;

#[derive(thiserror::Error, Debug)]
pub enum HttpError {
    #[error("invalid http request")]
    InvalidRequest,
}

pub async fn parse_request<T>(s: &mut T) -> Result<Request>
where
    T: AsyncReadExt + Unpin,
{
    let mut buf = [0; REQUEST_BUF_SIZE];

    let mut start;
    let mut end = 0;

    while {
        start = end;
        let len = s.read(&mut buf[start..]).await?;
        end = start + len;
        len > 0
    } {
        if buf[start..end].contains(&b'\r') {
            break;
        }
    }

    let mut iter = std::str::from_utf8(&buf[..end])?.trim().split_whitespace();
    let method = iter.next().ok_or(HttpError::InvalidRequest)?;
    let path = iter.next().ok_or(HttpError::InvalidRequest)?;

    Ok(Request {
        method: method.to_string(),
        path: path.to_string(),
    })
}

pub const fn response_message(status_code: StatusCode) -> &'static str {
    match status_code {
        100 => "Continue",
        200 => "OK",
        301 => "Moved Permanently",
        302 => "Found",
        304 => "Not Modified",
        400 => "Bad Request",
        401 => "Unauthorized",
        403 => "Forbidden",
        404 => "Not Found",
        405 => "Method Not Allowed",
        _ => "Internal Server Error",
    }
}

pub async fn start_response<T>(a: &mut T, status_code: StatusCode) -> Result<()>
where
    T: AsyncWriteExt + Unpin,
{
    let msg = format!(
        "HTTP/1.0 {} {}\r\n",
        status_code,
        response_message(status_code)
    );
    a.write_all((&msg).as_bytes()).await?;
    Ok(())
}

pub async fn send_header<T>(a: &mut T, key: &str, value: &str) -> Result<()>
where
    T: AsyncWriteExt + Unpin,
{
    let msg = format!("{}: {}\r\n", key, value);
    a.write_all((&msg).as_bytes()).await?;
    Ok(())
}

pub async fn end_headers<T>(s: &mut T) -> Result<()>
where
    T: AsyncWriteExt + Unpin,
{
    todo!("TODO: Part 1")
}

pub fn get_mime_type(path: &str) -> &'static str {
    todo!("TODO: Part 1")
}

fn file_extension(path: &str) -> Option<&str> {
    Path::new(path).extension().and_then(OsStr::to_str)
}

pub fn format_href(path: &str, filename: &str) -> String {
    format!("<a href=\"/{}\">{}</a><br/>", path, filename)
}

pub fn format_index(path: &str) -> String {
    format!("{}/index.html", path)
}
