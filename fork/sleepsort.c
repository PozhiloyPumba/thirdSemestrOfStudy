#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    for (int i = 1; i < argc; ++i) {
        pid = fork();
        if (pid == 0) {
            int a = atoi(argv[i]);

            usleep(a * 10000);
            printf("%d ", a);

            exit(0);
        }
    }
    for (int i = 1; i < argc; ++i) {
        wait(NULL);
    }

    printf("\n");
    return 0;
}