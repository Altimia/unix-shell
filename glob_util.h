// glob_util.h
// defines functions for expanding wildcard characters in command arguments
#ifndef GLOB_UTIL_H
#define GLOB_UTIL_H

#include <glob.h>
#include "simple_shell.h"

void expand_wildcards(Command *cmd);
void free_glob_result(glob_t *glob_result);

#endif // GLOB_UTIL_H
