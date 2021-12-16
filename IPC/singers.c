#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//=====================================================================================================

#define ERRORCHECKER(condition, todo, error) \
    do {                                     \
        if (condition) {                     \
            todo return error;               \
        }                                    \
    } while (0)

const int POISON = -1;
const int NUMSINGERS = 33;

//=====================================================================================================

struct dummy_msg_struct {
    long mtype;
    char mtext[1];
};

//=====================================================================================================

char *generateSong (int argc, char **argv);

char *getAlfabet (char *song);

void fillAlfQueue (int queue_id, char *alf);

int singer (char *song);

//=====================================================================================================

int main (int argc, char *argv[])
{
    ERRORCHECKER (
        argc < 2, {printf ("Please input correct song))\n"); return 0; }, 0);

    char *song = generateSong (argc, argv);

    char *alfabet = getAlfabet (song);

    key_t key;
    ERRORCHECKER ((key = ftok ("./singers.c", 'a')) == (key_t)(-1), { perror ("IPC error: ftok"); }, errno);

    int alfabetMsg = msgget (key, IPC_CREAT | 0666);
    ERRORCHECKER (
        (alfabetMsg == -1), { perror ("queue creation error"); }, errno);

    fillAlfQueue (alfabetMsg, alfabet);
    free (alfabet);

    ERRORCHECKER ((key = ftok ("./singers.c", 'b')) == (key_t)(-1), { perror ("IPC error: ftok"); }, errno);
    int sem_id = semget (key, 1, IPC_CREAT | IPC_EXCL | 0660);

    semctl (sem_id, 0, SETVAL, 0);

    pid_t pid = 0;
    for (int i = 0; i < NUMSINGERS; ++i) {
        pid = fork ();
        if (!pid) {
            singer (song);
            free (song);
            return 0;
        }
    }

    for (int i = 0; i < NUMSINGERS; ++i)
        wait (NULL);

    free (song);
    msgctl (alfabetMsg, IPC_RMID, NULL);
    semctl (sem_id, IPC_RMID, 0);
    return 0;
}

//=====================================================================================================

char *generateSong (int argc, char **argv)
{
    int sumLen = 0;
    for (int i = 1; i < argc; ++i)
        sumLen += strlen (argv[i]) + 1;

    char *song = (char *)malloc (sizeof (char) * (sumLen + 1));
    song[0] = 0;

    for (int i = 1; i < argc; ++i) {
        strcat (song, argv[i]);
        strcat (song, " ");
    }

    song[sumLen - 1] = 0;

    return song;
}

//-----------------------------------------------------------------------------------------------------

char *getAlfabet (char *song)
{
    char allSym[128] = {};
    int len = strlen (song);
    int alfSize = 0;

    for (int i = 0; i < len; ++i) {
        if (!allSym[song[i]]) {
            allSym[song[i]] = 1;
            ++alfSize;
        }
    }

    char *alfabet = (char *)malloc (sizeof (char) * (alfSize + 1));

    int place = 0;
    for (int i = 0; i < alfSize; ++i) {
        while (place < 128 && !allSym[place++])
            ;

        alfabet[i] = place - 1;
    }
    alfabet[alfSize] = 0;

    return alfabet;
}

//-----------------------------------------------------------------------------------------------------

void fillAlfQueue (int queue_id, char *alf)
{
    int len = strlen (alf);
    struct dummy_msg_struct forAlf = {1, 0};

    for (int i = 0; i < NUMSINGERS; ++i) {
        if (i >= len)
            forAlf.mtext[0] = POISON;
        else
            forAlf.mtext[0] = alf[i];

        msgsnd (queue_id, &forAlf, 1, 0);
    }
}

//-----------------------------------------------------------------------------------------------------

int singer (char *song)
{
    key_t key;
    ERRORCHECKER ((key = ftok ("./singers.c", 'a')) == (key_t)(-1), { perror ("IPC error: ftok"); }, errno);
    int alfabetMsg = msgget (key, 0666);

    ERRORCHECKER ((key = ftok ("./singers.c", 'b')) == (key_t)(-1), { perror ("IPC error: ftok"); }, errno);
    int sem_id = semget (key, 1, 0660);

    struct dummy_msg_struct forRcv;
    msgrcv (alfabetMsg, &forRcv, 1, 1, 0);

    if (forRcv.mtext[0] == -1) {
        // printf("I'm in sad [%d]((( I didn't have enough letter. Now I go to the sunset....\n", getpid());
        return 0;
    }
    // printf("I'm very glad [%d]! My letter is [%c]. Go sing this song\n", getpid(), forRcv.mtext[0]);

    char mySym = forRcv.mtext[0];
    int len = strlen (song);
    int countMySym = 0;

    for (int i = 0; i < len; ++i)
        if (song[i] == mySym)
            ++countMySym;

    struct sembuf next[] = {{0, 1, 0}};

    for (int i = 0; i < countMySym; ++i) {
        while (song[semctl (sem_id, 0, GETVAL)] != mySym)
            ;

        putchar (mySym);
        printf (" [%d]\n", getpid ());
        fflush (NULL);

        semop (sem_id, next, 1);
    }

    return 0;
}
