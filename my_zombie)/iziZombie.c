#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main ()
{
    if (fork ())
        sleep (2000);
    else
        exit (0);
    return 0;
}