/**
 * Important constants and utilities.
 *
 * Do not modify this file.
 */

#include "lib.h"

char* TMP_FILE_FORMAT_STR = "%s/mr-%d-%d";        /* Intermediate file path format. */
char* OUTPUT_FILE_FORMAT_STR = "%s/mr-out-%d";    /* Output file format. */
char* HOST = "localhost";

unsigned int ihash(sized_buffer buf) {
  const unsigned int fnv_prime = 0x811C9DC5;
  unsigned int hash = 0;
  unsigned int i = 0;

  for (i = 0; i < buf.length; buf.buffer++, i++) {
    hash *= fnv_prime;
    hash ^= (*buf.buffer);
  }

  return hash & 0x7fffffff;
}

int by_key(const void* elem1, const void* elem2) {
  key_value* kv1 = (key_value*)elem1;
  key_value* kv2 = (key_value*)elem2;

  sized_buffer key1 = kv1->key;
  sized_buffer key2 = kv2->key;

  int ret =
      strncmp(key1.buffer, key2.buffer, key1.length < key2.length ? key1.length : key2.length);
  if (ret == 0) {
    return key1.length - key2.length;
  }
  return ret;
}

sized_buffer read_all(char* filename) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    sized_buffer ret = {-1, NULL};
    return ret;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char* buffer = malloc(sizeof(char) * fsize);
  if (buffer == NULL) {
    printf("Error: failed to allocated memory\n");
    sized_buffer ret = {-1, NULL};
    return ret;
  }
  size_t read = fread(buffer, fsize, 1, f);
  fclose(f);

  sized_buffer ret = {fsize, buffer};
  return ret;
}

char* sized_buffer_to_str(sized_buffer buf) {
  char* str = malloc(sizeof(char) * (buf.length + 1));
  if (str == NULL) {
    return NULL;
  }
  memcpy(str, buf.buffer, buf.length);
  str[buf.length] = '\0';
  return str;
}

sized_buffer sized_buffer_dup(sized_buffer buf) {
  sized_buffer out;
  out.buffer = malloc(sizeof(char) * buf.length);
  if (out.buffer == NULL) {
    sized_buffer ret = {-1, NULL};
    return ret;
  }
  out.length = buf.length;
  memcpy(out.buffer, buf.buffer, buf.length);
  return out;
}

void free_buffer(sized_buffer* buf) {
  if (buf->length > 0)
    free(buf->buffer);
}

void free_key_value_array(key_value_array* kva) {
  if (kva->length > 0)
    free(kva->kvs);
}

void mkdirp(char *dir) {
  char tmp[PATH_MAX+1];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp),"%s",dir);
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;
      mkdir(tmp, S_IRWXU);
      *p = '/';
    }
  }
  mkdir(tmp, S_IRWXU);
}
