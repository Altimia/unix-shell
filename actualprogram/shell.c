/*
shell.c provides an interface for executing shell commands
It sets up the signal handlers for the signals SIGINT and SIGTSTP, 
which are responsible for handling interrupt and stop signals, respectively.
It enters an infinite loop that prompts the user to enter a command, 
reads the command line, parses the command line into a command list, and executes the command list.
If the command line cannot be read or parsed, an error message is displayed.
After executing the command list, it frees the memory allocated for the command list.
*/
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "commands.h"
#include "job.h"
#include "parser.h"

#define MAX_COMMAND_LEN 100
#define MAX_ARGS 1000
#define PROMPT_DEFAULT "Shell$ "

static char prompt[MAX_COMMAND_LEN] = PROMPT_DEFAULT;

void set_prompt(const char *new_prompt) {
    strncpy(prompt, new_prompt, MAX_COMMAND_LEN);
    prompt[MAX_COMMAND_LEN - 1] = '\0';
}

static void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n%s", prompt);
        fflush(stdout);
    }
}

int main() {
    char command_line[MAX_COMMAND_LEN];
    struct sigaction action;

    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

    while (true) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(command_line, MAX_COMMAND_LEN, stdin) == NULL) {
            printf("Failed to read command line.\n");
            continue;
        }

        struct command_list *cmd_list = parse_command_line(command_line);
        if (cmd_list == NULL) {
            printf("Failed to parse command line.\n");
            continue;
        }

        execute_command_list(cmd_list);
        free_command_list(cmd_list);
    }

    return 0;
}
