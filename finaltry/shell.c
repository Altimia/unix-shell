#include "commands.h"
#include <stdio.h>
#include "parser.h"
#include "job.h"

char shell_prompt[PROMPT_BUFFER] = "shell$ ";

int main() {
    char line[1024];
    while (1) {
        printf("%s", shell_prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        struct Job *job = parse_line(line);
        execute_job(job);
    }
    return 0;
}
