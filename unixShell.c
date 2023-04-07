#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <glob.h>
#include <stdbool.h>

#define MAX_COMMANDS 100
#define MAX_ARGUMENTS 1000
#define BUFFER_SIZE 1024

typedef struct command {
    char *cmd;
    char *args[MAX_ARGUMENTS];
    int nargs;
    char *input;
    char *output;
    bool background;
} command;

void change_directory(char *path);
void print_working_directory();
void parse_command(char *line, command *commands, int *num_commands);
void execute_commands(command *commands, int num_commands);
void execute_pipeline(command *commands, int start, int end);
void free_commands(command *commands, int num_commands);
void handle_exit(command *commands, int num_commands);

int main() {
    char prompt[BUFFER_SIZE] = "myshell> ";
    char line[BUFFER_SIZE];
    command commands[MAX_COMMANDS];
    int num_commands;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        parse_command(line, commands, &num_commands);
        if (num_commands == 0) {
            continue;
        }

        if (strcmp(commands[0].cmd, "prompt") == 0) {
            if (commands[0].nargs > 1) {
                printf("prompt: too many arguments\n");
            } else if (commands[0].nargs == 1) {
                strncpy(prompt, commands[0].args[0], BUFFER_SIZE - 1);
                prompt[BUFFER_SIZE - 1] = '\0';
            }
        } else if (strcmp(commands[0].cmd, "exit") == 0) {
            handle_exit(commands, num_commands);
        } else if (strcmp(commands[0].cmd, "pwd") == 0) {
            print_working_directory();
        } else if (strcmp(commands[0].cmd, "cd") == 0) {
            if (commands[0].nargs == 1) {
                change_directory(commands[0].args[0]);
            } else {
                printf("cd: too many arguments\n");
            }
        } else {
            execute_commands(commands, num_commands);
        }

        free_commands(commands, num_commands);
    }

    return 0;
}

void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("cd");
    }
}

void print_working_directory() {
    char cwd[BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

void parse_command(char *line, command *commands, int *num_commands) {
    int i = 0;
    char *token;
    char *saveptr_line, *saveptr_token;
    const char *delim_line = "&;\n";
    const char *delim_token = " \t";

    *num_commands = 0;
    token = strtok_r(line, delim_line, &saveptr_line);
    while (token != NULL) {
        command *current = &commands[(*num_commands)++];
        current->cmd = NULL;
        current->nargs = 0;
        current->input = NULL;
        current->output = NULL;
        current->background = false;

        char *arg = strtok_r(token, delim_token, &saveptr_token);
        while (arg != NULL) {
            if (strcmp(arg, "<") == 0) {
                arg = strtok_r(NULL, delim_token, &saveptr_token);
                if (arg == NULL) {
                    fprintf(stderr, "parse_command: no input file\n");
                    break;
                }
                current->input = strdup(arg);
            } else if (strcmp(arg, ">") == 0) {
                arg = strtok_r(NULL, delim_token, &saveptr_token);
                if (arg == NULL) {
                    fprintf(stderr, "parse_command: no output file\n");
                    break;
                }
                current->output = strdup(arg);
            } else if (strcmp(arg, "|") == 0) {
                break;
            } else if (strcmp(arg, "&") == 0) {
                current->background = true;
            } else {
                if (current->cmd == NULL) {
                    current->cmd = strdup(arg);
                }

                if (strchr(arg, '*') != NULL || strchr(arg, '?') != NULL) {
                    glob_t globbuf;
                    int ret = glob(arg, 0, NULL, &globbuf);
                    if (ret == 0) {
                        for (size_t j = 0; j < globbuf.gl_pathc; ++j) {
                            current->args[i++] = strdup(globbuf.gl_pathv[j]);
                        }
                        globfree(&globbuf);
                    } else if (ret == GLOB_NOMATCH) {
                        current->args[i++] = strdup(arg);
                    } else {
                        perror("glob");
                    }
                } else {
                    current->args[i++] = strdup(arg);
                }
            }

            arg = strtok_r(NULL, delim_token, &saveptr_token);
        }

        current->args[i] = NULL;
        current->nargs = i;
        i = 0;
        token = strtok_r(NULL, delim_line, &saveptr_line);
    }
}

void execute_commands(command *commands, int num_commands) {
    int start = 0, end;

    for (end = 0; end < num_commands; ++end) {
        if (commands[end].cmd == NULL) {
            execute_pipeline(commands, start, end);
            start = end + 1;
        }
    }

    if (start < num_commands) {
        execute_pipeline(commands, start, num_commands);
    }
}

void execute_pipeline(command *commands, int start, int end) {
    int fds[2], status, in_fd = 0;
    pid_t pid;

    for (int i = start; i < end; ++i) {
        if (pipe(fds) < 0) {
            perror("pipe");
            exit(1);
        }

        if ((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (i < end - 1) {
                dup2(fds[1], STDOUT_FILENO);
                close(fds[1]);
            }

            close(fds[0]);

            if (commands[i].input != NULL) {
                int fd = open(commands[i].input, O_RDONLY);
                if (fd < 0) {
                    perror(commands[i].input);
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (commands[i].output != NULL) {
                int fd = open(commands[i].output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror(commands[i].output);
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (execvp(commands[i].cmd, commands[i].args) < 0) {
                perror(commands[i].cmd);
                exit(1);
            }
        } else {
            if (in_fd != 0) {
                close(in_fd);
            }

            close(fds[1]);
            in_fd = fds[0];

            if (commands[i].background) {
                printf("Background job %d\n", pid);
            } else {
                waitpid(pid, &status, 0);
            }
        }
    }
}

void free_commands(command *commands, int num_commands) {
    for (int i = 0; i < num_commands; ++i) {
        free(commands[i].cmd);
        for (int j = 0; j < commands[i].nargs; ++j) {
            free(commands[i].args[j]);
        }
        free(commands[i].input);
        free(commands[i].output);
    }
}

void handle_exit(command *commands, int num_commands) {
    free_commands(commands, num_commands);
    exit(0);
}

