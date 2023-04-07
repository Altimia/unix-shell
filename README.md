# unix-shell
This project is a shell command interpreter that implements command separators, redirection, wildcard expressions, and a number of custom commands. The source code defines the data structure and functions required to handle these features, creating a lightweight and extensible shell environment.

Features
    Command Separators:
        Pipe separator: |
        Sequence separator: ;
        Background separator: &
    Command interpreter data structure
    Custom commands:
        prompt
        cd
        pwd
        exit
    Wildcard expressions
    Redirection handling
    Signal handling
    Memory management

Usage
    Compile the source code.
    Run the compiled binary to start the shell.
    Use the custom commands, as well as standard shell commands, with support for separators and redirection.
    Functions

Below is a brief description of the functions implemented in the project:
    is_sequence_seperator(): Checks if a token is a sequence separator.
    command_constructor(): Constructs a command from provided data.
    command_make(): Handles error checking and discovers command start and end in the token array.
    find_redirection(): Searches for redirection tokens in a command.
    cmd_argument_constructor(): Constructs a command's arguments.
    init_commands(): Initializes all commands.
    tokenise(): Creates an array of string tokens from a given input line.
    token_number(): Gets the number of tokens needed for a given input line.
    wildcard_check(): Checks for wildcard expressions in a command.
    wildcard_handle(): Handles wildcard expressions in a command.
    pwd_command(): Displays the current working directory.
    cd_command(): Changes the working directory.
    prompt_command(): Changes the command-line prompt.
    redirection(): Handles standard input and output redirections.
    pipe_command_execution(): Handles shell pipeline execution.
    command_execution(): Handles command execution.
    free_all(): Frees allocated memory for command arguments.
    handle_command_execution(): Handles command execution based on the command provided.
    handler(): Handles signals.
    init_signal(): Sets up signal handling.
    get_input(): Gets user input from the command line.

Custom Commands
    prompt: Changes the command-line prompt to a specified string.
    cd: Changes the working directory to a specified path.
    pwd: Displays the current working directory.
    exit: Exits the shell.