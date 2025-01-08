/**
 * The MapReduce worker.
 *
 * Do not modify this file.
 */

#include "task_handler.h"

void run_worker(char* host) {
  CLIENT* clnt;
  get_task_reply* reply;
  char* get_task_1_arg;

#ifndef DEBUG
  clnt = clnt_create(host, COORDINATOR, COORDINATOR_V1, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    exit(1);
  }
#endif /* DEBUG */

  while (true) {
    reply = get_task_1((void*)&get_task_1_arg, clnt);
    if (reply == (get_task_reply*)NULL) {
      clnt_perror(clnt, "call failed");
      exit(1);
    }

    bool success = true;
    if (reply->wait) {
      printf("Received wait, waiting %d seconds\n", WAIT_TIMEOUT_SECS);
      sleep(WAIT_TIMEOUT_SECS);
    } else {
      if (reply->reduce) {
        printf("Received reduce task %d for job %d\n", reply->task, reply->job_id);
        if (handle_reduce_task(reply) != 0)
          success = false;
      } else {
        printf("Received map task %d for job %d\n", reply->task, reply->job_id);
        if (handle_map_task(reply) != 0)
          success = false;
      }
      if (finish_task(clnt, reply, success))
        exit(1);
    }
    xdr_free((xdrproc_t)xdr_get_task_reply, (char*)reply);
  }
#ifndef DEBUG
  clnt_destroy(clnt);
#endif /* DEBUG */
}

int main(int argc, char* argv[]) {
  run_worker(HOST);
  exit(0);
}
