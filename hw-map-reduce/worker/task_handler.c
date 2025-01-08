/**
 * Helper functions for handling map and reduce tasks.
 *
 * Do not modify this file.
 */

#include "task_handler.h"

int handle_reduce_task(get_task_reply* reply) {
  app current_app = get_app(reply->app);

  length_delimited_reader readers[reply->n_map];

  ssize_t entries = open_reduce_readers(readers, reply);
  if (entries == -1)
    return 1;

  if (entries % 2 == 1) {
    printf("Error: malformed intermediate file\n");
    return 1;
  }

  length_delimited_writer output_writer;
  if (open_reduce_writer(&output_writer, reply) != 0)
    return 1;

  key_value_array kva;
  kva.length = entries / 2;
  if (kva.length > 0) {
    if (read_map_kvs(readers, &kva, reply) != 0) {
      return 1;
    }
    sized_buffer aux = {reply->args.args_len, reply->args.args_val};
    reduce_kvs(&output_writer, &kva, current_app.reduce, aux);
  }
  ldw_close(&output_writer);

  return 0;
}

ssize_t open_reduce_readers(length_delimited_reader* readers, get_task_reply* reply) {
  size_t entries = 0;
  for (int i = 0; i < reply->n_map; i++) {
    char filename[strlen(reply->output_dir) + 100];
    sprintf(filename, TMP_FILE_FORMAT_STR, reply->output_dir, i, reply->task);
    if (ldr_init(&readers[i], filename) != 0) {
      printf("Error: failed to read file: %s\n", filename);
      for (int j = 0; j < i; j++) {
        ldr_close(&readers[j]);
      }
      return -1;
    }
    entries += ldr_count(&readers[i]);
    ldr_rewind(&readers[i]);
  }
  return entries;
}

int open_reduce_writer(length_delimited_writer* writer, get_task_reply* reply) {
  char filename[strlen(reply->output_dir) + 100];
  sprintf(filename, OUTPUT_FILE_FORMAT_STR, reply->output_dir, reply->task);
  if (ldw_init(writer, filename) != 0) {
    printf("Error: Could not open output file %s for writing\n", filename);
    return 1;
  }
  return 0;
}

int read_map_kvs(length_delimited_reader* readers, key_value_array* kva, get_task_reply* reply) {
  kva->kvs = malloc(sizeof(key_value) * kva->length);
  if (kva->kvs == NULL) {
    printf("Error: failed to allocate memory for KV array\n");
    return 1;
  }
  size_t index = 0;
  for (int i = 0; i < reply->n_map; i++) {
    sized_buffer key = ldr_next(&readers[i]);
    sized_buffer value = ldr_next(&readers[i]);
    while (index < kva->length && key.length >= 0 && value.length >= 0) {
      key_value kv = {key, value};
      kva->kvs[index++] = kv;
      key = ldr_next(&readers[i]);
      value = ldr_next(&readers[i]);
    }
    free_buffer(&key);
    free_buffer(&value);
    ldr_close(&readers[i]);
  }
  kva->length = index;

  return 0;
}

void reduce_kvs(length_delimited_writer* writer, key_value_array* kva, reduce_fn reduce,
                sized_buffer aux) {
  qsort(kva->kvs, kva->length, sizeof(key_value), by_key);

  size_t p1 = 0;
  size_t p2 = 1;
  while (p1 < kva->length) {
    if (p2 >= kva->length || by_key((void*)(kva->kvs + p1), (void*)(kva->kvs + p2)) != 0) {
      if (p2 > p1) {
        key_value_array per_key_array = {p2 - p1, kva->kvs + p1};
        key_value kv = reduce(kva->kvs[p1].key, per_key_array, aux);
        ldw_write(writer, kv.key);
        ldw_write(writer, kv.value);
        free_buffer(&kv.key);
        free_buffer(&kv.value);
      }
      p1 = p2;
    }
    p2++;
  }

  free_key_value_array(kva);
}

int handle_map_task(get_task_reply* reply) {
  app current_app = get_app(reply->app);

  if (current_app.name == NULL) {
    printf("Error: received invalid app name %s\n", current_app.name);
    return 1;
  }

  length_delimited_writer writers[reply->n_reduce];
  if (open_map_writers(writers, reply) != 0)
    return 1;

  sized_buffer contents = read_all(reply->file);
  if (contents.length < 0) {
    printf("Error: failed to read file: %s\n", reply->file);
    for (int i = 0; i < reply->n_reduce; i++) {
      ldw_close(&writers[i]);
    }
    return 1;
  }

  sized_buffer aux = {reply->args.args_len, reply->args.args_val};
  key_value_array kva = map_file(reply->file, contents, current_app.map, aux);

  write_map_kvs(writers, &kva, reply);
  return 0;
}

int open_map_writers(length_delimited_writer* writers, get_task_reply* reply) {
  for (int i = 0; i < reply->n_reduce; i++) {
    char filename[strlen(reply->output_dir)+100];
    sprintf(filename, TMP_FILE_FORMAT_STR, reply->output_dir, reply->task, i);
    if (ldw_init(&writers[i], filename) != 0) {
      printf("Error: failed to write file %s\n", filename);
      return 1;
    }
  }
  return 0;
}

key_value_array map_file(char* filename, sized_buffer contents, map_fn map, sized_buffer aux) {
  sized_buffer key = {strlen(filename), filename};

  key_value kv = {key, contents};

  key_value_array res = map(kv, aux);

  free_buffer(&contents);

  return res;
}

void write_map_kvs(length_delimited_writer* writers, key_value_array* kva, get_task_reply* reply) {
  for (size_t i = 0; i < kva->length; i++) {
    key_value kv = kva->kvs[i];
    unsigned int reduce_task = ihash(kv.key) % reply->n_reduce;
    ldw_write(&writers[reduce_task], kv.key);
    ldw_write(&writers[reduce_task], kv.value);
    free_buffer(&kv.key);
    free_buffer(&kv.value);
  }

  free_key_value_array(kva);
  for (int i = 0; i < reply->n_reduce; i++) {
    ldw_close(&writers[i]);
  }
}

int finish_task(CLIENT* clnt, get_task_reply* reply, bool success) {
  finish_task_request finish_task_1_arg;
  void* ft_reply;

  finish_task_1_arg.job_id = reply->job_id;
  finish_task_1_arg.task = reply->task;
  finish_task_1_arg.reduce = reply->reduce;
  finish_task_1_arg.success = success;
  ft_reply = finish_task_1(&finish_task_1_arg, clnt);
  if (ft_reply == (void*)NULL) {
    clnt_perror(clnt, "call failed");
    return 1;
  }
  return 0;
}
