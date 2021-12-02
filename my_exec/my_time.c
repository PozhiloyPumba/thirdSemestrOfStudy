#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

double transform_time(struct timeval *tv1, struct timeval *tv2) {
    return ((double)tv2->tv_sec + (double)tv2->tv_usec / 1000000) - ((double)tv1->tv_sec + (double)tv1->tv_usec / 1000000);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Enter command!\n");
        return 0;
    }

    pid_t pid = fork();

    if (pid == 0) {
        int error = execvp(argv[1], argv + 1);
        if (error) {
            perror("Error of exec:");
            return errno;
        }
        return 0;
    }

    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

    int error = gettimeofday(&tv1, &tz1);
    if (error) {
        perror("Error of getting time:");
        return errno;
    }

    wait(NULL);

    error = gettimeofday(&tv2, &tz2);
    if (error) {
        perror("Error of getting time:");
        return errno;
    }

    printf("time = %.3lfs\n", transform_time(&tv1, &tv2));
    return 0;
}