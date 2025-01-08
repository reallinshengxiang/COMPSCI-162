//! Utilities for serializing and deserializing streams of data.
//!
//! Use [`LengthDelimitedWriter`] to create a buffer of packets.
//! The buffer can then be read by a [`LengthDelimitedReader`].
//!
//! # Examples
//!
//! Write byte packets into a writer, then retrieve them using a reader:
//! ```
//! # use map_reduce::codec::{LengthDelimitedWriter, LengthDelimitedReader};
//! # use bytes::Bytes;
//! let mut writer = LengthDelimitedWriter::new();
//! let p1 = Bytes::from("hello");
//! let p2 = Bytes::from("world");
//! writer.send(p1);
//! writer.send(p2);
//! let buf = writer.finish().freeze();
//!
//! let mut reader = LengthDelimitedReader::new(buf);
//! assert_eq!(reader.next(), Some(Bytes::from("hello")));
//! assert_eq!(reader.next(), Some(Bytes::from("world")));
//! assert_eq!(reader.next(), None);
//! ```
//!
//! Since the reader implements [`Iterator`], you can use it in a `for` loop:
//! ```
//! # use map_reduce::codec::{LengthDelimitedWriter, LengthDelimitedReader};
//! # use bytes::Bytes;
//! # let mut writer = LengthDelimitedWriter::new();
//! # let p1 = Bytes::from("hello");
//! # writer.send(p1);
//! # let buf = writer.finish().freeze();
//! let mut reader = LengthDelimitedReader::new(buf);
//! for packet in reader {
//!   // Do something with the packet.
//! }
//! ```
//!
//! You can also use the many utility functions that operate on iterators:
//! ```
//! # use map_reduce::codec::{LengthDelimitedWriter, LengthDelimitedReader};
//! # use bytes::Bytes;
//! # let mut writer = LengthDelimitedWriter::new();
//! # let p1 = Bytes::from("hello");
//! # writer.send(p1);
//! # let buf = writer.finish().freeze();
//! let mut reader = LengthDelimitedReader::new(buf);
//! // Collect all the packets in the reader into a Vec.
//! let messages = reader.collect::<Vec<_>>();
//! ```
//!
//!
// Do not modify this file.

use bytes::{Buf, BufMut, Bytes, BytesMut};

/// Constructs a buffer containing length-delimited packets.
pub struct LengthDelimitedWriter {
    buf: BytesMut,
}

impl LengthDelimitedWriter {
    /// Creates a new, initially empty, [`LengthDelimitedWriter`].
    pub fn new() -> Self {
        Self {
            buf: BytesMut::new(),
        }
    }

    /// Write a packet of [`Bytes`].
    ///
    /// The packet will be prefixed with its length and then written
    /// to an internal buffer.
    ///
    /// # Panics
    ///
    /// Panics if the length of the packet is larger than [`u32::MAX`].
    ///
    /// # Example
    ///
    /// ```
    /// # use map_reduce::codec::LengthDelimitedWriter;
    /// # use bytes::Bytes;
    /// let mut writer = LengthDelimitedWriter::new();
    /// let packet = Bytes::from("hello world\n");
    /// writer.send(packet);
    /// ```
    pub fn send(&mut self, b: Bytes) {
        assert!(b.len() <= u32::MAX as usize);

        self.buf.put_u32(b.len() as u32);
        self.buf.put(b);
    }

    /// Consumes the writer and returns a buffer
    /// containing all packets that were sent to the writer
    /// via [`LengthDelimitedWriter::send`].
    ///
    /// # Example
    ///
    /// ```
    /// # use map_reduce::codec::LengthDelimitedWriter;
    /// # use bytes::{Bytes, BytesMut};
    /// let mut writer = LengthDelimitedWriter::new();
    /// let packet = Bytes::from("hello world\n");
    /// writer.send(packet);
    /// let buf: BytesMut = writer.finish();
    /// // If you want to convert BytesMut to Bytes, use freeze:
    /// let buf: Bytes = buf.freeze();
    /// ```
    pub fn finish(self) -> BytesMut {
        self.buf
    }
}

impl Default for LengthDelimitedWriter {
    fn default() -> Self {
        Self::new()
    }
}

/// Reads a buffer of length-delimited packets, yielding single packets one at a time.
pub struct LengthDelimitedReader {
    buf: Bytes,
}

impl LengthDelimitedReader {
    /// Creates a new reader that reads from the given buffer.
    pub fn new(buf: Bytes) -> Self {
        Self { buf }
    }
}

impl Iterator for LengthDelimitedReader {
    type Item = Bytes;
    fn next(&mut self) -> Option<Self::Item> {
        if !self.buf.has_remaining() {
            return None;
        }

        let len = self.buf.get_u32();
        let chunk = self.buf.split_to(len as usize);
        Some(chunk)
    }
}
