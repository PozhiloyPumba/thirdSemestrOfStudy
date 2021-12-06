#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

//=====================================================================================================

#define SIZE_BUFFER 4096
#define SPACE_COND 1
#define WORD_COND 0

//=====================================================================================================

double transform_time (struct timeval *tv1, struct timeval *tv2)
{
    return ((double)tv2->tv_sec + (double)tv2->tv_usec / 1000000) - ((double)tv1->tv_sec + (double)tv1->tv_usec / 1000000);
}

//=====================================================================================================

int main (int argc, char *argv[])
{
    if (argc < 2) {
        printf ("Enter command!\n");
        return 0;
    }

    int quite = getopt (argc, argv, "+q");
    if (quite != 'q' && quite != -1)
        return 0;

    int pipe_fd[2];
    if (pipe (pipe_fd)) {
        perror ("Error of pipe");
        return errno;
    }

    pid_t pid = fork ();
    if (pid == -1) {
        perror ("Error of fork");
        return errno;
    }

    if (pid == 0) {
        int error = dup2 (pipe_fd[1], STDOUT_FILENO);
        if (error == -1) {
            perror ("Error of dup");
            return errno;
        }
        if (close (pipe_fd[0])) {
            perror ("Error of close");
            return errno;
        }
        if (close (pipe_fd[1])) {
            perror ("Error of close");
            return errno;
        }
    }
    else
        close (pipe_fd[1]);

    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

    int error = gettimeofday (&tv1, &tz1);
    if (error) {
        perror ("Error of getting time");
        return errno;
    }

    if (pid == 0) {
        int shift = 0;
        if (quite != -1)
            shift = 1;

        int error = execvp (argv[1 + shift], argv + 1 + shift);
        if (error) {
            perror ("Error of exec");
            return errno;
        }
        return 0;
    }

    char str[SIZE_BUFFER];
    int count_bytes = 0, count_words = 0, count_lines = 0;
    int condition = SPACE_COND;

    while (1) {
        int end = read (pipe_fd[0], str, SIZE_BUFFER);
        if (end < 0) {
            perror ("Error read");
            return errno;
        }
        if (!end)
            break;

        count_bytes += end;
        for (int i = 0; i < end; ++i) {
            if (str[i] == '\n')
                count_lines++;

            if (isspace (str[i])) {
                condition = SPACE_COND;
            }
            else {
                if (condition == SPACE_COND) {
                    count_words++;
                    condition = WORD_COND;
                }
            }
        }
        if (quite != 'q') {
            int position = 0;
            while (position < end) {
                int wr_count = write (1, str + position, end - position);
                if (wr_count < 0) {
                    perror ("Error write");
                    return errno;
                }
                position += wr_count;
            }
        }
    }
    close (pipe_fd[0]);

    wait (NULL);

    error = gettimeofday (&tv2, &tz2);
    if (error) {
        perror ("Error of getting time");
        return errno;
    }

    printf ("\t%d\t%d\t%d\n", count_lines, count_words, count_bytes);
    printf ("time = %.3lfs\n", transform_time (&tv1, &tv2));
    return 0;
}