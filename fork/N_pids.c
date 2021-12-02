#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Wrong count of params!!!\n");
        return 0;
    }
    int n = atoi(argv[1]);

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid != 0) {
            wait(NULL);
            exit(0);
        }
        if (pid == 0)
            printf("parent_pid = %d, pid = %d\n", getppid(), getpid());
    }
    return 0;
}