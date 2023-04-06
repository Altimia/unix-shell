#ifndef COMMANDS_H
#define COMMANDS_H

#include "parser.h"

int change_directory(const char *path);
int print_working_directory();
int is_builtin_command(const struct command *cmd);
int execute_builtin_command(const struct command *cmd);
int execute_command(const struct command *cmd);

#endif // COMMANDS_H
