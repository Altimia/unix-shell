#ifndef JOB_H
#define JOB_H

#include "parser.h"

int execute_job(const struct job *job);
int execute_command_list(const struct command_list *cmd_list);

#endif // JOB_H
