#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include "token.h"
#include "command.h"

// Max Prompt Size
#define MAXSIZE 128

// Max amount of commands
#define MAXCMDS 100

// All the specific command strings
#define CMD_PROMPT "prompt"
#define CMD_CD "cd"
#define CMD_PWD "pwd"
#define CMD_EXIT "exit"

// Check for wildcards 
void wildcard_check(Command *cmd);

// Handle wildcard expressions 
void wildcard_handle(Command *cmd, int index);

// Display current working directory 
void pwd_command();

// Change working directory
void cd_command(Command cmd);

// Change commandline prompter 
void prompt_command(char *prompt, Command cmd);

// Handle standard input and output redirections
void redirection(Command *cmd);

// Handle shell pipeline execution 
void pipe_command_execution(Command *cmd, int *index, int num_of_cmds);

// Handle command execution 
void command_execution(Command *cmd, int *index);

// Free allocated memory for command arguments 
void free_all(Command *commands, int num_of_cmds);

// Handle what execution to completed based off command 
void handle_command_execution(char *prompt, Command *commands, int num_of_cmds, char **command_line);

// Handle all signals 
void handler(int id);

// Set up signal handling 
void init_signal();

// Get user input from commandline 
void get_input(char **input);

// Main function
int main() {
	char *cmd = NULL;
	char prompt[MAXSIZE] = "";
	init_signal();
	do {
		printf("\n%s$ ", prompt);	
		get_input(&cmd);

		char **tokens = (char**) malloc((get_token_num(cmd)+1) * sizeof(char*));

		int i = tokenise(cmd, tokens);
	
		Command commands[MAXCMDS]; 
		init_commands(commands);
	
		int n_cmd = command_make(i, tokens, commands);

		free(tokens);

		handle_command_execution(prompt, commands, n_cmd, &cmd);

		free_all(commands, n_cmd);

	} while(1);

	exit(0);	
}

void print_command_details(const Command commands, char* tokens[]) {
	printf("Command: ");
	for (int j = commands.first; j < commands.last; j++) {
		printf("%s ", tokens[j]);
	}
	printf("\n");

	printf("Tokens:\n");

	int k = 0;
	while(commands.argv[k] != NULL) {
		printf("\t%s\n", commands.argv[k]);
		k++;
	}

	printf("Separator: %c\n", commands.seperator);

	printf("In Redirection: %s, Out Redirection %s\n", commands.in_file, commands.out_file);
}

void handle_command_execution(char *prompt, Command *commands, int num_of_cmds, char **command_line) {
	int n = 0;
	for (n = 0; n < num_of_cmds; n++) {
		// Needed for reference to command[n] when argv is changed in wildcard_handle
		Command *com = &commands[n];
		wildcard_check(com);

		if(strcmp(commands[n].argv[0], CMD_PROMPT) == 0) {
			prompt_command(prompt, commands[n]);
		}
		else if(strcmp(commands[n].argv[0], CMD_CD) == 0) {
			cd_command(commands[n]);
		}
		else if(strcmp(commands[n].argv[0], CMD_PWD) == 0) {
			pwd_command();
		}
		else if(strcmp(commands[n].argv[0], CMD_EXIT) == 0) {
			free_all(commands, num_of_cmds);
			free(*command_line);
			exit(0);
		}
		else {
			//Reference to current index in commands is needed when changed in pipe_command_execution()
			int *index = &n;
			if(commands[n].seperator == '|') {
				pipe_command_execution(commands, index, num_of_cmds);
			} 
			else {
				command_execution(commands, index);
			}	
		}
	}
}
	
void pipe_command_execution(Command *cmd, int *index, int num_of_cmds) {
	//Reference to current commands index
	int cmd_index = *index;
	pid_t pid;
	int pipe_num = 1;

	//Checks for consecutive pipes
	for(int i = cmd_index + 1; i < num_of_cmds; i++) {
		if(cmd[i].seperator == '|') {
			pipe_num++;
		}
		else {
			break;
		}
	}

	//Creates pipes
	int p[pipe_num][2];
	for(int i = 0; i < pipe_num; i++) {
		if(pipe(p[i]) < 0) {
			printf("Error: Failed to create pipe\n");
		}
	}	
		
	// pipe_num + 1 in order to execute the final command in the series of pipes
	for(int pipe_index = 0; pipe_index < pipe_num + 1; pipe_index++) {	
		// Checks if arguments are present after the separator
		if(cmd[cmd_index + pipe_index].argv != NULL) {
			pid = fork();
			if(pid == 0) {
				if(pipe_index == 0) {		
					//Only writes to pipe if first
					dup2(p[pipe_index][1], STDOUT_FILENO);
				}
				else if(pipe_index == pipe_num) {
					//Only reads from pipe if last
					dup2(p[pipe_index - 1][0], STDIN_FILENO);
				}
				else {
					//Reads from previous pipe and writes to current pipe
					dup2(p[pipe_index - 1][0], STDIN_FILENO);
					dup2(p[pipe_index][1], STDOUT_FILENO);
				}
			
				//Reference needed for redirection
				Command *com = &cmd[pipe_index + cmd_index];
				redirection(com);

				//Closes both read and write ends of all pipes
				for(int i = 0; i < pipe_num; i++) {
					close(p[i][0]);
					close(p[i][1]);
				}
				execvp(cmd[pipe_index + cmd_index].argv[0], cmd[pipe_index + cmd_index].argv);
			}
		}
		else {
			printf("Error: No command argument present following pipe operator\n");
		}
	}

	//Closes all pipes which are still active
	for(int i = 0; i < pipe_num; i++) {
		close(p[i][0]);
		close(p[i][1]);
	}

	// Checks if the pipe chain is a background process, executing in the background if true
	// else sequentially executes with other commands
	if(cmd_index > 0) {		
		if(cmd[cmd_index - 1].seperator != '&') {	
			for(int i = 0; i < pipe_num + 1; i++) {
				waitpid(pid, NULL, 0);
			}
		}
	}
	else {
		for(int i = 0; i < pipe_num + 1; i++) {
			waitpid(pid, NULL, 0);
		}
	}
		
	//Changes the current index for the commands[] to position after the final pipe output
	//For example so the command 'more' instead next executed standalone when the current command
	//is 'ls *.c | more'
	*index += pipe_num;
}

