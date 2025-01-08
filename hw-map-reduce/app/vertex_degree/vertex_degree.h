/**
 * A MapReduce-compatible implementation of `grep`.
 *
 * Do not modify this file.
 */

#ifndef VERTEX_DEGREE_H__
#define VERTEX_DEGREE_H__
#include "../../lib/lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#endif

key_value_array vertex_degree_map(key_value input, sized_buffer aux);
key_value vertex_degree_reduce(sized_buffer key, key_value_array input, sized_buffer aux);
void vertex_degree_process_output(FILE* out, key_value_array input);

int vertex_degree_compare_fn(const void* elem1, const void* elem2);
