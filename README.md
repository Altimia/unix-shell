# unix-shell
This code is a simple implementation of a custom shell in C. It supports basic shell functionalities such as input and output redirection, background jobs, pipelines, and a few built-in commands (cd, pwd, exit, and prompt).

The main function runs a loop that reads input, tokenizes the input command line, parses the command line structure, executes the command line, frees the token list memory, and frees the input memory.

tokenize_command_line() tokenizes the input command line into separate words and stores them in an array.

free_token_list() frees the memory allocated for the token list.

parse_command_line() parses the token list to fill a Command structure, determining the type of command, input and output files, and whether it's a background job or part of a pipeline.

execute_command_line() iterates through the jobs separated by ';', handles redirection, and executes the background job or foreground job accordingly.

execute_builtin_command() executes built-in commands like "cd", "exit", "pwd", and "prompt".

change_prompt() changes the shell prompt to the given string.

is_builtin_command() checks if the given command is a built-in command.

expand_wildcards() expands wildcard patterns (such as '*' and '?') in the command line.

handle_redirection() handles input and output redirection by opening the appropriate files and redirecting the standard input and output.

restore_standard_io() restores standard input and output after redirection.

execute_pipeline() executes a command as part of a pipeline by creating a pipe between commands and forking child processes to execute each command.

execute_background_job() forks a child process to execute a command in the background, handling input and output redirection as needed.

init_command_line() initializes a CommandLine structure with default values.

sigchld_handler() is a signal handler for the SIGCHLD signal, which is sent to the parent process when a child process terminates. This function ensures that the child processes are reaped properly.
