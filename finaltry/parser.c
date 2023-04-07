#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>

#define TOKEN_BUFFER 256

static char *skip_spaces(const char *str) {
    while (*str && *str <= ' ') {
        str++;
    }
    return (char *)str;
}

static struct Command *create_command() {
    struct Command *command = malloc(sizeof(struct Command));
    command->argv = NULL;
    command->input = NULL;
    command->output = NULL;
    command->pipe_to = NULL;
    return command;
}

static struct Job *create_job() {
    struct Job *job = malloc(sizeof(struct Job));
    job->command = NULL;
    job->background = 0;
    job->sequential = 0;
    job->next = NULL;
    return job;
}

static void free_command(struct Command *command) {
    if (command->argv) {
        free(command->argv);
    }
    if (command->input) {
        free(command->input);
    }
    if (command->output) {
        free(command->output);
    }
    free(command);
}

static void free_job(struct Job *job) {
    if (job->command) {
        free_command(job->command);
    }
    free(job);
}

static char **add_argument(char **argv, int argc, const char *arg) {
    char **new_argv = realloc(argv, (argc + 2) * sizeof(char *));
    if (!new_argv) {
        perror("realloc");
        exit(1);
    }

    new_argv[argc] = strdup(arg);
    new_argv[argc + 1] = NULL;
    return new_argv;
}

static char *parse_token(char *str, struct Command *command, int *argc) {
    glob_t globbuf;
    int glob_flags = GLOB_NOCHECK | GLOB_TILDE;

    char token[TOKEN_BUFFER];
    char *p = token;
    int quote = 0;
    while (*str && *str > ' ') {
        if (*str == '\\') {
            str++;
            if (*str) {
                *p++ = *str++;
            }
        } else if (*str == '\'' || *str == '"') {
            quote = *str++;
            while (*str && *str != quote) {
                if (*str == '\\') {
                    str++;
                }
                if (*str) {
                    *p++ = *str++;
                }
            }
            if (*str) {
                str++;
            }
        } else {
            *p++ = *str++;
        }
    }
    *p = '\0';

    if (glob(token, glob_flags, NULL, &globbuf) == 0) {
        for (size_t i = 0; i < globbuf.gl_pathc; i++) {
            command->argv = add_argument(command->argv, (*argc)++, globbuf.gl_pathv[i]);
        }
        globfree(&globbuf);
    }

    return str;
}

static char *parse_redirect(char *str, struct Command *command) {
    str = skip_spaces(str);
    if (*str == '<') {
        str++;
        str = skip_spaces(str);
        str = parse_token(str, command, &(int){0});
        command->input = strdup(command->argv[0]);
        free(command->argv[0]);
        command->argv[0] = NULL;
    } else if (*str == '>') {
                str++;
        str = skip_spaces(str);
        str = parse_token(str, command, &(int){0});
        command->output = strdup(command->argv[0]);
        free(command->argv[0]);
        command->argv[0] = NULL;
    }
    return str;
}

static char *parse_command(char *str, struct Command *command) {
    int argc = 0;

    str = skip_spaces(str);
    while (*str && *str > ' ' && *str != '|' && *str != '<' && *str != '>') {
        str = parse_token(str, command, &argc);
        str = skip_spaces(str);
    }

    if (*str == '<' || *str == '>') {
        str = parse_redirect(str, command);
        str = skip_spaces(str);
    }

    return str;
}

static char *parse_job(char *str, struct Job *job) {
    struct Command *command = create_command();
    job->command = command;

    str = parse_command(str, command);
    while (*str == '|') {
        str++;
        command->pipe_to = create_command();
        command = command->pipe_to;
        str = parse_command(str, command);
    }

    return str;
}

struct Job *parse_line(char *line) {
    struct Job *job = create_job();
    struct Job *current_job = job;

    line = skip_spaces(line);
    while (*line) {
        line = parse_job(line, current_job);
        if (*line == '&') {
            current_job->background = 1;
            line++;
        } else if (*line == ';') {
            current_job->sequential = 1;
            line++;
        }

        line = skip_spaces(line);
        if (*line) {
            current_job->next = create_job();
            current_job = current_job->next;
        }
    }

    return job;
}

