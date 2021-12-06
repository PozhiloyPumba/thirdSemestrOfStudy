#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERRORCHECKER(condition, todo, error) \
    do {                                     \
        if (condition) {                     \
            todo return error;               \
        }                                    \
    } while (0)

//=====================================================================================================

void init_queues (int forSendDet, int winter, int gayker);
int winterFunc (int forSendDet, int winter, int gayker, int workPlan);
int gaykerFunc (int forSendDet, int winter, int gayker, int workPlan);

//=====================================================================================================

struct dummy_msg_struct {
    long mtype;
    char mtext[1];
};

//=====================================================================================================

int main (int argc, char *argv[])
{
    ERRORCHECKER (
        argc != 2, { printf ("incorrect args\n"); }, 0);

    int workPlan = msgget (IPC_PRIVATE, IPC_CREAT | 0660);
    ERRORCHECKER (
        workPlan == -1, { perror ("queue creation error"); }, errno);
    struct dummy_msg_struct msgInit = {1, {atoi (argv[1])}};
    msgsnd (workPlan, &msgInit, 1, 0);

    int forSendDet = msgget (IPC_PRIVATE, IPC_CREAT | 0660);
    ERRORCHECKER (
        forSendDet == -1, { perror ("queue creation error"); }, errno);
    int winter = msgget (IPC_PRIVATE, IPC_CREAT | 0660);
    ERRORCHECKER (
        winter == -1, { perror ("queue creation error"); }, errno);
    int gayker = msgget (IPC_PRIVATE, IPC_CREAT | 0660);
    ERRORCHECKER (
        gayker == -1, { perror ("queue creation error"); }, errno);

    init_queues (forSendDet, winter, gayker);

    int pid = fork ();
    if (!pid) {
        gaykerFunc (forSendDet, winter, gayker, workPlan);
        return 0;
    }
    pid = fork ();
    if (!pid) {
        gaykerFunc (forSendDet, winter, gayker, workPlan);
        return 0;
    }
    pid = fork ();
    if (!pid) {
        winterFunc (forSendDet, winter, gayker, workPlan);
        return 0;
    }

    for (int i = 0; i < 3; ++i)
        wait (NULL);

    msgctl (forSendDet, IPC_RMID, NULL);
    msgctl (winter, IPC_RMID, NULL);
    msgctl (gayker, IPC_RMID, NULL);
    msgctl (workPlan, IPC_RMID, NULL);
    printf ("work done\n");

    return 0;
}

//=====================================================================================================

void init_queues (int forSendDet, int winter, int gayker)
{
    struct dummy_msg_struct msgInit = {1, {0}};
    msgsnd (forSendDet, &msgInit, 1, 0);

    struct dummy_msg_struct msgInitWorker = {1, {1}};
    msgsnd (gayker, &msgInitWorker, 1, 0);
    msgsnd (gayker, &msgInitWorker, 1, 0);
    msgsnd (winter, &msgInitWorker, 1, 0);
}

//-----------------------------------------------------------------------------------------------------

int winterFunc (int forSendDet, int winter, int gayker, int workPlan)
{
    while (1) {
        int reminder = 0;
        struct dummy_msg_struct msg;

        msgrcv (winter, &msg, 1, 1, 0);

        // check plan

        msgrcv (workPlan, &msg, 1, 1, 0);
        reminder = msg.mtext[0];
        msgsnd (workPlan, &msg, 1, 0);
        if (!reminder)
            return 0;

        // end checking

        printf ("weweWinter [%d] in work\n", getpid ());

        msgrcv (forSendDet, &msg, 1, 1, 0);
        reminder = msg.mtext[0];

        if (reminder == 2) {
            msgrcv (workPlan, &msg, 1, 1, 0);
            --msg.mtext[0];
            msgsnd (workPlan, &msg, 1, 0);

            printf ("weweWinter [%d] pick a detail\n", getpid ());
            printf ("details remained = %d\n", msg.mtext[0]);
            printf ("weweWinter [%d] has done\n\n", getpid ());

            init_queues (forSendDet, winter, gayker);
        }
        else {
            struct dummy_msg_struct msgForSend = {1, {reminder + 1}};
            msgsnd (forSendDet, &msgForSend, 1, 0);
            printf ("weweWinter [%d] has done\n", getpid ());
        }
    }
}

//-----------------------------------------------------------------------------------------------------

int gaykerFunc (int forSendDet, int winter, int gayker, int workPlan)
{
    while (1) {
        int reminder = 0;
        struct dummy_msg_struct msg;

        msgrcv (gayker, &msg, 1, 1, 0);

        // check plan

        msgrcv (workPlan, &msg, 1, 1, 0);
        reminder = msg.mtext[0];
        msgsnd (workPlan, &msg, 1, 0);
        if (!reminder)
            return 0;

        // end checking

        printf ("gayker [%d] in work\n", getpid ());

        msgrcv (forSendDet, &msg, 1, 1, 0);
        reminder = msg.mtext[0];

        if (reminder == 2) {
            msgrcv (workPlan, &msg, 1, 1, 0);
            reminder = --msg.mtext[0];
            msgsnd (workPlan, &msg, 1, 0);

            printf ("gayker [%d] pick a detail\n", getpid ());
            printf ("details remained = %d\n", msg.mtext[0]);
            printf ("gayker [%d] has done\n\n", getpid ());

            init_queues (forSendDet, winter, gayker);
        }
        else {
            struct dummy_msg_struct msgForSend = {1, {reminder + 1}};
            msgsnd (forSendDet, &msgForSend, 1, 0);
            printf ("gayker [%d] has done\n", getpid ());
        }
    }
}
