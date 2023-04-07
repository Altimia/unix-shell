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
void check_wildcard(Command *cmd);

// Handle wildcard expressions 
void handle_wildcard(Command *cmd, int index);

// Display current working directory 
void pwd_command();

// Change working directory
void cd_command(Command cmd);

// Change commandline prompter 
void prompt_command(char *prompt, Command cmd);

// Handle standard input and output redirections
void redirection(Command *cmd);

// Execute commands with pipe
void pipe_command_execution(Command *cmd, int *index, int num_of_cmds);

// Execute commands
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

// Zombie process claimer
void wait_for_zombies();

// Main function
int main() {
	char *cmd = NULL;
	char prompt[MAXSIZE] = "";
	init_signal();
	do {
		wait_for_zombies();
		printf("$ ", prompt);	
		get_input(&cmd);

		char **tokens = (char**) malloc((token_number(cmd)+1) * sizeof(char*));

		int i = tokenise(cmd, tokens);
	
		Command commands[MAXCMDS]; 
		init_commands(commands);
	
		int n_cmd = parse_command(i, tokens, commands);

		free(tokens);

		handle_command_execution(prompt, commands, n_cmd, &cmd);

		free_all(commands, n_cmd);

	} while(1);

	exit(0);	
}

void handle_command_execution(char *prompt, Command *commands, int num_of_cmds, char **command_line) {
	int n = 0;
	for (n = 0; n < num_of_cmds; n++) {
		Command *com = &commands[n];
		check_wildcard(com);

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

void pipe_command_execution(Command *cmd, int *index, int num_of_cmds) {
	int cmd_index = *index;
	pid_t pid;
	int pipe_num = 1;

	for(int i = cmd_index + 1; i < num_of_cmds; i++) {
		if(cmd[i].seperator == '|') {
			pipe_num++;
		}
		else {
			break;
		}
	}

	int p[pipe_num][2];
	for(int i = 0; i < pipe_num; i++) {
		if(pipe(p[i]) < 0) {
			printf("Error: Failed to create pipe\n");
		}
	}	
		
	for(int pipe_index = 0; pipe_index < pipe_num + 1; pipe_index++) {	
		if(cmd[cmd_index + pipe_index].argv != NULL) {
			pid = fork();
			if(pid == 0) {
				if(pipe_index == 0) {		
					dup2(p[pipe_index][1], STDOUT_FILENO);
				}
				else if(pipe_index == pipe_num) {
					dup2(p[pipe_index - 1][0], STDIN_FILENO);
				}
				else {
					dup2(p[pipe_index - 1][0], STDIN_FILENO);
					dup2(p[pipe_index][1], STDOUT_FILENO);
				}
			
				Command *com = &cmd[pipe_index + cmd_index];
				redirection(com);

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

	for(int i = 0; i < pipe_num; i++) {
		close(p[i][0]);
		close(p[i][1]);
	}

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
	*index += pipe_num;
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

void check_wildcard(Command *cmd) {
	int m = 0;
	while(cmd->argv[m] != NULL) {
		if(strchr(cmd->argv[m], '*') != NULL || strchr(cmd->argv[m], '?') != NULL) {
			handle_wildcard(cmd, m);
			break;
		}
		m++;
	}
}

void handle_wildcard(Command *cmd, int index) {
	glob_t glob_buf;
	int glob_return;

	char **extraTokens = NULL;
	int tokenCounter = 0;
	
	glob_return = glob(cmd->argv[index], GLOB_ERR, NULL, &glob_buf);
		
	extraTokens = (char **) realloc(extraTokens, sizeof(char *) * (MAX_CMDS));

	int j = index + 1;
	while(cmd->argv[j] != NULL) {
		extraTokens[tokenCounter] = cmd->argv[j];
		tokenCounter++;
		j++;
	}	

	if(glob_return == 0) {
		cmd->argv = (char **) realloc(cmd->argv, sizeof(char *) * (glob_buf.gl_pathc + index + tokenCounter + 1));

		int i = 0;
		for(i = 0; i < glob_buf.gl_pathc; i++) {
			cmd->argv[i + index] = glob_buf.gl_pathv[i];
		}

		for(int k = 0; k < tokenCounter; k++) {
			cmd->argv[i + index] = extraTokens[k];
			i++;
		}
		cmd->argv[i + index] = NULL;
			
		if(tokenCounter > 0) {
			check_wildcard(cmd);
		}
	}
	else {
		printf("Cannot access '%s': No such file or directory\n", cmd->argv[index]);

		if(tokenCounter > 0) {
			int i = 0;
			for(i = 0; i < tokenCounter; i++) {
				cmd->argv[i + index] = extraTokens[i];
			}
			cmd->argv[i + index] = NULL;
			check_wildcard(cmd);
		}
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

void wait_for_zombies() {
    int more = 1;        // more zombies to claim
    pid_t pid;           // pid of the zombie
    int status;          // termination status of the zombie

    while (more) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) 
            more = 0;
    }
}