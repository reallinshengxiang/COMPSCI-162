/**
 * Utilities for serializing and deserializng streams of data.
 *
 * Do not modify this file.
 */

#include "codec.h"

int ldw_init(length_delimited_writer* ldw, char* filename) {
  ldw->outfile = fopen(filename, "wb");
  if (ldw->outfile == NULL)
    return 1;
  return 0;
}

void ldw_write(length_delimited_writer* ldw, sized_buffer buf) {
  fwrite(&buf.length, sizeof(buf.length), 1, ldw->outfile);
  fwrite(buf.buffer, sizeof(char), buf.length, ldw->outfile);
}

void ldw_close(length_delimited_writer* ldw) { fclose(ldw->outfile); }

int ldr_init(length_delimited_reader* ldr, char* filename) {
  ldr->infile = fopen(filename, "rb");
  if (ldr->infile == NULL)
    return 1;
  return 0;
}

sized_buffer ldr_next(length_delimited_reader* ldr) {
  size_t bytes_read;

  size_t length;
  bytes_read = fread(&length, sizeof(length), 1, ldr->infile);
  if (bytes_read != 1) {
    sized_buffer ret = {-1, NULL};
    return ret;
  }

  if (length == 0) {
    sized_buffer ret = {0, NULL};
    return ret;
  }

  char* buffer = malloc(sizeof(char) * length);
  if (buffer == NULL) {
    sized_buffer ret = {-1, NULL};
    return ret;
  }

  bytes_read = fread(buffer, sizeof(char), length, ldr->infile);
  if (bytes_read != length) {
    sized_buffer ret = {-1, NULL};
    return ret;
  }

  sized_buffer ret = {length, buffer};

  return ret;
}

size_t ldr_count(length_delimited_reader* ldr) {
  sized_buffer next = ldr_next(ldr);
  size_t count = 0;
  while (next.length >= 0) {
    free_buffer(&next);
    next = ldr_next(ldr);
    count++;
  }
  free_buffer(&next);

  return count;
}

void ldr_rewind(length_delimited_reader* ldr) { rewind(ldr->infile); }

void ldr_close(length_delimited_reader* ldr) { fclose(ldr->infile); }
