#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    printf("pid is: %d\nparent pid is: %d\n", getpid(), getppid());

    return 0;
}