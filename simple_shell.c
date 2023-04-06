// simple_shell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "simple_shell.h"
#include "built_in_commands.h"
#include "glob_util.h"
#include <fcntl.h>

// ... Add any necessary helper functions here ...

// parse the input string according to the specified
// Extended BNF grammar and return a CommandLine struct.
CommandLine *parse_command_line(const char *input) {
    if (!input) {
        return NULL;
    }

    CommandLine *cmd_line = calloc(1, sizeof(CommandLine));
    cmd_line->count = 0;
    cmd_line->jobs = NULL;

    Job *current_job = NULL;
    Command *current_command = NULL;

    char *token;
    char *remaining = strdup(input);
    char *separator = " \t\n;&|<>";

    while ((token = strtok_r(remaining, separator, &remaining))) {
        // If token is a special character, update the current job/command
        if (strcmp(token, "&") == 0 || strcmp(token, ";") == 0) {
            if (current_command) {
                if (!current_job) {
                    current_job = calloc(1, sizeof(Job));
                }
                current_job->count++;
                current_job->commands = realloc(current_job->commands, current_job->count * sizeof(Command *));
                current_job->commands[current_job->count - 1] = current_command;
            }

            if (current_job) {
                cmd_line->count++;
                cmd_line->jobs = realloc(cmd_line->jobs, cmd_line->count * sizeof(Job *));
                cmd_line->jobs[cmd_line->count - 1] = current_job;
            }

            current_command = NULL;
            current_job = NULL;
        } else if (strcmp(token, "|") == 0) {
            if (current_command) {
                if (!current_job) {
                    current_job = calloc(1, sizeof(Job));
                }
                current_job->count++;
                current_job->commands = realloc(current_job->commands, current_job->count * sizeof(Command *));
                current_job->commands[current_job->count - 1] = current_command;
            }

            current_command = NULL;
        } else if (strcmp(token, "<") == 0) {
            if (current_command) {
                token = strtok_r(remaining, separator, &remaining);
                if (token) {
                    current_command->input_file = strdup(token);
                }
            }
        } else if (strcmp(token, ">") == 0) {
            if (current_command) {
                token = strtok_r(remaining, separator, &remaining);
                if (token) {
                    current_command->output_file = strdup(token);
                }
            }
        } else {
            // Regular token, add it to the current command
            if (!current_command) {
                current_command = calloc(1, sizeof(Command));
                current_command->name = strdup(token);
                current_command->arg_count = 1;
                current_command->args = calloc(current_command->arg_count + 1, sizeof(char *));
                current_command->args[0] = strdup(token);
                current_command->args[current_command->arg_count] = NULL;
            } else {
                current_command->arg_count++;
                current_command->args = realloc(current_command->args, (current_command->arg_count + 1) * sizeof(char *));
                current_command->args[current_command->arg_count - 1] = strdup(token);
                current_command->args[current_command->arg_count] = NULL;
            }
        }
    }

    if (current_command) {
        if (!current_job) {
            current_job = calloc(1, sizeof(Job));
        }
        current_job->count++;
        current_job->commands = realloc(current_job->commands, current_job->count * sizeof(Command *));
        current_job->commands[current_job->count - 1] = current_command;
    }

    if (current_job) {
        cmd_line->count++;
        cmd_line->jobs = realloc(cmd_line->jobs, cmd_line->count * sizeof(Job *));
        cmd_line->jobs[cmd_line->count - 1] = current_job;
    }

    free(remaining_copy);
    return cmd_line;
}


// free the memory associated with the given CommandLine struct.
void free_command(Command *cmd) {
    if (!cmd) {
        return;
    }

    if (cmd->name) {
        free(cmd->name);
    }

    if (cmd->args) {
        for (int i = 0; cmd->args[i]; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }

    if (cmd->input_file) {
        free(cmd->input_file);
    }

    if (cmd->output_file) {
        free(cmd->output_file);
    }

    free(cmd);
}

void free_job(Job *job) {
    if (!job) {
        return;
    }

    if (job->commands) {
        for (int i = 0; i < job->count; i++) {
            free_command(job->commands[i]);
        }
        free(job->commands);
    }

    free(job);
}

void free_command_line(CommandLine *cmd_line) {
    if (!cmd_line) {
        return;
    }

    if (cmd_line->jobs) {
        for (int i = 0; i < cmd_line->count; i++) {
            free_job(cmd_line->jobs[i]);
        }
        free(cmd_line->jobs);
    }

    free(cmd_line);
}

// Eexecute the given command and handle input/output redirections,
// pipelines, and background/sequential job execution.
pid_t execute_command(const Command *cmd) {
    if (!cmd) {
        return -1;
    }

    int in_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;

    // Handle input redirection
    if (cmd->input_file) {
        in_fd = open(cmd->input_file, O_RDONLY);
        if (in_fd == -1) {
            perror("open");
            return -1;
        }
    }

    // Handle output redirection
    if (cmd->output_file) {
        out_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1) {
            perror("open");
            close(in_fd);
            return -1;
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            close(out_fd);
        }
        return -1;
    }

    if (pid == 0) {
        // Child process
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execvp(cmd->name, cmd->args);
        perror("execvp");
        exit(1);
    } else {
        // Parent process
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            close(out_fd);
        }
        return pid;
    }
}

// iterates over the parsed command line jobs and commands, 
// expanding wildcards and executing built-in commands 
// or external commands as needed.
int execute_command_line(const CommandLine *cmd_line) {
    if (!cmd_line) {
        return 1;
    }

    for (int i = 0; i < cmd_line->count; i++) {
        Job *job = cmd_line->jobs[i];
        if (!job) {
            continue;
        }

        for (int j = 0; j < job->count; j++) {
            Command *cmd = job->commands[j];
            if (!cmd) {
                continue;
            }

            expand_wildcards(cmd);

            // Check for built-in commands
            if (strcmp(cmd->name, "prompt") == 0) {
                change_prompt(cmd->args[1]);
            } else if (strcmp(cmd->name, "pwd") == 0) {
                print_working_directory();
            } else if (strcmp(cmd->name, "cd") == 0) {
                change_directory(cmd->args[1]);
            } else if (strcmp(cmd->name, "exit") == 0) {
                exit_shell();
            } else {
                // Execute the external command
                execute_command(cmd);
            }
        }
    }

    return 0;
}

