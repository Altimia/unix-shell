#ifndef PARSER_H
#define PARSER_H

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"

#define MAX_ARGS 128

// Represents a single command in a pipeline
struct command {
    char *argv[MAX_ARGS];            // Array of arguments
    char *input_redir;               // Input redirection filename
    char *output_redir;              // Output redirection filename
    struct command *next;            // Pointer to the next command in the pipeline
};

// Represents a list of jobs to be executed
struct command_list {
    struct job *first_job;           // Pointer to the first job in the list
};

// Parses a command line string and returns a command_list structure
struct command_list *parse_command_line(const char *command_line);

#endif // PARSER_H
