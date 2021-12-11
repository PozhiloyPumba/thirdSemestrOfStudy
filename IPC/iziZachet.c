#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//=====================================================================================================

void slowPrint (const char *str)
{
    int len = strlen (str);

    for (int i = 0; i < len; ++i) {
        printf ("%c", str[i]);
        sleep (1);
        fflush (NULL);
    }
    printf ("\n");
}

//=====================================================================================================

int main (int argc, char *argv[])
{
    int sem_id;
    struct sembuf begin[] = {{0, 1, 0}};
    struct sembuf end[] = {{0, -1, 0}};
    key_t key;
    if ((key = ftok ("./iziZachet.c", 'a')) == (key_t)(-1)) {
        perror ("IPC error: ftok");
        exit (1);
    }

    errno = 0;
    if (((sem_id = semget (key, 1, IPC_CREAT | IPC_EXCL | 0660)) == -1) && (errno == EEXIST)) {
        sem_id = semget (key, 1, 0660);
        semop (sem_id, begin, 1);
        
        slowPrint ("Goodbye!");
        
        semop (sem_id, end, 1);

        if (!semctl (sem_id, 0, GETVAL, 0))
            semctl (sem_id, IPC_RMID, 0);
    }
    else {
        semctl (sem_id, 0, SETVAL, 0);

        semop (sem_id, begin, 1);
        
        slowPrint ("Hello world!");
        
        semop (sem_id, end, 1);

        if (!semctl (sem_id, 0, GETVAL, 0))
            semctl (sem_id, IPC_RMID, 0);
    }

    return 0;
}