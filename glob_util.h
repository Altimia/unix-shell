// glob_util.h
// defines functions for expanding wildcard characters in command arguments
#ifndef GLOB_UTIL_H
#define GLOB_UTIL_H

#include <glob.h>

typedef struct Command {
    char *name;
    int argc;
    char **args;
} Command;

// Expands wildcards in the given command
void expand_wildcards(Command *cmd);

// Frees the memory associated with the given glob_t struct
void free_glob_result(glob_t *glob_result);

#endif // GLOB_UTIL_H