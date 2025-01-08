/**
 * A MapReduce-compatible implementation of `grep`.
 *
 * Do not modify this file.
 */

#include "vertex_degree.h"

struct pair {
  int a;
  int b;
};

struct pair* get_pair(char* contents, char** end) {
  char* next;
  long a = strtol(contents, &next, 10);
  if (next == contents) return NULL;
  contents = next;
  long b = strtol(contents, &next, 10);
  if (next == contents) return NULL;
  *end = next;

  struct pair* p = malloc(sizeof(struct pair));
  p->a = a;
  p->b = b;

  return p;
}

GList* get_pairs(char* contents) {
  GList* pairs = NULL;

  struct pair* p;
  while (p = get_pair(contents, &contents)) {
    pairs = g_list_append(pairs, p);
  }

  return pairs;
}

key_value_array vertex_degree_map(key_value input, sized_buffer aux) {
  char* contents = sized_buffer_to_str(input.value);

  GList* pairs = get_pairs(contents);

  size_t n = g_list_length(pairs);
  key_value* kva = malloc(sizeof(key_value) * n * 2);

  int index = 0;
  GList* elem = pairs;
  while (elem) {
    struct pair* p = elem->data;

    kva[index].key.buffer = malloc(sizeof(int));
    *((int*) kva[index].key.buffer) = p->a;
    kva[index].key.length = sizeof(int);
    kva[index].value.buffer = malloc(sizeof(int));
    *((int*) kva[index].value.buffer) = 1;
    kva[index].value.length = sizeof(int);
    index++;

    kva[index].key.buffer = malloc(sizeof(int));
    *((int*) kva[index].key.buffer) = p->b;
    kva[index].key.length = sizeof(int);
    kva[index].value.buffer = malloc(sizeof(int));
    *((int*) kva[index].value.buffer) = 1;
    kva[index].value.length = sizeof(int);
    index++;

    GList* temp = elem;
    elem = elem->next;
    pairs = g_list_delete_link(pairs, temp);
  }

  free(contents);
  key_value_array ret = {2*n, kva};

  return ret;
}

key_value vertex_degree_reduce(sized_buffer key, key_value_array input, sized_buffer aux) {
  int count = 0;
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];
    count += *((int*)kv.value.buffer);
  }

  int* value_buffer = malloc(sizeof(int));
  *value_buffer = count;
  key_value ret = {key, {sizeof(int), (char*)value_buffer}};
  return ret;
}

void vertex_degree_process_output(FILE* out, key_value_array input) {
  qsort(input.kvs, input.length, sizeof(key_value), vertex_degree_compare_fn);
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];

    int key = *((int*)kv.key.buffer);
    int value = *((int*)kv.value.buffer);

    fprintf(out, "%d\t%d\n", key, value);
  }
}

int vertex_degree_compare_fn(const void* elem1, const void* elem2) {
  key_value* kv1 = (key_value*)elem1;
  key_value* kv2 = (key_value*)elem2;

  int key1 = *((int*)kv1->key.buffer);
  int key2 = *((int*)kv2->key.buffer);

  return key1 - key2;
}
