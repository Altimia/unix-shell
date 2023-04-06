// built_in_commands.h
// defines functions for the built-in commands ("prompt", "pwd", "cd", and "exit").
#ifndef BUILT_IN_COMMANDS_H
#define BUILT_IN_COMMANDS_H

int change_prompt(const char *new_prompt);
int print_working_directory(void);
int change_directory(const char *path);
int exit_shell(void);

#endif // BUILT_IN_COMMANDS_H
