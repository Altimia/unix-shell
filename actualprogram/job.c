/*
jobs.c implements the functions execute_job and execute_command_list 
which are used to execute a list of jobs (collections of commands).
The function execute_job takes a pointer to a struct job as an argument 
and executes the commands in the job. It sets up pipes between the commands to 
handle the output of one command being used as the input of the next command. 
It also handles background jobs and foreground jobs differently by using waitpid 
to wait for the last command in the job to finish if the job is a foreground job, 
and not waiting if it's a background job.
The function execute_command_list takes a pointer to a struct command_list 
as an argument and executes the jobs in the list. It calls execute_job for each job in the list, 
and waits for the previous job to finish if the next job is not a background job.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "commands.h"
#include "job.h"
#include "parser.h"

int execute_job(const struct job *job) {
    int num_commands = 0;
    const struct command *cmd = job->first_command;
    while (cmd != NULL) {
        num_commands++;
        cmd = cmd->next;
    }

    int pipe_fds[num_commands - 1][2];
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    int status = 0;
    cmd = job->first_command;
    for (int i = 0; cmd != NULL; cmd = cmd->next, i++) {
        int pid = execute_command(cmd);
        if (pid == -1) {
            status = -1;
            break;
        }

        if (i > 0) {
            dup2(pipe_fds[i - 1][0], STDIN_FILENO);
            close(pipe_fds[i - 1][0]);
            close(pipe_fds[i - 1][1]);
        }

        if (i < num_commands - 1) {
            dup2(pipe_fds[i][1], STDOUT_FILENO);
            close(pipe_fds[i][0]);
            close(pipe_fds[i][1]);
        }

        if (!job->background && cmd->next == NULL) {
            waitpid(pid, &status, 0);
        } else {
            printf("[%d] %d\n", i + 1, pid);
        }
    }

    return status;
}

int execute_command_list(const struct command_list *cmd_list) {
    int status = 0;
    const struct job *job = cmd_list->first_job;
    while (job != NULL) {
        int job_status = execute_job(job);
        if (job_status == -1) {
            status = -1;
        }

        if (!job->background && job->next != NULL) {
            wait(NULL);
        }

        job = job->next;
    }

    return status;
}
