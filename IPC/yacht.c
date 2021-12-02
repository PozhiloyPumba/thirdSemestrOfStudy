#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//=====================================================================================================

#define NPASS attractionParams[0]  // num of passengers
#define NRIDE attractionParams[1]  // limit of rides
#define NSEAT attractionParams[2]  // num of seats

//=====================================================================================================

int captainMode(int sem_id, int countRides, int countSeats);  // ))))

//=====================================================================================================

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Please, input correct num of arguments");
        return 0;
    }

    int attractionParams[3];
    for (int i = 0; i < 3; ++i) {
        attractionParams[i] = atoi(argv[i + 1]);
        if (attractionParams[i] <= 0) {
            printf("You are gave me uncorrect arg number %d\n Please repeat run!\n", attractionParams[i]);
            return 0;
        }
    }

    if (NPASS < NSEAT)
        NSEAT = NPASS;

    int sem_id = semget(IPC_PRIVATE, 4, 0660);  // place on yacht; ban of debarcation; trap; ban of enter
    unsigned short initSemsValue[4] = {0, 0, 0, 0};
    semctl(sem_id, 0, SETALL, initSemsValue);

    struct sembuf landing[] = {{3, 0, 0}, {2, -1, 0}, {0, -1, 0}, {1, 1, 0}};
    struct sembuf debarkation[] = {{1, 0, 0}, {2, -1, 0}, {0, 1, 0}};
    struct sembuf freeTrap[] = {{2, 1, 0}};

    for (int i = 0; i < NPASS; ++i) {
        int pid = fork();
        if (pid == -1) {
            perror("fork error");
            return 0;
        }

        if (pid) {
            printf("Hi, I'm a [%d] passenger\n", getpid());

            //-----------------------------------------------------------------------------------------------------

            while (1) {
                if ((semop(sem_id, landing, 4) == -1) && ((errno == EIDRM) || (errno == EINVAL))) {
                    printf("Passenger [%d] went into the sunset...\n", getpid());
                    return 0;
                }

                printf("\t\t[%d] set on yacht\n", getpid());
                semop(sem_id, freeTrap, 1);
                semop(sem_id, debarkation, 3);
                printf("\t\t[%d] got off the yacht\n", getpid());
                semop(sem_id, freeTrap, 1);
            }

            //-----------------------------------------------------------------------------------------------------
        }
    }

    // main part

    if (captainMode(sem_id, NRIDE, NSEAT) == -1)
        return -1;

    // wait part

    for (int i = 0; i < NPASS + 1; ++i)
        wait(NULL);

    return 0;
}

//=====================================================================================================

int captainMode(int sem_id, int countRides, int countSeats)
{
    int captPid = fork();
    if (captPid == -1) {
        perror("Captain fork error");
        return -1;
    }

    if (captPid) {  // place on yacht; ban of debarcation; trap

        struct sembuf initLanding[] = {{0, countSeats, 0}, {2, 1, 0}};
        struct sembuf waitLanding[] = {{0, 0, 0}, {3, 1, 0}, {2, -1, 0}};
        struct sembuf endOfTour[] = {{1, -countSeats, 0}, {2, 1, 0}};
        struct sembuf waitEscapePass[] = {{0, -countSeats, 0}, {3, -1, 0}, {2, -1, 0}};

        for (int i = 0; i < countRides; ++i) {
            printf("Captain announced [%d] landing:\n", i + 1);
            semop(sem_id, initLanding, 2);
            printf("\tCaptain: i'm waiting\n");

            semop(sem_id, waitLanding, 3);
            usleep(5);  // for be on time printf("\t[%d] set on yacht\n", getpid()); (maybe don't need)
            printf("\tCaptain: land is out. We are leaving\n");

            printf("POEXALI))))\t");  // this part for fun
            for (int j = 0; j < 40; ++j) {
                fflush(NULL);
                usleep(20000);
                printf(".");
            }

            printf("\n\tCaptain: We are arrive\n");
            semop(sem_id, endOfTour, 2);
            printf("\tCaptain: I'm waiting your going off\n");
            semop(sem_id, waitEscapePass, 3);
            printf("Captain got off the yacht\n\n");
        }

        printf("We are closed\n");
        if (semctl(sem_id, IPC_RMID, 0) == -1) {
            perror("semaphor removal error");
            return -1;
        }

        return 0;
    }
}