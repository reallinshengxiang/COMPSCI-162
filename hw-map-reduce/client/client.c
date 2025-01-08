/**
 * A client for interacting with a MapReduce cluster.
 *
 * Do not modify this file.
 */

#include "../rpc/rpc.h"
#include "../lib/lib.h"
#include "../codec/codec.h"
#include "../app/app.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <argp.h>
#include <time.h>
#include <limits.h>

static char mr_client_doc[] = "mr-client subcommands:\n\
  \tsubmit\tSubmit a job to a MapReduce cluster\n\
  \tprocess\tPost-process the output of a completed MapReduce job\n\
  \tpoll\tPoll a job running on a MapReduce cluster\n";

static char submit_doc[] = "Submit a job to a MapReduce cluster";

static char submit_args_doc[] = "[FILES]...";

static struct argp_option submit_options[] = {
    {"app", 'a', "APP", 0, "The name of the MapReduce application"},
    {"num-reduce", 'n', "NUM_REDUCE", 0, "The number of reduce tasks [default: 5]"},
    {"output-dir", 'o', "OUTPUT_DIR", 0, "The directory in which output files should be stored"},
    {"args", 'x', "ARGS", 0, "Arguments to the supplied MapReduce application"},
    {"wait", 'w', 0, 0, "Whether or not to wait until the job is finished"},
    {0}};

struct submit_arguments {
  char** files;
  int n_map;
  char* app;
  int n_reduce;
  char* output_dir;
  char* args;
  int wait;
};

