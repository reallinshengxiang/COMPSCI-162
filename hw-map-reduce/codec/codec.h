/**
 * Utilities for serializing and deserializng streams of data.
 *
 * Do not modify this file.
 */

#ifndef CODEC_H__
#define CODEC_H__
#include "../lib/lib.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  FILE* outfile;
} length_delimited_writer;

typedef struct {
  FILE* infile;
} length_delimited_reader;

int ldw_init(length_delimited_writer* ldw, char* filename);

void ldw_write(length_delimited_writer* ldw, sized_buffer buf);

void ldw_close(length_delimited_writer* ldw);

int ldr_init(length_delimited_reader* ldr, char* filename);

sized_buffer ldr_next(length_delimited_reader* ldr);

size_t ldr_count(length_delimited_reader* ldr);

void ldr_rewind(length_delimited_reader* ldr);

void ldr_close(length_delimited_reader* ldr);
#endif
