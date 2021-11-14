#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

//=====================================================================================================

#define SIZE_BUFFER 4096
#define INWORD 1
#define OUTWORD 0
#define LIM_OF_MY_SHELL 1000

//=====================================================================================================

int main()
{
	while(1)
	{
		printf("my_shell > ");

		char *command = readline(NULL);
		if(command == NULL || !strcmp(command, "exit")){
			if(command != NULL)
				free(command);
			return 0;
		}

		char count_args[LIM_OF_MY_SHELL];
		size_t count_commands = 0;
		int flag_word = OUTWORD, count_words = 0;

		for(size_t i = 0; command[i] != 0 || count_args < LIM_OF_MY_SHELL; ++i)
		{
			
		}

		// int pipe_fd[2];
		// if(pipe(pipe_fd)){
		// 	perror("Error of pipe");
		// 	return errno;
		// }
		
		// pid_t pid = fork();
		// if (pid == -1){
		// 	perror("Error of fork");
		// 	return errno;
		// }

		// if(pid == 0){
		// 	int error = dup2(pipe_fd[1], STDOUT_FILENO);
		// 	if(error == -1){
		// 		perror("Error of dup");
		// 		return errno;
		// 	}
		// 	if(close(pipe_fd[0])){
		// 		perror("Error of close");
		// 		return errno;
		// 	}
		// 	if(close(pipe_fd[1])){
		// 		perror("Error of close");
		// 		return errno;
		// 	}
		// }
		// else
		// 	close(pipe_fd[1]);


		// if(pid == 0){
		// 	int shift = 0;

		// 	int error = execvp(argv[1 + shift], argv + 1);
		// 	if(error){
		// 		perror("Error of exec");
		// 		return errno;
		// 	}
		// 	return 0;
		// }
		
		// char str[SIZE_BUFFER];
		
		// while(1)
		// {
		// 	int end = read(pipe_fd[0], str, SIZE_BUFFER);
		// 	if(end < 0) {
		// 		perror("Error read");
		// 		return errno;
		// 	}
		// 	if(!end)
		// 		break;

		// }
		// close(pipe_fd[0]);

		// wait(NULL);
	}

	return 0;
}