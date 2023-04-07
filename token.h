#define TOKEN_DELIMITER " \t\n"

// Create array of string tokens from string input 
int tokenise(char *input_line, char *token[]);

// Get number of tokens needed given a string 
int get_token_num(char *input_line);
