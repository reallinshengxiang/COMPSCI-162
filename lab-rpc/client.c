#include "kv_store.h"
#include "kv_store_client.h"

static char client_doc[] = "client subcommands:\n\
  \texample\tMake an EXAMPLE RPC\n\
  \techo\tMake an ECHO RPC\n\
  \tput\tMake a PUT RPC\n\
  \tget\tMake a GET RPC\n";

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("No subcommand specified.\n%s", client_doc);
    exit(1);
  }
  if (strcmp(argv[1], "example") == 0) {
    if (argc < 3) {
      printf("Input not specified.\nUsage: client example [INPUT]\n");
    }
    printf("%d\n", example(atoi(argv[2])));
  } else if (strcmp(argv[1], "echo") == 0) {
    if (argc < 3) {
      printf("Message not specified.\nUsage: client echo [MSG]\n");
    }
    char* message = echo(argv[2]);
    printf("%s\n", message);
    free(message);
  } else if (strcmp(argv[1], "put") == 0) {
    if (argc < 4) {
      printf("Key and/or value not specified.\nUsage: client put [KEY] [VALUE]\n");
    }
    buf key = {strlen(argv[2]), argv[2]};
    buf value = {strlen(argv[3]), argv[3]};
    put(key, value);
  } else if (strcmp(argv[1], "get") == 0) {
    if (argc < 3) {
      printf("Key not specified.\nUsage: client get [KEY]\n");
    }
    buf key = {strlen(argv[2]), argv[2]};
    buf* value = get(key);
    printf("%.*s\n", value->buf_len, value->buf_val);
    xdr_free((xdrproc_t)xdr_buf, (char *)value);
  } else {
    printf("Error: Invalid subcommand.\n%s", client_doc);
    exit(1);
  }

  return 0;
}
