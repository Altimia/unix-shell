#include "job.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "parser.h"

void execute_job(struct Job *job) {
    if (!job) {
        return;
    }

    struct Command *command = job->command;
    while (command) {
        execute_command(command);
        command = command->pipe_to;
    }
}

void free_job(struct Job *job) {
    if (!job) {
        return;
    }

    struct Command *command = job->command;
    while (command) {
        struct Command *next = command->pipe_to;
        free_command(command);
        command = next;
    }
    free(job);
}