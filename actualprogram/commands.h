#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>

#define MAX_ARGS 1000

struct command {
    char **argv;
    char *input_redir;
    char *output_redir;
    struct command *next;
};

int change_directory(const char *path);
int print_working_directory();
bool is_builtin_command(const struct command *cmd);
int execute_builtin_command(const struct command *cmd);
int execute_command(const struct command *cmd);

#endif // COMMANDS_H
