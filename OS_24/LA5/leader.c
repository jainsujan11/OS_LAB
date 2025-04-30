#include <malloc.h>
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
    int n = 10;
    if (argc == 2) {
        n = atoi(argv[1]);
    }
    int shmid;
    const char *pathname = "/";
    int proj_id = 65;
    int key = ftok(pathname, proj_id);
    int x = 104;
    shmid = shmget(key, x * sizeof(int), 0777 | IPC_CREAT | IPC_EXCL);
    int *M = (int *)shmat(shmid, 0, 0);
    M[0] = n;
    M[1] = M[2] = 0;
    while (M[1] < n) {
        ;
    }
    // max sum generated can be atmost 100*10 + 99 = 1000 + 99 = 1099
    int *hash;
    hash = (int *)malloc(1100 * sizeof(int));
    for (int i = 0; i < 1100; i++) {
        hash[i] = 0;
    }
    int sm = 0;
    srand(time(0));
    while (1) {
        if (M[2] == 0) {
            int rnd = rand() % 99 + 1;
            M[3] = rnd;
            M[2] = 1;
        }
        while (M[2] != 0) {
            ;
        }
        printf("%d", M[3]);
        sm = M[3];
        for (int i = 4; i <= n + 3; i++) {
            printf(" + %d", M[i]);
            sm += M[i];
        }
        printf(" = %d\n", sm);
        hash[sm]++;
        if (hash[sm] == 2) break;
        sm = 0;
    }
    M[2] = -1;
    while (M[2] != 0) {
        ;
    }
    shmdt(M);
    shmctl(shmid, IPC_RMID, 0);
    return 0;
}
