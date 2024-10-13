#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

struct tokens {
  size_t tokens_length;
  char** tokens;
  size_t buffers_length;
  char** buffers;
};

#define unused __attribute__((unused))

bool shell_is_interactive;
int shell_terminal;
struct termios shell_tmodes;
pid_t shell_pgid;

struct process {
  pid_t pid;
  struct termios tmodes;
  int is_stopped;  
  int is_background; 
};

struct process processes[1024];  
struct tokens *bg_tokens;
int num_bg_tokens=0,start_bg_token=0;
int num_processes = 0; 

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);
int cmd_wait(struct tokens* tokens); 
int cmd_fg(struct tokens* tokens);
int cmd_bg(struct tokens* tokens);

typedef int cmd_fun_t(struct tokens* tokens);

typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print current working directory"},
    {cmd_cd, "cd", "change directory"},
    {cmd_wait, "wait", "wait for all background jobs to finish"},
    {cmd_fg, "fg", "bring job to the foreground"},
    {cmd_bg, "bg", "resume a job in the background"}
};

int cmd_help(unused struct tokens* tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

int cmd_exit(unused struct tokens* tokens) { exit(0); }

int cmd_pwd(unused struct tokens* tokens) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror("getcwd");
  }
  return 1;
}

int cmd_cd(struct tokens* tokens) {
  char* path = tokens_get_token(tokens, 1);
  
  if (path == NULL) {
    path = getenv("HOME");
    if (path == NULL) {
      fprintf(stderr, "cd: HOME not set\n");
      return -1;
    }
  }
  if (chdir(path) != 0) {
    perror("cd");
  }
  return 1;
}

int cmd_wait(unused struct tokens* tokens) {
  for (int i = 0; i < num_processes; i++) {
    if (processes[i].is_background) {
      waitpid(processes[i].pid, NULL, 0);
    }
  }
  return 1;
}

int cmd_fg(struct tokens* tokens) {
  pid_t pid = -1;
  if (tokens_get_length(tokens) > 1) {
    pid = atoi(tokens_get_token(tokens, 1));
  } else {
    for (int i = num_processes - 1; i >= 0; i--) {
      if (processes[i].is_background && processes[i].is_stopped == 0) {
        pid = processes[i].pid;
        break;
      }
    }
  }

  if (pid == -1) {
    fprintf(stderr, "fg: no such job\n");
    return 1;
  }

  tcsetpgrp(shell_terminal, pid);
  kill(pid, SIGCONT);
  int status;
  waitpid(pid, &status, WUNTRACED);
  tcsetpgrp(shell_terminal, shell_pgid);

  return 1;
}

int cmd_bg(struct tokens* tokens) {
  pid_t pid = -1;
  if (tokens_get_length(tokens) > 1) {
    pid = atoi(tokens_get_token(tokens, 1));
  } else {
    for (int i = num_processes - 1; i >= 0; i--) {
      if (processes[i].is_background && processes[i].is_stopped == 1) {
        pid = processes[i].pid;
        break;
      }
    }
  }

  if (pid == -1) {
    fprintf(stderr, "bg: no such job\n");
    return 1;
  }

  kill(pid, SIGCONT);

  return 1;
}

int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

void init_shell() {
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);
    shell_pgid = getpid();
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

char* search_in_path(const char* program_name) {
  char* path = getenv("PATH");
  if (!path) return NULL;
  char* path_dup = strdup(path);
  char* save_ptr;
  char* directory = strtok_r(path_dup, ":", &save_ptr);
  while (directory != NULL) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, program_name);
    if (access(full_path, X_OK) == 0) {
      free(path_dup); 
      return strdup(full_path); 
    }
    directory = strtok_r(NULL, ":", &save_ptr);
  }
  free(path_dup);
  return NULL;
}

