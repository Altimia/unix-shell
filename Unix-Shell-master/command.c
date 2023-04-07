#include "command.h"

int isSep(char *token)
{
	int i = 0;
	char cseps[4] = {pSep, sSep, aSep, '\0'};

	while (cseps[i] != '\0')
	{
		if(cseps[i] == token[0])
		{
			return 1;
		}
		i++;
	}

	return 0;
}

void command_constructor(Command *c, int first, int last, char *sep)
{
	c->first = first;
	c->last = last;
	c->sep = *sep;
}

void search_redirection(char* tokens[], Command *cmd)
{
	for (int i = cmd->first; i <= cmd->last; i++)	
	{
		if(strcmp(tokens[i], "<") == 0)
		{
			if(!isSep(tokens[i + 1]))
			{
				cmd->in_file = tokens[i + 1];
				i++;
			}

		}
		else if(strcmp(tokens[i], ">") == 0)
		{
			if(!isSep(tokens[i + 1]))
			{
				cmd->out_file = tokens[i + 1];
				i++;
			}
		}
	}
}

void init_commands(Command c[])
{
	for(int i = 0; i < MAX_CMDS; i++)
	{
		c[i].first = 0;
		c[i].last = 0;
		c[i].sep = '\0'; 
		c[i].argv = NULL; 
		c[i].in_file = NULL; 
		c[i].out_file = NULL; 
	}
}

void command_args_make(char* tokens[], Command *cmd)
{
	int n = (cmd->last - cmd->first + 1) + 1;

	cmd->argv = (char**) realloc(cmd->argv, sizeof(char*) * n);
	if(cmd->argv == NULL)
	{
		perror("realloc");
		exit(1);
	}

	int k = 0;
	for(int i = cmd->first; i < cmd->last; i++)
	{
		if(strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0)
		{
			i++;
		}
		else
		{
			cmd->argv[k] = tokens[i];
			k++;
		}
	}
	cmd->argv[k] = NULL;
}

int command_make(int tok_num, char* tokens[], Command command[])
{
	if(tok_num == 0)
	{
		return -1;
	}

	if(isSep(tokens[0]))
	{
		return -2;
	}

	if(!isSep(tokens[tok_num - 1]))
	{
		tokens[tok_num] = ";";
		tok_num++;
	}

	int first = 0;
	int last;
	char *sep;
	int n_cmds = 0;
	for(int i = 0; i < tok_num; i++)
	{
		last = i;
		if(isSep(tokens[i]))
		{
			sep = tokens[i];

			if(first == last)
			{
				printf("Error: Two consecutive separators\n");
				return -3;
			}

			command_constructor(&command[n_cmds], first, last, sep);
			n_cmds++;
			first = i + 1;
		}
	}

	for(int i = 0; i < n_cmds; i++)
	{
		search_redirection(tokens, &(command[i]));
		command_args_make(tokens, &(command[i]));
	}

	return n_cmds;
}


