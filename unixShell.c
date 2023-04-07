#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <glob.h>

#define MAX_CMD_SIZE 100
#define MAX_ARGS_SIZE 1000

void set_signal_handlers();
void execute_command_line(char *cmd);
void execute_job(char *job, int bg);
void execute_command(char *command);
void expand_wildcards(char *token, glob_t *globbuf);
void builtin_commands(char *cmd);

int main() {
    char cmd[MAX_CMD_SIZE];
    set_signal_handlers();
    while (1) {
        printf("shell> ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }
        execute_command_line(cmd);
    }
    return 0;
}

void set_signal_handlers() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

void execute_command_line(char *cmd) {
    char *job = strtok(cmd, "&;");
    char *next_token = strtok(NULL, "&;");
    while (job != NULL) {
        execute_job(job, (next_token != NULL && *next_token == '&'));
        job = next_token;
        if (next_token != NULL) {
            next_token = strtok(NULL, "&;");
        }
    }
}

void execute_job(char *job, int bg) {
    char *command = strtok(job, "|");
    int pipefd[2];
    int prev_pipefd = -1;
    while (command != NULL) {
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(1);
        }
        if (fork() == 0) {
            if (prev_pipefd != -1) {
                dup2(prev_pipefd, STDIN_FILENO);
                close(prev_pipefd);
            }
            if (strtok(NULL, "|") != NULL) {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);
            close(pipefd[1]);
            execute_command(command);
        }
        if (prev_pipefd != -1) {
            close(prev_pipefd);
        }
        prev_pipefd = pipefd[0];
        close(pipefd[1]);
        command = strtok(NULL, "|");
    }
    if (!bg) {
        int status;
        wait(&status);
    }
}

void execute_command(char *command) {
    char *args[MAX_ARGS_SIZE];
    int arg_idx = 0;
    char *token = strtok(command, " \t\n");
    int input_fd = -1;
    int output_fd = -1;
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            input_fd = open(token, O_RDONLY);
            if (input_fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            output_fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        } else {
            glob_t globbuf;
            globbuf.gl_offs = 0;
            expand_wildcards(token, &globbuf);
            for (size_t i = 0; i < globbuf.gl_pathc; i++) {
                args[arg_idx++] = globbuf.gl_pathv[i];
            }
            globfree(&globbuf);
        }
        token = strtok(NULL, " \t\n");
    }
    args[arg_idx] = NULL;

    if (arg_idx > 0) {
        if (strcmp(args[0], "prompt") == 0 || strcmp(args[0], "pwd") == 0 ||
            strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0) {
            builtin_commands(args[0]);
            exit(0);
        } else {
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(1);
            }
        }
    }
}

void expand_wildcards(char *token, glob_t *globbuf) {
    int flags = GLOB_NOCHECK | GLOB_APPEND;
    if (globbuf->gl_pathc == 0) {
        flags = GLOB_NOCHECK;
    }
    glob(token, flags, NULL, globbuf);
}

void builtin_commands(char *cmd) {
    if (strcmp(cmd, "prompt") == 0) {
        printf("Enter new prompt: ");
        char new_prompt[100];
        if (fgets(new_prompt, sizeof(new_prompt), stdin) != NULL) {
            strtok(new_prompt, "\n");
            strcpy(cmd, new_prompt);
        }
    } else if (strcmp(cmd, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        exit(0);
    } else if (strcmp(cmd, "cd") == 0) {
        char *path = strtok(NULL, " \t\n");
        if (chdir(path) == -1) {
            perror("chdir");
        }
        exit(0);
    } else if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
}

