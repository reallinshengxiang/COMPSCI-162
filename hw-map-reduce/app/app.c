/**
 * Converts MapReduce application names to actual application code.
 *
 * Do not modify this file.
 */

#include "app.h"

#define NUM_APPS 3

app apps[NUM_APPS] = {{"wc", wc_map, wc_reduce, wc_process_output},
                      {"grep", grep_map, grep_reduce, grep_process_output},
                      {"vertex-degree", vertex_degree_map, vertex_degree_reduce, vertex_degree_process_output}};

app get_app(char* name) {
  for (int i = 0; i < NUM_APPS; i++) {
    if (strcmp(name, apps[i].name) == 0) {
      return apps[i];
    }
  }
  app ret = {NULL, NULL, NULL, NULL};
  return ret;
}
