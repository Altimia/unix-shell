// simple_shell.h
// defines structures (Command, Job, and CommandLine) and related functions.
#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <sys/types.h>
#include "glob_util.h"

typedef struct CommandLine {
    int count;
    struct Job **jobs;
} CommandLine;

typedef struct Job {
    int count;
    Command **commands;
} Job;

// Parses the input string according to the specified Extended BNF grammar
// and returns a CommandLine struct
CommandLine *parse_command_line(const char *input);

// Frees the memory associated with the given CommandLine struct
void free_command_line(CommandLine *cmd_line);

// Executes the given command and handles input/output redirections,
// pipelines, and background/sequential job execution
pid_t execute_command(const Command *cmd);

// Iterates over the parsed command line jobs and commands,
// expanding wildcards and executing built-in commands
// or external commands as needed
int execute_command_line(const CommandLine *cmd_line);

#endif // SIMPLE_SHELL_H
