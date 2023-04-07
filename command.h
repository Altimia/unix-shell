#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Max number of commands
#define MAX_CMDS 100

// Command separators
#define pipeSep '|'
#define sequenceSep ';'
#define backSep '&'

// Data structure used for command management
typedef struct CommandStructure {
	// command start in token array
	int first;

	// command end in token array
	int last;

	// command separator
	char seperator;

	// command arguments
	char **argv;

	// Redirction files
	char *in_file;
	char *out_file;
}Command;

// Check if token is a separator
int is_sequence_seperator(char *token);

// Constructs command 
void command_constructor(Command *c, int first, int last, char *seperator);

// Various error checking and discovers command start and end in token array 
int parse_command(int tokenNum, char* tokens[], Command command[]);

// Search for redirection tokens
void find_redirection(char* tokens[], Command *cmd);

// Constructs command's arguments 
void cmd_argument_constructor(char* tokens[], Command *cmd);

// Desc: Initialises all commands 
void init_commands(Command c[]);

