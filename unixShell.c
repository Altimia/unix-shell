#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>
#include <stdbool.h>

// Structure for storing command line information
typedef struct command {
    char **args;
    int arg_count;
    char *input_redirection;
    char *output_redirection;
    struct command *next;
} command_t;

typedef struct {
    command_t *commands;
    char **tokens;
    int token_count;
} command_line_info;

// Function prototypes
void tokenize_command_line(char *input, char ***tokens, int *token_count);
void parse_command_line(command_line_info *cmd_info);
bool is_builtin_command(char *command);
void execute_command_line(command_line_info *cmd_info);
void execute_builtin_command(const char *command, char **args);
void execute_non_builtin_command(command_t *command);
void expand_wildcard_characters(char **tokens, int token_count, char ***expanded_tokens, int *expanded_token_count);
//void handle_redirection(...);
//void execute_pipeline(...);
//void execute_background_job(...);
//void execute_sequential_job(...);

int main() {
    char *input;
    size_t buffer_size = 1024;
    
    while (1) {
        // Display the shell prompt
        printf("> ");
        
        // Read the input command line
        input = (char *)malloc(buffer_size * sizeof(char));
        getline(&input, &buffer_size, stdin);
        
        // Tokenize the input command line
        char **tokens;
        int token_count;
        tokenize_command_line(input, &tokens, &token_count);

        char **expanded_tokens;
        int expanded_token_count;
        expand_wildcard_characters(tokens, token_count, &expanded_tokens, &expanded_token_count);

        for (int i = 0; i < expanded_token_count; i++) {
            printf("Expanded token %d: %s\n", i, expanded_tokens[i]);
        }
        
        // Parse the command line structure
        //parse_command_line(...);

        // Create a command line info structure to store parsed information
        //command_line_info cmd_info;
        
        // TODO: Fill the cmd_info structure with the parsed information
        
        // Execute the command line
        //execute_command_line(&cmd_info);
        
        // Free the memory
        free(input);
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        for (int i = 0; i < expanded_token_count; i++) {
            free(expanded_tokens[i]);
        }
        free(expanded_tokens);
    }
    
    return 0;
}

void tokenize_command_line(char *input, char ***tokens, int *token_count) {
    *token_count = 0;
    *tokens = NULL;
    char *token = strtok(input, " \t\n");
    while (token) {
        *tokens = realloc(*tokens, (*token_count + 1) * sizeof(char *));
        (*tokens)[*token_count] = strdup(token);
        (*token_count)++;
        token = strtok(NULL, " \t\n");
    }
}

void parse_command_line(command_line_info *cmd_info) {
    command_t *current_command = NULL;
    command_t *last_command = NULL;

    for (int i = 0; i < cmd_info->token_count; i++) {
        char *token = cmd_info->tokens[i];

        if (strcmp(token, "|") == 0) {
            if (!current_command) {
                fprintf(stderr, "Invalid syntax: Unexpected '|'\n");
                exit(EXIT_FAILURE);
            }
            current_command = NULL;
        } else if (strcmp(token, "<") == 0) {
            if (!current_command || i + 1 >= cmd_info->token_count || !current_command->args[0]) {
                fprintf(stderr, "Invalid syntax: Unexpected '<'\n");
                exit(EXIT_FAILURE);
            }
            current_command->input_redirection = cmd_info->tokens[++i];
        } else if (strcmp(token, ">") == 0) {
            if (!current_command || i + 1 >= cmd_info->token_count || !current_command->args[0]) {
                fprintf(stderr, "Invalid syntax: Unexpected '>'\n");
                exit(EXIT_FAILURE);
            }
            current_command->output_redirection = cmd_info->tokens[++i];
        } else {
            if (!current_command) {
                current_command = calloc(1, sizeof(command_t));
                current_command->args = calloc(cmd_info->token_count + 1, sizeof(char *));
                current_command->arg_count = 0;

                if (last_command) {
                    last_command->next = current_command;
                } else {
                    cmd_info->commands = current_command;
                }
            }

            current_command->args[current_command->arg_count++] = token;
            last_command = current_command;
        }
    }
}

bool is_builtin_command(char *command) {
    if (strcmp(command, "prompt") == 0 || strcmp(command, "pwd") == 0 ||
        strcmp(command, "cd") == 0 || strcmp(command, "exit") == 0) {
        return true;
    }
    return false;
}

void execute_builtin_command(const char *command, char **args) {
    if (strcmp(command, "prompt") == 0) {
        // Change shell prompt
        if (args[1]) {
            // Assume the shell prompt variable is a global variable or accessible in some other way
            strcpy(shell_prompt, args[1]);
        } else {
            fprintf(stderr, "Usage: prompt <new_prompt>\n");
        }
    } else if (strcmp(command, "pwd") == 0) {
        // Print working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("Error getting current directory");
        }
    } else if (strcmp(command, "cd") == 0) {
        // Change directory
        if (args[1]) {
            if (chdir(args[1]) != 0) {
                perror("Error changing directory");
            }
        } else {
            // Change to user's home directory if no path is provided
            const char *home = getenv("HOME");
            if (home) {
                chdir(home);
            } else {
                fprintf(stderr, "Error: HOME environment variable not set\n");
            }
        }
    } else if (strcmp(command, "exit") == 0) {
        // Exit shell
        exit(0);
    }
}

void execute_non_builtin_command(command_t *command) {
    // TODO: Implement the execution of non-built-in commands, handling
    // input/output redirection, pipelines, background jobs, and sequential jobs
}

void execute_command_line(command_line_info *cmd_info) {
    command_t *current_command = cmd_info->commands;

    while (current_command) {
        if (is_builtin_command(current_command->args[0])) {
            execute_builtin_command(current_command);
        } else {
            execute_non_builtin_command(current_command);
        }
        current_command = current_command->next;
    }
}

void expand_wildcard_characters(char **tokens, int token_count, char ***expanded_tokens, int *expanded_token_count) {
    *expanded_token_count = 0;
    *expanded_tokens = calloc(token_count, sizeof(char *));

    for (int i = 0; i < token_count; i++) {
        char *token = tokens[i];

        if (strchr(token, '*') || strchr(token, '?')) {
            glob_t glob_result;
            glob(token, 0, NULL, &glob_result);

            *expanded_tokens = realloc(*expanded_tokens, (*expanded_token_count + glob_result.gl_pathc) * sizeof(char *));

            for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                (*expanded_tokens)[*expanded_token_count] = strdup(glob_result.gl_pathv[j]);
                (*expanded_token_count)++;
            }

            globfree(&glob_result);
        } else {
            (*expanded_tokens)[*expanded_token_count] = strdup(token);
            (*expanded_token_count)++;
        }
    }
}

void handle_redirection(...) {
    // TODO: Implement support for input and output redirection
}

void execute_pipeline(...) {
    // TODO: Implement support for the pipeline symbol '|'
}

void execute_background_job(...) {
    // TODO: Implement support for executing background jobs using the '&' symbol
}

void execute_sequential_job(...) {
    // TODO: Implement support for sequential job execution using the ';' symbol
}
