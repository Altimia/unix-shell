#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Max number of commands
#define MAX_CMDS 100

// Command separators
#define pSep '|'
#define sSep ';'
#define aSep '&'

/*
 * Command Structure
 * Desc: Data structure used for command management
*/
typedef struct CommandStructure
{
	// command start in token array
	int first;

	// command end in token array
	int last;

	// command separator
	char sep;

	// command arguments
	char **argv;

	// Redirction files
	char *in_file;
	char *out_file;
}Command;

/*
 * Desc: Check if token is a separator
 * Params
 * 	character token - token to be checked
 * Return: integer - true or false 
 * Notes: NA
*/
int isSep(char *token);

/*
 * Desc: Constructs command 
 * Params
 * 	Command c - command to be created 
 * 	integer first - command start in array
 * 	integer last - command end in array
 * 	char sep - command separator
 * Return: void
 * Notes: NA
*/
void command_constructor(Command *c, int first, int last, char *sep);

/*
 * Desc: Various error checking and discovers command start and end in token array 
 * Params
 *     	integer tok_num - number of tokens 
 *      character tokens[] - pointer to array of tokens
 *      Command command[] - array of commands
 * Return: integer - number of commands
 * Notes: NA
*/
int command_make(int tok_num, char* tokens[], Command command[]);

/*
 * Desc: Search for redirection tokens
 * Params
 * 	Command cmd - pointer to command
 *      character tokens[] - pointer to array of tokens
 * Return: void
 * Notes: NA
*/
void search_redirection(char* tokens[], Command *cmd);

/*
 * Desc: Constructs command's arguments 
 * Params
 * 	Command cmd - pointer to command
 *      character tokens[] - pointer to array of tokens
 * Return: void
 * Notes: NA
*/
void command_args_make(char* tokens[], Command *cmd);

/*
 * Desc: Initialises all commands 
 * Params
 *      Command c[] - array of commands 
 * Return: void
 * Notes: NA
*/
void init_commands(Command c[]);

