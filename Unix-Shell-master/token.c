#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"

int tokenise(char *input_line, char *token[])
{
	char *toks;
	int i = 0;

	toks = strtok(input_line, TOKEN_DELIMITER);
	token[i] = toks;

	while(toks != NULL)
	{
		i++;

		toks = strtok(NULL, TOKEN_DELIMITER);
		token[i] = toks;
	}

	return i;
}

int get_token_num(char *input_line)
{
	char *toks;
	int i = 0;

	char *str= (char*) malloc(strlen(input_line) * sizeof(char));
	strcpy(str, input_line);

	toks = strtok(str, TOKEN_DELIMITER);

	while(toks != NULL)
	{
		i++;
		toks = strtok(NULL, TOKEN_DELIMITER);
	}

	free(str);

	return i;
}

