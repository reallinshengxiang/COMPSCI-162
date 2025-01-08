/**
 * A MapReduce-compatible implementation of `grep`.
 *
 * Do not modify this file.
 */

#ifndef GREP_H__
#define GREP_H__
#include "../../lib/lib.h"
#include <libgen.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <glib.h>

key_value_array grep_map(key_value input, sized_buffer aux);
key_value grep_reduce(sized_buffer key, key_value_array input, sized_buffer aux);
void grep_process_output(FILE* out, key_value_array input);

int grep_line_num_compare_fn(const void* elem1, const void* elem2);
int grep_file_name_compare_fn(const void* elem1, const void* elem2);
#endif
