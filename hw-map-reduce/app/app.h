/**
 * Converts MapReduce application names to actual application code.
 *
 * Do not modify this file.
 */

#ifndef APP_H__
#define APP_H__
#include "../lib/lib.h"
#include "wc/wc.h"
#include "grep/grep.h"
#include "vertex_degree/vertex_degree.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>

typedef key_value_array (*map_fn)(key_value, sized_buffer);
typedef key_value (*reduce_fn)(sized_buffer, key_value_array, sized_buffer);
typedef void (*process_output_fn)(FILE*, key_value_array);

typedef struct {
  char* name;
  map_fn map;
  reduce_fn reduce;
  process_output_fn process_output;
} app;

app get_app(char* name);
#endif
