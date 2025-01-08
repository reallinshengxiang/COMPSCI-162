/**
 * A MapReduce-compatible implementation of word count.
 *
 * Do not modify this file.
 */

#include "wc.h"

int num_words(sized_buffer buf) {
  int num_words = 0;

  int counter = 0;
  for (int i = 0; i < buf.length; i++) {
    if (isalpha(buf.buffer[i])) {
      counter++;
    } else {
      if (counter > 1) {
        num_words++;
      }
      counter = 0;
    }
  }

  if (counter > 1) num_words++;

  return num_words;
}

key_value_array wc_map(key_value input, sized_buffer aux) {
  int n = num_words(input.value);
  key_value* kva = malloc(sizeof(key_value) * n);

  size_t p1 = 0;
  size_t p2 = 0;
  size_t counter = 0;
  while (p1 < input.value.length) {
    if (p2 >= input.value.length || !isalpha(input.value.buffer[p2])) {
      if (p2 > p1+1) {
        char* word = malloc(sizeof(char) * (p2 - p1));
        size_t initial = p1;
        for (; p1 < p2; p1++) {
          word[p1 - initial] = tolower(input.value.buffer[p1]);
        }
        int* value_buffer = malloc(sizeof(int));
        *value_buffer = 1;
        key_value kv = {{p2 - initial, word}, {sizeof(int), (char*)value_buffer}};
        kva[counter++] = kv;
      }
      p1 = p2 + 1;
    }
    p2++;
  }

  key_value_array ret = {n, kva};
  return ret;
}

key_value wc_reduce(sized_buffer key, key_value_array input, sized_buffer aux) {
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

void wc_process_output(FILE* out, key_value_array input) {
  qsort(input.kvs, input.length, sizeof(key_value), wc_compare_fn);
  for (int i = 0; i < input.length; i++) {
    key_value kv = input.kvs[i];
    fprintf(out, "%i\t%.*s\n", *((int*)kv.value.buffer), (int)kv.key.length, kv.key.buffer);
  }
}

int wc_compare_fn(const void* elem1, const void* elem2) {
  key_value* kv1 = (key_value*)elem1;
  key_value* kv2 = (key_value*)elem2;

  sized_buffer key1 = kv1->key;
  sized_buffer key2 = kv2->key;

  int value1 = *((int*)kv1->value.buffer);
  int value2 = *((int*)kv2->value.buffer);
  if (value1 == value2) {
    int ret =
        strncmp(key1.buffer, key2.buffer, key1.length < key2.length ? key1.length : key2.length);
    if (ret == 0) {
      return key1.length - key2.length;
    }
    return ret;
  }
  return value1 - value2;
}
