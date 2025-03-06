#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>



void signal_handler(int signo) {
    // detach all shared memory segments and semaphores
    // remove all shared memory segments and semaphores
    semdtl(shmid1, shmaddr1, 0);
    semdtl(shmid2, shmaddr2, 0);
    shmctl(shmid1, IPC_RMID, 0);
    shmctl(shmid2, IPC_RMID, 0);
    semctl(semid1, 0, IPC_RMID, 0);
    semctl(semid2, 0, IPC_RMID, 0);
    exit(0);
}


int main(int argc, char const *argv[])
{
    signal(SIGINT, signal_handler);
    return 0;
}
