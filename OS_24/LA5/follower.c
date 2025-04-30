#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    int nf = 1;
    if (argc == 2) {
        nf = atoi(argv[1]);
    }
    const char *pathname = "/";
    int proj_id = 65;
    int key = ftok(pathname, proj_id);
    int shmid = shmget(key, 0, 0777);
    if (shmid == -1) {
        printf("Leader not running\n");
        exit(1);
    }
    int *M = (int *)shmat(shmid, 0, 0);
    for (int i = 0; i < nf; i++) {
        if (M[1] == M[0]) {
            printf("follower error: %d followers have already joined\n", M[0]);
            continue;
        }
        ++M[1];
        int cpid = fork();
        if (cpid == 0) {
            srand(getpid());
            // const char *pathname = "/";
            // int proj_id = 65;
            // int key = ftok(pathname, proj_id);
            // int shmid = shmget(key, 0, 0777);
            // int *M = (int *)shmat(shmid, 0, 0);
            int fno = M[1];
            printf("follower %d joins\n", fno);
            while (1) {
                while (abs(M[2]) != (fno)) {
                    ;
                }
                if (M[2] == fno) {
                    int rnd = rand() % 9 + 1;
                    M[fno + 3] = rnd;
                    M[2] = (fno + 1);
                    if (M[2] == M[0] + 1) M[2] = 0;
                } else if (M[2] == -fno) {
                    printf("follower %d leaves\n", fno);
                    M[2] = -(fno + 1);
                    if (M[2] == -(M[0] + 1)) M[2] = 0;
                    shmdt(M);
                    exit(1);
                }
            }
        } else {
            sleep(1);
            continue;
        }
    }
    for (int i = 0; i < M[0]; i++)
    {
        wait(NULL);
    }
    shmdt(M);
    return 0;
}
