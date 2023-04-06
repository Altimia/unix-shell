#ifndef JOB_H
#define JOB_H

#include "commands.h"

struct job {
    struct command *first_command;
    struct job *next;
    bool background;
};

struct command_list {
    struct job *first_job;
};

int execute_command(const struct command *cmd);
int execute_command_list(const struct command_list *cmd_list);
void free_command_list(struct command_list *cmd_list);


#endif // JOB
