#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

//=====================================================================================================

#define INWORD 1
#define OUTWORD 0
#define LIM_OF_MY_SHELL 1000
#define CLOSE(fd1)              \
    if (close (fd1) < 0) {      \
        perror ("Error close"); \
        return errno;           \
    }

//=====================================================================================================

int main ()
{
    char cmd_string[LIM_OF_MY_SHELL];  // it is string from user
    char *commands[LIM_OF_MY_SHELL];   // it is array of pointers to commands
    char *cmd[LIM_OF_MY_SHELL];        // it is array of pointers to args of command

    while (!feof (stdin)) {
        printf ("my_shell > ");

        fgets (cmd_string, LIM_OF_MY_SHELL, stdin);

        cmd_string[strlen (cmd_string) - 1] = 0;

        if ((cmd_string == NULL) || !strcmp (cmd_string, "exit"))
            break;

        int commands_count = 0;
        commands[commands_count] = strtok (cmd_string, "|");
        while (commands[++commands_count] = strtok (NULL, "|"))
            ;  //(not ==) spliting into commands

        int pass_pipe = -1;
        int pid;
        int pipefd[2];
        for (int i = 0; i < commands_count; ++i) {
            if (pipe (pipefd) == -1) {
                printf ("Pipe error\n");
                break;
            }

            pid = fork ();
            if (pid == -1) {
                printf ("Fork error\n");
                break;
            }

            if (pid) {
                CLOSE (pipefd[1]);
                if (pass_pipe != -1)
                    CLOSE (pass_pipe);
                pass_pipe = pipefd[0];
            }
            else {
                if (pass_pipe != -1)
                    if (dup2 (pass_pipe, 0) == -1) {
                        perror ("copy stdin error");
                        return -1;
                    }

                if (i < (commands_count - 1))
                    if (dup2 (pipefd[1], 1) == -1) {
                        perror ("copy stdout error");
                        return -1;
                    }

                CLOSE (pipefd[0]);
                CLOSE (pipefd[1]);
                if (pass_pipe != -1)
                    CLOSE (pass_pipe);

                int num_of_args = 0;
                cmd[num_of_args] = strtok (commands[i], " ");
                while (cmd[++num_of_args] = strtok (NULL, " "))
                    ;  //(not ==) spliting into args in command

                execvp (cmd[0], cmd);
                perror (cmd[0]);
                return -1;
            }
        }

        CLOSE (pass_pipe);

        for (int i = 0; i < commands_count; i++)
            wait (NULL);
    }

    return 0;
}