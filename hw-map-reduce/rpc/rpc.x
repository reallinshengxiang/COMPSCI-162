typedef string path<>;

struct submit_job_request {
  path files<>;    /* Input files. */
  path output_dir; /* Output directory. */
  string app<>;    /* MapReduce application. */
  int n_reduce;    /* Desired number of reduce tasks. */
  char args<>;     /* Auxiliary arguments. */
};
typedef struct submit_job_request submit_job_request;

struct poll_job_reply {
  bool done;           /* Job has completed. */
  bool failed;         /* Job has failed. */
  bool invalid_job_id; /* Provided job ID does not correspond to a submitted job. */
};
typedef struct poll_job_reply poll_job_reply;

struct get_task_reply {
  int job_id;      /* Job ID of assigned task. */
  int task;        /* ID of assigned task. */
  path file;       /* [Map tasks only] Input file to map. */
  path output_dir; /* Location to save final output files. */
  string app<>;    /* MapReduce application. */
  int n_reduce;    /* Number of reduce tasks. */
  int n_map;       /* Number of map tasks (i.e. number of input files). */
  bool reduce;     /* Assigned task is a reduce task. */
  bool wait;       /* Worker should wait until a task becomes available. */
  char args<>;     /* Auxiliary arguments. */
};
typedef struct get_task_reply get_task_reply;

struct finish_task_request {
  int job_id;   /* Job ID of finished task. */
  int task;     /* ID of finished task. */
  bool reduce;  /* Finished task is a reduce task. */
  bool success; /* Task was completed successfully. */
};
typedef struct finish_task_request finish_task_request;

program COORDINATOR {
  version COORDINATOR_V1 {
    int EXAMPLE(int) = 1;                       /* [Example] Example RPC definition. */
    int SUBMIT_JOB(submit_job_request) = 2;     /* [Client] Submit a job to a MapReduce cluster. */
    poll_job_reply POLL_JOB(int) = 3;           /* [Client] Poll a job running on a MapReduce cluster. */
    get_task_reply GET_TASK(void) = 4;          /* [Worker] Retrieve task to run from MapReduce coordinator. */
    void FINISH_TASK(finish_task_request) = 5;  /* [Worker] Notify MapReduce coordinator of completed task. */
  } = 1;
} = 0x20000001;
