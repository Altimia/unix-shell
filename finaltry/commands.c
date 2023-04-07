#include "commands.h"
#include "job.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