static error_t submit_parse_opt(int key, char* arg, struct argp_state* state) {
  struct submit_arguments* arguments = state->input;

  switch (key) {
    case 'a':
      arguments->app = arg;
      break;
    case 'n':
      arguments->n_reduce = arg ? atoi(arg) : 5;
      break;
    case 'o':
      arguments->output_dir = arg;
      break;
    case 'x':
      arguments->args = arg;
      break;
    case 'w':
      arguments->wait = 1;
      break;

    case ARGP_KEY_NO_ARGS:
      argp_usage(state);
      break;

    case ARGP_KEY_ARG:

      arguments->n_map = state->argc - state->next + 1;
      arguments->files = malloc(sizeof(char*) * arguments->n_map);
      for (int i = 0; i < arguments->n_map; i++) {
        arguments->files[i] = malloc(sizeof(char) * (PATH_MAX+1));
        char* result = realpath(state->argv[state->next - 1 + i], arguments->files[i]);
        if (result == NULL) {
          printf("Error: file %s does not exist\n", state->argv[state->next - 1 + i]);
          exit(1);
        }
      }
      state->next = state->argc;

      break;

    case ARGP_KEY_END:
      if (arguments->app == NULL || arguments->output_dir == NULL) {
        argp_failure(state, 1, 0,
                     "--app and --output_dir are required arguments are required arguments. See "
                     "--help for more information");
        exit(1);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp submit_argp = {submit_options, submit_parse_opt, submit_args_doc, submit_doc};

void submit_job(char* host, struct submit_arguments* arguments);

static char poll_doc[] = "Poll a job running on a MapReduce cluster";

static char poll_args_doc[] = "[JOB_ID]";

struct poll_arguments {
  int job_id;
  int wait;
};

static error_t poll_parse_opt(int key, char* arg, struct argp_state* state) {
  struct poll_arguments* arguments = state->input;

  switch (key) {
    case 'w':
      arguments->wait = 1;
      break;

    case ARGP_KEY_ARG:

      arguments->job_id = arg ? atoi(arg) : -1;
      state->next = state->argc;

      break;

    case ARGP_KEY_END:
      if (arguments->job_id == -1) {
        argp_failure(state, 1, 0, "JOB_ID is required. See --help for more information");
        exit(1);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp poll_argp = {NULL, poll_parse_opt, poll_args_doc, poll_doc};

void poll_job(char* host, struct poll_arguments* arguments);

static char process_doc[] = "Post-process the output of a completed MapReduce job.";

static char process_args_doc[] = "";

static struct argp_option process_options[] = {
    {"app", 'a', "APP", 0, "The name of the MapReduce application"},
    {"num-reduce", 'n', "NUM_REDUCE", 0, "The number of reduce tasks [default: 5]"},
    {"output-dir", 'o', "OUTPUT_DIR", 0, "The directory in which output files should be stored"},
    {0}};

struct process_arguments {
  char* app;
  int n_reduce;
  char* output_dir;
};

static error_t process_parse_opt(int key, char* arg, struct argp_state* state) {
  struct process_arguments* arguments = state->input;

  switch (key) {
    case 'a':
      arguments->app = arg;
      break;
    case 'n':
      arguments->n_reduce = arg ? atoi(arg) : 5;
      break;
    case 'o':
      arguments->output_dir = arg;
      break;

    case ARGP_KEY_END:
      if (arguments->app == NULL || arguments->output_dir == NULL) {
        argp_failure(state, 1, 0,
                     "--app and --output_dir are required arguments are required arguments. See "
                     "--help for more information");
        exit(1);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp process_argp = {process_options, process_parse_opt, process_args_doc,
                                   process_doc};

void process(struct process_arguments* arguments);

void submit_job(char* host, struct submit_arguments* arguments) {
  CLIENT* clnt;
  int* reply;
  submit_job_request submit_job_1_arg;

  app current_app = get_app(arguments->app);

  if (current_app.name == NULL) {
    printf("Error: invalid app %s\n", arguments->app);
    return;
  }

#ifndef DEBUG
  clnt = clnt_create(host, COORDINATOR, COORDINATOR_V1, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    exit(1);
  }
#endif /* DEBUG */

  submit_job_1_arg.files.files_len = arguments->n_map;
  submit_job_1_arg.files.files_val = arguments->files;
  submit_job_1_arg.output_dir = arguments->output_dir;
  submit_job_1_arg.app = arguments->app;
  submit_job_1_arg.n_reduce = arguments->n_reduce;
  if (arguments->args == NULL || strlen(arguments->args) == 0) {
    submit_job_1_arg.args.args_len = 0;
  } else {
    submit_job_1_arg.args.args_len = strlen(arguments->args);
    submit_job_1_arg.args.args_val = arguments->args;
  }
  reply = submit_job_1(&submit_job_1_arg, clnt);
  if (reply == (int*)NULL) {
    clnt_perror(clnt, "call failed");
    exit(1);
  }
  int job_id = *reply;
  xdr_free((xdrproc_t)xdr_int, (char*)reply);
  if (job_id >= 0) {
    printf("Job ID: %d\n", job_id);

    if (arguments->wait > 0) {
      struct poll_arguments arguments = { job_id, 1 };
      poll_job(HOST, &arguments);
    }
  } else {
    printf("Error: failed to submit job\n");
  }

#ifndef DEBUG
  clnt_destroy(clnt);
#endif /* DEBUG */
}

void poll_job(char* host, struct poll_arguments* arguments) {
  CLIENT* clnt;
  poll_job_reply* reply;
  int poll_job_1_arg = arguments->job_id;

#ifndef DEBUG
  clnt = clnt_create(host, COORDINATOR, COORDINATOR_V1, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    exit(1);
  }
#endif /* DEBUG */

  if (arguments->wait > 0) {
    while (true) {
      int poll_job_1_arg = arguments->job_id;
      reply = poll_job_1(&poll_job_1_arg, clnt);
      if (reply == (poll_job_reply*)NULL) {
        clnt_perror(clnt, "call failed");
        exit(1);
      }
      if (reply->invalid_job_id) {
        printf("Error: invalid job ID %d\n", arguments->job_id);
        break;
      }
      bool done = reply->done;
      bool failed = reply->failed;
      xdr_free((xdrproc_t)xdr_poll_job_reply, (char*)reply);
      if (done) {
        if (failed)
          printf("Error: job %d failed\n", arguments->job_id);
        else
          printf("Job %d completed successfully\n", arguments->job_id);
        break;
      }

      struct timespec ts = { POLL_WAIT_DELAY_MS / 1000, (POLL_WAIT_DELAY_MS % 1000) * 1000000 };
      nanosleep(&ts, &ts);
    }
  } else {
    reply = poll_job_1(&poll_job_1_arg, clnt);
    if (reply == (poll_job_reply*)NULL) {
      clnt_perror(clnt, "call failed");
      exit(1);
    }
    if (reply->invalid_job_id)
      printf("Error: invalid job ID %d\n", arguments->job_id);
    else
      printf("Job Status:\n\tdone - %d\n\tfailed - %d\n", reply->done, reply->failed);
    xdr_free((xdrproc_t)xdr_poll_job_reply, (char*)reply);
  }
#ifndef DEBUG
  clnt_destroy(clnt);
#endif /* DEBUG */
}

void process(struct process_arguments* arguments) {
  app current_app = get_app(arguments->app);

  if (current_app.name == NULL) {
    printf("Error: invalid app %s\n", arguments->app);
    return;
  }

  length_delimited_reader* readers = malloc(sizeof(length_delimited_reader) * arguments->n_reduce);
  size_t entries = 0;
  for (int i = 0; i < arguments->n_reduce; i++) {
    char* filename = malloc(strlen(arguments->output_dir) + 100);
    sprintf(filename, OUTPUT_FILE_FORMAT_STR, arguments->output_dir, i);
    if (ldr_init(&readers[i], filename) != 0) {
      printf("Error: failed to read file: %s\n", filename);
      exit(1);
    }
    free(filename);
    entries += ldr_count(&readers[i]);
    ldr_rewind(&readers[i]);
  }

  if (entries % 2 == 1) {
    printf("Error: malformed intermediate file\n");
    exit(1);
  }

  key_value_array kva;
  kva.length = entries / 2;
  if (kva.length > 0) {
    kva.kvs = malloc(sizeof(key_value) * kva.length);
    size_t index = 0;
    for (int i = 0; i < arguments->n_reduce; i++) {
      sized_buffer key = ldr_next(&readers[i]);
      sized_buffer value = ldr_next(&readers[i]);
      while (index < kva.length && key.length >= 0 && value.length >= 0) {
        key_value kv = {key, value};
        kva.kvs[index++] = kv;
        key = ldr_next(&readers[i]);
        value = ldr_next(&readers[i]);
      }
      free_buffer(&key);
      free_buffer(&value);
      ldr_close(&readers[i]);
    }
    kva.length = index;

    current_app.process_output(stdout, kva);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Error: no subcommand specified.\n%s", mr_client_doc);
    exit(1);
  }
  if (strcmp(argv[1], "submit") == 0) {
    argv[1] = "mr-client submit";
    struct submit_arguments arguments;
    char* host;

    arguments.app = NULL;
    arguments.n_reduce = 5;
    arguments.output_dir = NULL;
    arguments.args = NULL;
    arguments.wait = -1;

    argp_parse(&submit_argp, argc - 1, argv + 1, 0, 0, &arguments);

    submit_job(HOST, &arguments);
  } else if (strcmp(argv[1], "poll") == 0) {
    argv[1] = "mr-client poll";
    struct poll_arguments arguments;
    char* host;

    arguments.job_id = -1;
    arguments.wait = -1;

    argp_parse(&poll_argp, argc - 1, argv + 1, 0, 0, &arguments);

    poll_job(HOST, &arguments);
  } else if (strcmp(argv[1], "process") == 0) {
    argv[1] = "mr-client process";
    struct process_arguments arguments;
    char* host;

    arguments.app = NULL;
    arguments.n_reduce = 5;
    arguments.output_dir = NULL;

    argp_parse(&process_argp, argc - 1, argv + 1, 0, 0, &arguments);

    process(&arguments);
  } else {
    printf("Error: invalid subcommand.\n%s", mr_client_doc);
    exit(1);
  }
}
