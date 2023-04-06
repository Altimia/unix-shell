// built_in_commands.h
// defines functions for the built-in commands ("prompt", "pwd", "cd", and "exit").
#ifndef BUILT_IN_COMMANDS_H
#define BUILT_IN_COMMANDS_H

#define PROMPT_SIZE 256

// Updates the shell prompt
int change_prompt(const char *new_prompt);

// Returns the current prompt string
const char *get_prompt(void);

// Prints the current working directory
int print_working_directory(void);

// Changes the working directory
int change_directory(const char *path);

// Exits the shell program
int exit_shell(void);

#endif // BUILT_IN_COMMANDS_H