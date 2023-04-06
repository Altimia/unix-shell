/*
parser.c implements functions that parse a command line string into a data structure 
that represents a list of jobs, each job being a sequence of commands.
*/
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"
#include "parser.h"

//takes a string pointer as input and 
//returns a pointer to a string that represents the next token in the string
static char *parse_token(char **str_ptr) {
    char *str = *str_ptr;
    while (isspace(*str)) str++;
    if (*str == '\0') return NULL;

    char *start = str;
    while (*str != '\0' && !isspace(*str) && !strchr("&;|<>", *str)) {
        if (*str == '\\') {
            str++;
            if (*str == '\0') break;
        }
        str++;
    }

    size_t length = str - start;
    char *token = malloc(length + 1);
    strncpy(token, start, length);
    token[length] = '\0';

    *str_ptr = str;
    return token;
}

//takes a string pointer as input and returns 
//a pointer to a struct command that represents a single command.
static struct command *parse_command(char **str_ptr) {
    struct command *cmd = calloc(1, sizeof(struct command));
    cmd->argv = calloc(MAX_ARGS + 1, sizeof(char *));
    int argc = 0;

    while (true) {
        cmd->argv[argc] = parse_token(str_ptr);
        if (cmd->argv[argc] == NULL) break;
        argc++;

        if (argc >= MAX_ARGS) {
            printf("Too many arguments in command\n");
            free_command(cmd);
            return NULL;
        }
    }

    if (argc == 0) {
        free_command(cmd);
        return NULL;
    }

    return cmd;
}

//takes a string command_line as input and returns a 
//pointer to a struct command_list that represents the list of jobs in the command line. 
struct command_list *parse_command_line(const char *command_line) {
    struct command_list *cmd_list = calloc(1, sizeof(struct command_list));
    struct job *current_job = NULL;
    struct command *current_cmd = NULL;

    char *str = strdup(command_line);
    char *str_ptr = str;

    while (*str_ptr != '\0') {
        struct command *cmd = parse_command(&str_ptr);

        if (cmd == NULL) break;

        if (current_cmd == NULL) {
            current_cmd = cmd;
        } else {
            current_cmd->next = cmd;
            current_cmd = cmd;
        }

        if (current_job == NULL) {
            current_job = calloc(1, sizeof(struct job));
            current_job->first_command = current_cmd;
            cmd_list->first_job = current_job;
        }

        while (isspace(*str_ptr)) str_ptr++;

        if (*str_ptr == '|' || *str_ptr == '<' || *str_ptr == '>') {
            if (*str_ptr == '<') {
                cmd->input_redir = parse_token(&str_ptr);
            } else if (*str_ptr == '>') {
                cmd->output_redir = parse_token(&str_ptr);
            }
            str_ptr++;
        } else if (*str_ptr == '&' || *str_ptr == ';') {
            struct job *new_job = calloc(1, sizeof(struct job));
            current_job->next = new_job;
            current_job = new_job;
            current_cmd = NULL;
            str_ptr++;

            if (*str_ptr == '&') {
                current_job->background = true;
            }
        }
    }

    free(str);
    return cmd_list;
}
