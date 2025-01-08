use crate::http::*;
use anyhow::Result;

#[test]
fn test_mime_type() {
    assert_eq!(get_mime_type("hello.html"), "text/html");
    assert_eq!(get_mime_type("hello.htm"), "text/html");
    assert_eq!(get_mime_type("hello.jpg"), "image/jpeg");
    assert_eq!(get_mime_type("hello.jpeg"), "image/jpeg");
    assert_eq!(get_mime_type("hello.png"), "image/png");
    assert_eq!(get_mime_type("hello.css"), "text/css");
    assert_eq!(get_mime_type("hello.js"), "application/javascript");
    assert_eq!(get_mime_type("hello.pdf"), "application/pdf");
    assert_eq!(get_mime_type("hello.txt"), "text/plain");
    assert_eq!(get_mime_type("a/b/c/hello.html"), "text/html");
    assert_eq!(get_mime_type("a/b/c/hello.htm"), "text/html");
    assert_eq!(get_mime_type("a/b/c/hello.jpg"), "image/jpeg");
    assert_eq!(get_mime_type("a/b/c/hello.jpeg"), "image/jpeg");
    assert_eq!(get_mime_type("a/b/c/hello.png"), "image/png");
    assert_eq!(get_mime_type("a/b/c/hello.css"), "text/css");
    assert_eq!(get_mime_type("a/b/c/hello.js"), "application/javascript");
    assert_eq!(get_mime_type("a/b/c/hello.pdf"), "application/pdf");
    assert_eq!(get_mime_type("a/b/c/hello.txt"), "text/plain");
    assert_eq!(get_mime_type("/a/b/c/hello.html"), "text/html");
    assert_eq!(get_mime_type("/a/b/c/hello.htm"), "text/html");
    assert_eq!(get_mime_type("/a/b/c/hello.jpg"), "image/jpeg");
    assert_eq!(get_mime_type("/a/b/c/hello.jpeg"), "image/jpeg");
    assert_eq!(get_mime_type("/a/b/c/hello.png"), "image/png");
    assert_eq!(get_mime_type("/a/b/c/hello.css"), "text/css");
    assert_eq!(get_mime_type("/a/b/c/hello.js"), "application/javascript");
    assert_eq!(get_mime_type("/a/b/c/hello.pdf"), "application/pdf");
    assert_eq!(get_mime_type("/a/b/c/hello.txt"), "text/plain");
}

#[tokio::test]
async fn test_start_response() -> Result<()> {
    let mut buf = Vec::new();
    start_response(&mut buf, 200).await?;
    let s = std::str::from_utf8(&buf)?;
    assert_eq!(s, "HTTP/1.0 200 OK\r\n");
    Ok(())
}

#[tokio::test]
async fn test_end_headers() -> Result<()> {
    let mut buf = Vec::new();
    end_headers(&mut buf).await?;
    let s = std::str::from_utf8(&buf)?;
    assert_eq!(s, "\r\n");
    Ok(())
}
