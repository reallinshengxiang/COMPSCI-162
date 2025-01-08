/**
 * The MapReduce worker.
 *
 * Do not modify this file.
 */

#ifndef WORKER_H__
#define WORKER_H__
#include "../rpc/rpc.h"
#include "../lib/lib.h"
#include "../codec/codec.h"
#include "../app/app.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

int handle_reduce_task(get_task_reply* reply);
ssize_t open_reduce_readers(length_delimited_reader* readers, get_task_reply* reply);
int open_reduce_writer(length_delimited_writer* writer, get_task_reply* reply);
void read_map_kvs(length_delimited_reader* readers, key_value_array* kva, get_task_reply* reply);
void reduce_kvs(length_delimited_writer* writer, key_value_array* kva, reduce_fn reduce,
                sized_buffer aux);

int handle_map_task(get_task_reply* reply);
int open_map_writers(length_delimited_writer* writers, get_task_reply* reply);
key_value_array map_file(char* filename, sized_buffer contents, map_fn map, sized_buffer aux);
void write_map_kvs(length_delimited_writer* writers, key_value_array* kva, get_task_reply* reply);

void finish_task(CLIENT* clnt, get_task_reply* reply, bool success);
#endif
