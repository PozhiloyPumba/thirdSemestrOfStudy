#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE_BUFFER 1024

//=====================================================================================================

int copy_in_stdout(int fd1);

//=====================================================================================================

int main(int argc, char *argv[]) {
    if (argc == 1) {
        copy_in_stdout(0);
    } else {
        for (int i = 1; i < argc; i++) {
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0) {
                perror("Error open");
                return errno;
            }

            if (copy_in_stdout(fd) < 0) {
                perror("Error copy");
                return errno;
            }
            if (close(fd) < 0) {
                perror("Error close");
                return errno;
            }
        }
    }
    return 0;
}

//=====================================================================================================

int copy_in_stdout(int fd1) {
    char str[SIZE_BUFFER];

    while (1) {
        int end = read(fd1, str, SIZE_BUFFER);
        if (end < 0) {
            perror("Error read");
            return errno;
        }
        if (!end)
            break;

        int position = 0;
        while (position < end) {
            int wr_count = write(1, str + position, end - position);
            if (wr_count < 0)
                perror("Error write");

            position += wr_count;
        }
    }
    return 0;
}