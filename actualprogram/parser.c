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
#include <stdio.h>
#include <glob.h>

//takes a string pointer as input and 
//returns a pointer to a string that represents the next token in the string
static char *parse_token(char **str_ptr) {
    char *str = *str_ptr;
    while (isspace(*str)) str++;
    if (*str == '\0') return NULL;

    char *start = str;
    if (strchr("&;|<>", *str)) {
        str++;
    } else {
        while (*str != '\0' && !isspace(*str) && !strchr("&;|<>", *str)) {
            if (*str == '\\') {
                str++;
                if (*str == '\0') break;
            }
            str++;
        }
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

    glob_t globbuf;
    int glob_flags = GLOB_NOCHECK | GLOB_APPEND;
    bool glob_initialized = false;

    while (true) {
        char *token = parse_token(str_ptr);
        if (token == NULL) break;
    
        if (strpbrk(token, "*?") != NULL) {
            if (!glob_initialized) {
                glob(token, GLOB_NOCHECK, NULL, &globbuf);
                glob_initialized = true;
            } else {
                glob(token, glob_flags, NULL, &globbuf);
            }
        } else {
            if (!glob_initialized) {
                globbuf.gl_pathv = NULL;
                globbuf.gl_pathc = 0;
                glob_initialized = true;
            }
            if (strcmp(token, ";") == 0) {
                cmd->type = CMD_SEQ;
                break;
            } else if (strcmp(token, "&") == 0) {
                cmd->type = CMD_BACKGROUND;
                break;
            } else if (strcmp(token, "|") == 0) {
                cmd->type = CMD_PIPE;
                break;
            } else if (strcmp(token, ">") == 0) {
                cmd->type = CMD_REDIRECT_OUT;
                break;
            } else if (strcmp(token, "<") == 0) {
                cmd->type = CMD_REDIRECT_IN;
                break;
            } else {
            globbuf.gl_pathv = realloc(globbuf.gl_pathv, (globbuf.gl_pathc + 2) * sizeof(*globbuf.gl_pathv));
            globbuf.gl_pathv[globbuf.gl_pathc++] = strdup(token);
            globbuf.gl_pathv[globbuf.gl_pathc] = NULL;
        }

        free(token);

        if (globbuf.gl_pathc >= MAX_ARGS) {
            printf("Too many arguments in command\n");
            free_command(cmd);
            return NULL;
        }
    }

    if (globbuf.gl_pathc == 0) {
        free_command(cmd);
        return NULL;
    }

    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
        cmd->argv[argc++] = strdup(globbuf.gl_pathv[i]);
    }
    cmd->argv[argc] = NULL;
    
    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
        free(globbuf.gl_pathv[i]);
    }
    free(globbuf.gl_pathv);

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
            } else if (*str_ptr == '|') {
                str_ptr++;
                struct command *next_cmd = parse_command(&str_ptr);
                cmd->next = next_cmd;
                current_cmd = next_cmd;
            }
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
        while (isspace(*str_ptr)) str_ptr++;
    }

    free(str);
    return cmd_list;
}
