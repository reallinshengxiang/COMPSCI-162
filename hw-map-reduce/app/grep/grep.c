/**
 * A MapReduce-compatible implementation of `grep`.
 *
 * Do not modify this file.
 */

#include "grep.h"
#include "libgen.h"

struct occurrence {
  int line_num;
  char* line;
};

GList* find_occurrences(char* contents, char* search) {
  GList* occurrences = NULL;

  char* rest = contents;
  char* end;

  int line_num = 1;
  while (end = strstr(rest, "\n")) {
    *end = '\0';
    if (strstr(rest, search) != NULL) {
      struct occurrence* occ = malloc(sizeof(struct occurrence));
      occ->line_num = line_num;
      occ->line = rest;
      occurrences = g_list_append(occurrences, occ);
    }
    rest = end + 1;
    line_num++;
  }

  return occurrences;
}

/* Returns a GList of lines of contents where search occurs */
key_value_array grep_map(key_value input, sized_buffer aux) {
  char* contents = sized_buffer_to_str(input.value);
  char* search = sized_buffer_to_str(aux);

  GList* occurrences = find_occurrences(contents, search);

  size_t n = g_list_length(occurrences);
  key_value* kva = malloc(sizeof(key_value) * n);

  int index = 0;
  GList* elem = occurrences;
  while (elem) {
    kva[index].key = sized_buffer_dup(input.key);
    struct occurrence* occ = elem->data;
    kva[index].value.buffer = malloc(sizeof(char) * (50 + strlen(occ->line)));
    kva[index].value.length = sprintf(kva[index].value.buffer, "\t%d: %s", occ->line_num, occ->line);
    GList* temp = elem;
    elem = elem->next;
    occurrences = g_list_delete_link(occurrences, temp);
    index++;
  }

  free(contents);
  free(search);
  key_value_array ret = {n, kva};

  return ret;
}

key_value grep_reduce(sized_buffer key, key_value_array input, sized_buffer aux) {
  qsort(input.kvs, input.length, sizeof(key_value), grep_line_num_compare_fn);

  size_t total_length = 0;
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];
    total_length += kv.value.length + 1;
  }

  char* value_buffer = malloc(sizeof(char) * (total_length + 1));
  int index = 0;
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];
    index += sprintf(value_buffer + index, "%.*s\n", (int)kv.value.length, kv.value.buffer);
  }

  key_value ret = {key, {total_length - 1, value_buffer}};
  return ret;
}

void grep_process_output(FILE* out, key_value_array input) {
  qsort(input.kvs, input.length, sizeof(key_value), grep_file_name_compare_fn);
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];
    char buffer[kv.key.length+1];
    memcpy(buffer, kv.key.buffer, kv.key.length);
    buffer[kv.key.length] = '\0';
    fprintf(out, "%s:\n%.*s\n", basename(buffer), (int)kv.value.length,
            kv.value.buffer);
  }
}

int grep_line_num_compare_fn(const void* elem1, const void* elem2) {
  key_value* kv1 = (key_value*)elem1;
  key_value* kv2 = (key_value*)elem2;

  char* str1 = sized_buffer_to_str(kv1->value);
  char* str2 = sized_buffer_to_str(kv2->value);

  int line_num1 = atoi(str1);
  int line_num2 = atoi(str2);

  free(str1);
  free(str2);
  return line_num1 - line_num2;
}

int grep_file_name_compare_fn(const void* elem1, const void* elem2) {
  key_value* kv1 = (key_value*)elem1;
  key_value* kv2 = (key_value*)elem2;

  char* str1 = sized_buffer_to_str(kv1->key);
  char* str2 = sized_buffer_to_str(kv2->key);

  int res = strcmp(str1, str2);

  free(str1);
  free(str2);

  return res;
}
