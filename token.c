#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"

int tokenise(char *input_line, char *token[]) {
	char *token_s;
	int i = 0;

	token_s = strtok(input_line, TOKEN_DELIMITER);
	token[i] = token_s;

	while(token_s != NULL) {
		i++;
		token_s = strtok(NULL, TOKEN_DELIMITER);
		token[i] = token_s;
	}
	return i;
}

int get_token_num(char *input_line) {
	char *token_s;
	int i = 0;

	char *str= (char*) malloc(strlen(input_line) * sizeof(char));
	strcpy(str, input_line);

	token_s = strtok(str, TOKEN_DELIMITER);

	while(token_s != NULL) {
		i++;
		token_s = strtok(NULL, TOKEN_DELIMITER);
	}
	free(str);

	return i;
}

