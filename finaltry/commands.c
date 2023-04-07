#include "commands.h"
#include "job.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int change_directory(char *path) {
    if (chdir(path) == -1) {
        perror("cd");
        return 1;
    }
    return 0;
}

int print_working_directory() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        return 1;
    }
    printf("%s\n", cwd);
    return 0;
}

int change_prompt(char *new_prompt) {
    strncpy(shell_prompt, new_prompt, PROMPT_BUFFER);
    shell_prompt[PROMPT_BUFFER - 1] = '\0';
    return 0;
}

int exit_shell() {
    exit(0);
}

void execute_command(struct Command *command) {
    if (!command || !command->argv || !command->argv[0]) {
        return;
    }

    // Check for built-in commands
    if (strcmp(command->argv[0], "cd") == 0) {
        change_directory(command->argv[1]);
        return;
    }

    if (strcmp(command->argv[0], "pwd") == 0) {
        print_working_directory();
        return;
    }

    if (strcmp(command->argv[0], "prompt") == 0) {
        change_prompt(command->argv[1]);
        return;
    }

    if (strcmp(command->argv[0], "exit") == 0) {
        exit_shell();
        return;
    }

    // Execute external commands
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        if (command->input) {
            int fd = open(command->input, O_RDONLY);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (command->output) {
            int fd = open(command->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(command->argv[0], command->argv);
        perror("execvp");
        exit(1);
    } else if (pid < 0) {
        // Error in fork()
        perror("fork");
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}