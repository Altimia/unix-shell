#include "command.h"

int is_sequence_seperator(char *token) {
	int i = 0;
	char cmdSep[4] = {pipeSep, sequenceSep, backSep, '\0'};

	while (cmdSep[i] != '\0') {
		if(cmdSep[i] == token[0]) {
			return 1;
		}
		i++;
	}
	return 0;
}

void command_constructor(Command *c, int first, int last, char *seperator) {
	c->first = first;
	c->last = last;
	c->seperator = *seperator;
}

void find_redirection(char* tokens[], Command *cmd) {
	for (int i = cmd->first; i <= cmd->last; i++) {
		if(strcmp(tokens[i], "<") == 0) {
			if(!is_sequence_seperator(tokens[i + 1])) {
				cmd->in_file = tokens[i + 1];
				i++;
			}
		}
		else if(strcmp(tokens[i], ">") == 0) {
			if(!is_sequence_seperator(tokens[i + 1])) {
				cmd->out_file = tokens[i + 1];
				i++;
			}
		}
	}
}

void init_commands(Command c[]) {
	for(int i = 0; i < MAX_CMDS; i++) {
		c[i].first = 0;
		c[i].last = 0;
		c[i].seperator = '\0'; 
		c[i].argv = NULL; 
		c[i].in_file = NULL; 
		c[i].out_file = NULL; 
	}
}

void cmd_argument_constructor(char* tokens[], Command *cmd) {
	int n = (cmd->last - cmd->first + 1) + 1;

	cmd->argv = (char**) realloc(cmd->argv, sizeof(char*) * n);
	if(cmd->argv == NULL) {
		perror("realloc");
		exit(1);
	}

	int k = 0;
	for(int i = cmd->first; i < cmd->last; i++) {
		if(strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0) {
			i++;
		} else {
			cmd->argv[k] = tokens[i];
			k++;
		}
	}
	cmd->argv[k] = NULL;
}

int command_make(int tokenNum, char* tokens[], Command command[]) {
	if(tokenNum == 0) {
		return -1;
	}

	if(is_sequence_seperator(tokens[0])) {
		return -2;
	}

	if(!is_sequence_seperator(tokens[tokenNum - 1])) {
		tokens[tokenNum] = ";";
		tokenNum++;
	}

	int first = 0;
	int last;
	char *seperator;
	int noCmds = 0;
	for(int i = 0; i < tokenNum; i++) {
		last = i;
		if(is_sequence_seperator(tokens[i])) {
			seperator = tokens[i];

			if(first == last) {
				printf("Error: Two consecutive separators\n");
				return -3;
			}
			command_constructor(&command[noCmds], first, last, seperator);
			noCmds++;
			first = i + 1;
		}
	}

	for(int i = 0; i < noCmds; i++) {
		find_redirection(tokens, &(command[i]));
		cmd_argument_constructor(tokens, &(command[i]));
	}
	return noCmds;
}


