#ifndef SHELL_H
#define SHELL_H

#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "commands.h"
#include "job.h"
#include "parser.h"

#define MAX_COMMAND_LEN 100
#define MAX_ARGS 1000
#define PROMPT_DEFAULT "Shell$ "

void set_prompt(const char *new_prompt);

#endif