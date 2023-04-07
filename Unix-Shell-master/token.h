
//token delimiters
#define TOKEN_DELIMITER " \t\n" 

/*
 * Desc: Creates an array of string tokens from string input 
 * Params
 * 	string input_line - string input to be tokenised	
 * 	character token - array of tokens to be constructed
 * Return: int - number of tokens 
 * Notes: NA
*/
int tokenise(char *input_line, char *token[]);

/*
 * Desc: Gets the number of tokens that would be created given a string 
 * Params
 *      string input_line - string input to be tokenised
 * Return: int - number of tokens
 * Notes: NA
*/
int get_token_num(char *input_line);
