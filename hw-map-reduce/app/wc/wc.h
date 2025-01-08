/**
 * A MapReduce-compatible implementation of word count.
 *
 * Do not modify this file.
 */

#ifndef WC_H__
#define WC_H__
#include "../../lib/lib.h"
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

key_value_array wc_map(key_value input, sized_buffer aux);
key_value wc_reduce(sized_buffer key, key_value_array input, sized_buffer aux);
void wc_process_output(FILE* out, key_value_array input);

int wc_compare_fn(const void* elem1, const void* elem2);
#endif
