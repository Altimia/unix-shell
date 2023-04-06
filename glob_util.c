// glob_util.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "glob_util.h"

static void add_arguments(Command *cmd, glob_t *glob_result) {
    for (size_t i = 0; i < glob_result->gl_pathc; i++) {
        char *arg = strdup(glob_result->gl_pathv[i]);
        if (!arg) {
            perror("strdup");
            continue;
        }
        cmd->args = realloc(cmd->args, sizeof(char *) * (cmd->argc + 2));
        if (!cmd->args) {
            perror("realloc");
            free(arg);
            break;
        }
        cmd->args[cmd->argc++] = arg;
        cmd->args[cmd->argc] = NULL;
    }
}

void expand_wildcards(Command *cmd) {
    if (!cmd || !cmd->args) {
        return;
    }

    Command new_cmd = {0};
    new_cmd.name = strdup(cmd->name);
    new_cmd.args = malloc(sizeof(char *));
    new_cmd.args[0] = strdup(cmd->name);
    new_cmd.argc = 1;

    for (size_t i = 1; cmd->args[i]; i++) {
        if (strpbrk(cmd->args[i], "*?")) {
            glob_t glob_result;
            int ret = glob(cmd->args[i], 0, NULL, &glob_result);
            if (ret == 0) {
                add_arguments(&new_cmd, &glob_result);
                globfree(&glob_result);
            } else {
                fprintf(stderr, "Error: glob: %s\n", strerror(ret));
            }
        } else {
            new_cmd.args = realloc(new_cmd.args, sizeof(char *) * (new_cmd.argc + 2));
            if (!new_cmd.args) {
                perror("realloc");
                break;
            }
            new_cmd.args[new_cmd.argc++] = strdup(cmd->args[i]);
            new_cmd.args[new_cmd.argc] = NULL;
        }
    }

    // Free old command arguments
    for (size_t i = 0; cmd->args[i]; i++) {
        free(cmd->args[i]);
    }
    free(cmd->args);

    // Replace the old command with the new one
    cmd->args = new_cmd.args;
    cmd->argc = new_cmd.argc;
}

void free_glob_result(glob_t *glob_result) {
    globfree(glob_result);
}
