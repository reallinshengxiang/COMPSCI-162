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

pid_t background_pids[1024];
int num_background_pids = 0;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);
int cmd_wait(struct tokens* tokens); 

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
    {cmd_wait, "wait", "wait for all background jobs to finish"}  // 新增命令描述
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
  for (int i = 0; i < num_background_pids; i++) {
    waitpid(background_pids[i], NULL, 0);
  }
  //printf("Done waiting\n");
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
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO); 
                close(fd);
            }
            if (output_file && i == num_commands - 1) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO); 
                close(fd);
            }
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipe_arr[j][0]);
                close(pipe_arr[j][1]);
            }
            char* program = args[0];
            if (strchr(program, '/') == NULL) {
                char* full_path = search_in_path(program);
                if (full_path != NULL) {
                    execv(full_path, args);
                    free(full_path);
                }
            } else {
                execv(program, args);
            }
            perror("execv");
            exit(EXIT_FAILURE);
        }

        if (background) {
            background_pids[num_background_pids++] = pid;
        }

        command_index++; 
    }
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipe_arr[i][0]);
        close(pipe_arr[i][1]);
    }
    if (!background) {
        for (int i = 0; i < num_commands; i++) {
            wait(NULL);
        }
    }
}

int main(unused int argc, unused char* argv[]) {
  init_shell();
  signal(SIGINT,  SIG_IGN);  
  signal(SIGTSTP, SIG_IGN);  
  int line_num = 0;
  while (1) {
    char* line = NULL;
    size_t size = 0;
    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", line_num++);
    if (getline(&line, &size, stdin) == -1) {
      if (feof(stdin)) {
        exit(0);
      } else {
        perror("getline");
        continue;
      }
    }

    line[strcspn(line, "\n")] = 0;

    struct tokens* tokens = tokenize(line);
    if (tokens == NULL || tokens_get_length(tokens) == 0) {
      free(line);
      continue;
    }

    char* first_token = tokens_get_token(tokens, 0);
    if (first_token == NULL) {
      tokens_destroy(tokens);
      free(line);
      continue;
    }

    int cmd_index = lookup(first_token);
    if (cmd_index >= 0) {
      cmd_table[cmd_index].fun(tokens);
    } else {
      bool background = false;
      int token_length = tokens_get_length(tokens);
      if (strcmp(tokens_get_token(tokens, token_length - 1), "&") == 0) {
        background = true;
        tokens->tokens[token_length - 1] = NULL; 
        tokens->tokens_length--; 
      }

      run_command_with_pipes(tokens, background);

    }

    tokens_destroy(tokens);
    free(line);
  }

  return 0;
}
