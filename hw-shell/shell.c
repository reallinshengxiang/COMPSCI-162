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
/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print current working directory"},
    {cmd_cd, "cd", "change directory"}
};
/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) { exit(0); }
/* Prints the current working directory */


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


/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
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
void tokens_remove(struct tokens* tokens, int start, int count) {
    memmove(&tokens->tokens[start], &tokens->tokens[start + count],
            (tokens->tokens_length - start - count) * sizeof(char*));
    tokens->tokens_length -= count;
}
void run_external_command(struct tokens* tokens,bool background) {
    int stdin_fd = dup(STDIN_FILENO);  
    int stdout_fd = dup(STDOUT_FILENO); 
    for (int i = 0; i < tokens_get_length(tokens) - 1; ++i) {
        char* token = tokens_get_token(tokens, i);
        if (strcmp(token, ">") == 0) {
            char* output_file = tokens_get_token(tokens, i + 1);
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO); 
            close(fd); 
            printf("%d",i);
            tokens_remove(tokens, i, 2);
            i--; 
        } else if (strcmp(token, "<") == 0) {
            char* input_file = tokens_get_token(tokens, i + 1);
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO); 
            close(fd); 
            printf("%d",i);
            tokens_remove(tokens, i, 2);
            i--; 
        }
    }

    pid_t pid = fork(); 
    if (pid == 0) {
        int token_count = tokens_get_length(tokens);
        char* args[token_count + 1];
        for (int i = 0; i < token_count; i++) {
            args[i] = tokens_get_token(tokens, i); 
        }
        args[token_count] = NULL;
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
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
    dup2(stdin_fd, STDIN_FILENO);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdin_fd);
    close(stdout_fd);
}
void run_command_with_pipes(struct tokens* tokens) {
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

        command_index++; 
    }
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipe_arr[i][0]);
        close(pipe_arr[i][1]);
    }
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main(unused int argc, unused char* argv[]) {
  init_shell();
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGSTOP, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  static char line[4096];
  int line_num = 0;
  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens* tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      run_command_with_pipes(tokens);  // 执行外部命令
      //fprintf(stdout, "This shell doesn't know how to run programs.\n");
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}