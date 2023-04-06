/*
commands.c implements the function used to execute a single command. The function checks if the command 
is a built-in command (cd, pwd, exit, and prompt) 
and executes it accordingly. If the command is not a built-in command, 
the function forks a new process and calls execvp to execute the command. 
The function also handles input/output redirection by opening the specified files and 
redirecting standard input/output to the opened files. 
The function returns the process ID of the newly created process if the command is not a built-in command, 
and returns 0 or -1 depending on the result of executing the built-in command.
*/

#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "commands.h"
#include "job.h"
#include "parser.h"
#include <fcntl.h>

#define PROMPT_DEFAULT "$ "

int change_directory(const char *path) {
    if (chdir(path) != 0) {
        printf("cd: %s: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
}

int print_working_directory() {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        printf("pwd: %s\n", strerror(errno));
        return -1;
    }
    printf("%s\n", cwd);
    free(cwd);
    return 0;
}

bool is_builtin_command(const struct command *cmd) {
    return strcmp(cmd->argv[0], "cd") == 0 ||
           strcmp(cmd->argv[0], "pwd") == 0 ||
           strcmp(cmd->argv[0], "exit") == 0 ||
           strcmp(cmd->argv[0], "prompt") == 0;
}

int execute_builtin_command(const struct command *cmd) {
    if (strcmp(cmd->argv[0], "cd") == 0) {
        return change_directory(cmd->argv[1] ? cmd->argv[1] : getenv("HOME"));
    } else if (strcmp(cmd->argv[0], "pwd") == 0) {
        return print_working_directory();
    } else if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(cmd->argv[0], "prompt") == 0) {
        if (cmd->argv[1]) {
            set_prompt(cmd->argv[1]);
        } else {
            set_prompt(PROMPT_DEFAULT);
        }
        return 0;
    }

    return -1;
}

int execute_command(const struct command *cmd) {
    if (is_builtin_command(cmd)) {
        return execute_builtin_command(cmd);
    }

    pid_t pid = fork();

    if (pid == -1) {
        printf("Failed to fork: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0) {
        if (cmd->input_redir) {
            int fd = open(cmd->input_redir, O_RDONLY);
            if (fd == -1) {
                printf("Failed to open input file: %s\n", strerror(errno));
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (cmd->output_redir) {
            int fd = open(cmd->output_redir, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                printf("Failed to open output file: %s\n", strerror(errno));
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(cmd->argv[0], cmd->argv);
        printf("Failed to execute command: %s: %s\n", cmd->argv[0], strerror(errno));
        exit(1);
    }

    return pid;
}
void free_command(struct command *cmd) {
    if (!cmd) return;
    
    for (int i = 0; cmd->argv[i]; i++) {
        free(cmd->argv[i]);
    }

    free(cmd->input_redir);
    free(cmd->output_redir);
    free(cmd->argv);
    free(cmd);
}

void free_command_list(struct command_list *cmd_list) {
    if (!cmd_list) return;

    struct job *current_job = cmd_list->first_job;
    while (current_job) {
        struct job *next_job = current_job->next;

        struct command *current_cmd = current_job->first_command;
        while (current_cmd) {
            struct command *next_cmd = current_cmd->next;
            free_command(current_cmd);
            current_cmd = next_cmd;
        }

        free(current_job);
        current_job = next_job;
    }

    free(cmd_list);
}