void run_command_with_pipes(struct tokens* tokens, bool background) {
  int num_commands = 0;
  int token_length = tokens_get_length(tokens);
  for (int i = 0; i < token_length; i++) {
    if (strcmp(tokens_get_token(tokens, i), "|") == 0) {
      num_commands++;
    }
  }
  num_commands++;
  //printf("%d  %d\n",token_length,num_commands);
  int pipe_arr[num_commands - 1][2];  
  for (int i = 0; i < num_commands - 1; i++) {
    if (pipe(pipe_arr[i]) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }
  int command_index = 0;
  for (int i = 0; i < num_commands; i++) {
    int start = command_index;
    int input_redirect = -2;
    int output_redirect = -2;
    char* input_file = NULL;
    char* output_file = NULL;
    while (command_index < token_length && strcmp(tokens_get_token(tokens, command_index), "|") != 0) {
      if (strcmp(tokens_get_token(tokens, command_index), "<") == 0) {
        input_redirect = command_index;
        input_file = tokens_get_token(tokens, command_index + 1);
      } else if (strcmp(tokens_get_token(tokens, command_index), ">") == 0) {
        output_redirect = command_index;
        output_file = tokens_get_token(tokens, command_index + 1);
      }
      command_index++;
    }
    int arg_count = 0;
    char* args[command_index - start + 1];
    for (int j = start; j < command_index; j++) {
      if (j == input_redirect || j == input_redirect + 1 || j == output_redirect || j == output_redirect + 1) {
        continue;
      }
      args[arg_count++] = tokens_get_token(tokens, j);
    }
    args[arg_count] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTERM, SIG_DFL);
      signal(SIGSTOP, SIG_DFL);
      signal(SIGCONT, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      if (i > 0) {
        dup2(pipe_arr[i - 1][0], STDIN_FILENO);
      }
      if (i < num_commands - 1) {
        dup2(pipe_arr[i][1], STDOUT_FILENO);
      }
      if (input_file && i == 0) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
          perror("open input");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      }
      if (output_file && i == num_commands - 1) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
          perror("open output");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
      for (int j = 0; j < num_commands - 1; j++) {
        close(pipe_arr[j][0]);
        close(pipe_arr[j][1]);
      }
      char* path = search_in_path(args[0]);
      if (path) {
        execv(path, args);
      } else {
        execv(args[0], args);
      }
      perror("execv");
      exit(EXIT_FAILURE);
    } else if (pid < 0) {
      perror("fork");
    } else {
      if (i > 0) {
        close(pipe_arr[i - 1][0]);
      }
      if (i < num_commands - 1) {
        close(pipe_arr[i][1]);
      }
      if (!background) {
        int status;
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
          processes[num_processes].pid = pid;
          processes[num_processes].is_background = 0;
          processes[num_processes].is_stopped = 1;
          tcgetattr(shell_terminal, &processes[num_processes].tmodes);
          num_processes++;
        }
      } else {
        processes[num_processes].pid = pid;
        processes[num_processes].is_background = 1;
        processes[num_processes].is_stopped = 0;
        tcgetattr(shell_terminal, &processes[num_processes].tmodes);
        num_processes++;
      }
    }
    command_index++;
  }
}

int main(unused int argc, unused char* argv[]) {
  init_shell();
  signal(SIGINT,  SIG_IGN);  
  signal(SIGTSTP, SIG_IGN); 
  int line_num=0;
  static char line[4096];
  while (1) {
    if (shell_is_interactive) {
      fprintf(stdout, "%d: ", line_num++);
      fflush(stdout);
    }

    if (!fgets(line, 4096, stdin)) {
      break;
    }
    
    struct tokens* tokens = tokenize(line);
    if (tokens_get_length(tokens) == 0) {
      tokens_destroy(tokens);
      continue;
    }

    char* cmd = tokens_get_token(tokens, 0);
    int fundex = lookup(cmd);
    if (fundex >= 0) {
      if(fundex!=5) cmd_table[fundex].fun(tokens);
      else
      {
        run_command_with_pipes(bg_tokens, 0);
      }
    } else {
      int background = 0;
      if (tokens_get_length(tokens) > 1 && strcmp(tokens_get_token(tokens, tokens_get_length(tokens) - 1), "&") == 0) {
        background = 1;
        tokens->tokens_length--;
        if(strcmp(cmd,"/usr/bin/tee")==0)
        {
          char *ampersand = strchr(line, '&');
          if (ampersand != NULL) *ampersand = '\0';
          struct tokens* tokens = tokenize(line);
          bg_tokens=tokens;
        }
        else run_command_with_pipes(tokens, background);
      }
      else run_command_with_pipes(tokens, background);
    }
    tokens_destroy(tokens);
  }
  return 0;
}