void command_execution(Command *cmd, int *index) {
	int n = *index;
	pid_t pid = fork();
	if(pid == 0) {
		Command *com = &cmd[n];
		redirection(com);
		execvp(cmd[n].argv[0], cmd[n].argv);
		exit(0);
	}
	else {
		if(cmd[n].seperator != '&') {
			waitpid(pid, NULL, WUNTRACED | WCONTINUED);
		}
	}
}

void redirection(Command *cmd) {
	int fdout;
	int fdin;

	if(cmd->out_file != NULL) {
		fdout = open(cmd->out_file, O_CREAT | O_TRUNC | O_WRONLY, 0664);
		if(fdout == -1) {
			perror("Error");
			exit(0);
		}

		dup2(fdout, 1);
		close(fdout);
	}

	if(cmd->in_file != NULL) {
		fdin = open(cmd->in_file, O_RDONLY, 0664);
		if(fdin == -1) {
			perror("Error");
			exit(0);
		}
	
		dup2(fdin, STDIN_FILENO);
		close(fdin);
	}
}

void prompt_command(char *prompt, Command cmd) {
	if(cmd.argv[1] != NULL) {
		if(strlen(cmd.argv[1]) < MAXSIZE) {
			strcpy(prompt, cmd.argv[1]);
		}
		else {
			printf("Error: Limit Exceeded - first argument exceeded set length\n");
		}
	}
	else {
		printf("Error: Expected Argument - Missing first argument\n");
	}

}

void cd_command(Command cmd) {
	if(cmd.argv[1] != NULL) {
		if(chdir(cmd.argv[1]) < 0) {
			perror("Error");
		}
	}
	else {
		chdir("/");
	}
}

void handler(int id) {
	if(id == 17) {
		int more_zombies = 1;
		pid_t pid;
		int status;

		while (more_zombies) {
			pid = waitpid(-1, &status, WNOHANG);
			if(pid <= 0) {
				more_zombies = 0;
			}
		}
	}
}

void init_signal() {
	struct sigaction act;

	act.sa_flags = 0;
	act.sa_handler = handler;

	sigaction(SIGCHLD, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
}

void pwd_command() {
	int max_length = 256;
	char current_directory[max_length];

	if(getcwd(current_directory, sizeof(current_directory)) != NULL) {
		printf("%s\n", current_directory);
	}
	else {
		printf("Error: Path name exceeds set length\n");
	}
}

void wildcard_handle(Command *cmd, int index) {
	glob_t glob_buf;
	int glob_return;

	char **extraTokens = NULL;
	int extraTokenCount = 0;
	
	glob_return = glob(cmd->argv[index], GLOB_ERR, NULL, &glob_buf);
		
	extraTokens = (char **) realloc(extraTokens, sizeof(char *) * (MAX_CMDS));

	int j = index + 1;
	while(cmd->argv[j] != NULL) {
		extraTokens[extraTokenCount] = cmd->argv[j];
		extraTokenCount++;
		j++;
	}	

	if(glob_return == 0) {
		cmd->argv = (char **) realloc(cmd->argv, sizeof(char *) * (glob_buf.gl_pathc + index + extraTokenCount + 1));

		int i = 0;
		for(i = 0; i < glob_buf.gl_pathc; i++) {
			cmd->argv[i + index] = glob_buf.gl_pathv[i];
		}

		for(int k = 0; k < extraTokenCount; k++) {
			cmd->argv[i + index] = extraTokens[k];
			i++;
		}
		cmd->argv[i + index] = NULL;
			
		if(extraTokenCount > 0) {
			wildcard_check(cmd);
		}
	}
	else {
		printf("Cannot access '%s': No such file or directory\n", cmd->argv[index]);

		if(extraTokenCount > 0) {
			int i = 0;
			for(i = 0; i < extraTokenCount; i++) {
				cmd->argv[i + index] = extraTokens[i];
			}
			cmd->argv[i + index] = NULL;
			wildcard_check(cmd);
		}
	}
}		

void wildcard_check(Command *cmd) {
	int m = 0;
	while(cmd->argv[m] != NULL) {
		if(strchr(cmd->argv[m], '*') != NULL || strchr(cmd->argv[m], '?') != NULL) {
			wildcard_handle(cmd, m);
			break;
		}
		m++;
	}
}

void get_input(char **input) {
	int repeat = 1;
	int line_point;
	size_t len = 0;

	while(repeat) {
		repeat = 0;

		line_point = getline(&*input, &len, stdin);
		if(line_point == -1) {
			if(errno = EINTR) {
				clearerr(stdin);
				repeat = 1;
			}
		}
	}
}

void free_all(Command *commands, int num_of_cmds) {
	for(int n = 0; n < num_of_cmds; n++) {
		free(commands[n].argv);
	}
}
