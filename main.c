#include <stdio.h>
#include "simple_shell.h"

int main() {
    while (1) {
        printf("%s", get_prompt());
        char input[4096];
        fgets(input, sizeof(input), stdin);

        CommandLine *cmd_line = parse_command_line(input);
        execute_command_line(cmd_line);
        free_command_line(cmd_line);
    }

    return 0;
}
