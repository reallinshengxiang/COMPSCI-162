/**
 * Important constants and utilities.
 *
 * Do not modify this file.
 */

#ifndef LIB_H__
#define LIB_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

/**
 * General constants and utilities.
 */

extern char* TMP_FILE_FORMAT_STR;    /* Intermediate file path format. */
extern char* OUTPUT_FILE_FORMAT_STR; /* Output file format. */
extern char* HOST;

/* Data structure for storing arbitrary bytes. */
typedef struct {
  ssize_t length;
  char* buffer;
} sized_buffer;

/**
 * Coordinator constants and utilities.
 */

#define TASK_TIMEOUT_SECS 10 /* Timeout before task is re-assigned. */

/**
 * Worker constants and utilities.
 */

#define WAIT_TIMEOUT_SECS 3 /* Idle worker wait time between GET_TASK RPCs. */

/* Key-value pair. */
typedef struct {
  sized_buffer key;
  sized_buffer value;
} key_value;

/* Variable-length array of key-value pairs. */
typedef struct {
  ssize_t length;
  key_value* kvs;
} key_value_array;

/* FNV hash implementation. */
unsigned int ihash(sized_buffer buf);

/* Comparison function for sorting key-value pairs by key. */
int by_key(const void* elem1, const void* elem2);

/* Read all contents of a file into a `sized_buffer`. */
sized_buffer read_all(char* filename);

/* Create a string from a `sized_buffer`, assuming that it contains no NULL characters. */
char* sized_buffer_to_str(sized_buffer buf);

/* Duplicate the contents of the given `sized_buffer` by allocating memory with `malloc`. */
sized_buffer sized_buffer_dup(sized_buffer buf);

/* Free the contents of the given `sized_buffer` (does not free the `sized_buffer` itself). */
void free_buffer(sized_buffer* buf);

/* Free the contents of the given `key_value_array` (does not free the `key_value_array` itself). */
void free_key_value_array(key_value_array* kva);

/* Create directories recursively, ignoring errors. */
void mkdirp(char* dir);

/**
 * Client constants and utilities.
 */

#define POLL_WAIT_DELAY_MS 100 /* Delay between polls when waiting on a job. */
#endif
