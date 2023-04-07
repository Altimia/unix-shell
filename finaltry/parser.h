#ifndef PARSER_H
#define PARSER_H

struct Command {
    char **argv;
    char *input;
    char *output;
    struct Command *pipe_to;
};

struct Job {
    struct Command *command;
    int background;
    int sequential;
    struct Job *next;
};

struct Job *parse_line(char *line);

#endif
