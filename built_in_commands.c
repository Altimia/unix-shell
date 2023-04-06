// built_in_commands.c
// implements the functions defined in the built_in_commands.h header file
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "built_in_commands.h"

static char prompt[PROMPT_SIZE] = "simple_shell$ ";

// updates the shell prompt
int change_prompt(const char *new_prompt) {
    if (new_prompt) {
        snprintf(prompt, PROMPT_SIZE, "%s ", new_prompt);
    } else {
        snprintf(prompt, PROMPT_SIZE, "simple_shell$ ");
    }
    return 0;
}

// returns the current prompt string
const char *get_prompt(void) {
    return prompt;
}

// prints the current working directory
int print_working_directory(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return 1;
    }
    printf("%s\n", cwd);
    return 0;
}

// changes the working directory
int change_directory(const char *path) {
    if (!path) {
        path = getenv("HOME");
    }

    if (chdir(path) == -1) {
        perror("chdir");
        return 1;
    }
    return 0;
}

// exits the shell program
int exit_shell(void) {
    exit(0);
}
