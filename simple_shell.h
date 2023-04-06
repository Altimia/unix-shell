// simple_shell.h
// defines structures (Command, Job, and CommandLine) and related functions.
#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <stdbool.h>

#define MAX_COMMANDS 100
#define MAX_ARGUMENTS 1000
#define PROMPT_SIZE 256

typedef struct {
    char *name;
    char **args;
    char *input_redirect;
    char *output_redirect;
} Command;

typedef struct {
    Command *commands[MAX_COMMANDS];
    int count;
    bool background;
    bool sequential;
} Job;

typedef struct {
    Job *jobs[MAX_COMMANDS];
    int count;
} CommandLine;

CommandLine *parse_command_line(const char *input);
void free_command_line(CommandLine *cmd_line);

int execute_command_line(const CommandLine *cmd_line);

#endif // SIMPLE_SHELL_H
