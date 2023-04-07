#ifndef COMMANDS_H
#define COMMANDS_H

#define PROMPT_BUFFER 256
extern char shell_prompt[PROMPT_BUFFER];

int change_directory(char *path);
int print_working_directory();
int change_prompt(char *new_prompt);
int exit_shell();

#endif
