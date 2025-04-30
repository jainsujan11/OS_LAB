#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h> 
#include <assert.h>


#define TIME 100000
#define CSTART 1100
#define CFRONT 1101
#define CBACK 1102
#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing the V(s) operation */

struct sembuf pop, vop ;
int customer[256], waiter[5], cook, mutex;
int *M;

/* --- HELPER FUNCTIONS --- */
void print_time(int add)
{
    int hr = 11, min = 0;
    min += add;
    hr += min/60;
    min %= 60;
    if(hr > 12)
    {
        hr -= 12;
        if(min < 10 && hr < 10){printf("[0%d:0%d pm]", hr, min);}
        else if(min < 10) {printf("[%d:0%d pm]", hr, min);}
        else if(hr < 10) {printf("[0%d:%d pm]", hr, min);}
        else printf("[%d:%d pm]", hr, min);
    }
    else if(hr == 12)
    {
        if(min < 10){printf("[%d:0%d pm]", hr, min);}
        else printf("[%d:%d pm]", hr, min);
    }
    else
    {
        if(min < 10){printf("[%d:0%d am]", hr, min);}
        else printf("[%d:%d am]", hr, min);
    }

}

void print_space(int i)
{
    if(i == 0) printf(" ");
    else printf("  ");
}

int get_st(int i) {return 200*i+100;}
int front(int i) {return 200*i+102;}
int rear(int i) {return 200*i+103;}
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void cmain(int cook_id)
{
    while(1){
        P(cook);
        P(mutex);
        if(M[4] == 1)
        {
            // denoting last cook leaving so signal all the waiters 
            for (int i = 0; i < 5; i++)
            {
                V(waiter[i]);
            }
            print_time(M[0]);
            print_space(cook_id);
            printf("Cook %c: leaving\n",'C'+cook_id);
            V(mutex);
            exit(0);
        }
        // read information from shared memory
        int waiter_id = M[M[CFRONT]];
        int cust_id = M[M[CFRONT]+1];  
        int cnt = M[M[CFRONT]+2];
        int cur_time = M[0];
        print_time(M[0]);
        print_space(cook_id);
        printf("Cook %c: Preparing order (Waiter %c, Customer %d, Count %d)\n", 'C'+cook_id, 'U'+waiter_id, cust_id, cnt);
        M[CFRONT] += 3;
        M[CSTART]--;
        V(mutex);
        // cook food
        usleep(TIME*5*cnt);
        // signal the waiter
        P(mutex);
        M[0] = cur_time + 5*cnt;
        M[get_st(waiter_id)] = cust_id;
        print_time(M[0]);
        print_space(cook_id);
        printf("Cook %c: Prepared order (Waiter %c, Customer %d, Count %d)\n", 'C'+cook_id, 'U'+waiter_id, cust_id, cnt);
        V(waiter[waiter_id]);
        if(M[0] > 240 && M[CSTART] == 0)
        {
            if(M[4] == 0){
                V(cook);
                M[4]++;
            }
            else{
                for (int i = 0; i < 5; i++)
                {
                    V(waiter[i]);
                }
            }
            print_time(M[0]);
            print_space(cook_id);
            printf("Cook %c: Leaving\n",'C'+cook_id);
            V(mutex);
            exit(0);
        }
        V(mutex);
    }
    exit(0);
}
int main(int argc, char const *argv[])
{
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    key_t key;
    
    // Create customer semaphores
    for (int i = 0; i < 256; i++) {
        key = ftok("customer.c", i);
        customer[i] = semget(key, 1, 0777 | IPC_CREAT);
        if (semctl(customer[i], 0, SETVAL, 0) == -1)
            handle_error("semctl failed for customer semaphore");
    }

    // Create waiter semaphores
    for (int i = 0; i < 5; i++) {
        key = ftok("waiter.c", i);
        waiter[i] = semget(key, 1, 0777 | IPC_CREAT);
        if (semctl(waiter[i], 0, SETVAL, 0) == -1)
            handle_error("semctl failed for waiter semaphore");
    }

    // Create cook semaphore
    key = ftok("cook.c", 1);
    cook = semget(key, 1, 0777 | IPC_CREAT);
    if (semctl(cook, 0, SETVAL, 0) == -1)
        handle_error("semctl failed for cook semaphore");

    // Create mutex semaphore
    key = ftok("/tmp", 65);
    mutex = semget(key, 1, 0777 | IPC_CREAT);
    if (semctl(mutex, 0, SETVAL, 1) == -1)
        handle_error("semctl failed for mutex semaphore");

    // Create shared memory
    key = ftok("/", 65);
    int shmid = shmget(key, 2048 * sizeof(int), 0777 | IPC_CREAT);
    // Attach shared memory
    M = (int *)shmat(shmid, NULL, 0);

    P(mutex);
    // M[0] stores time 
    // M[1] stores number of empty tables 
    // M[2] next waiter 
    M[0] = 0;M[1] = 10;M[2] = 0;M[3] = 0;
    // M[4] stores a flag to find out who is last cook 
    M[4] = 0;
    for (int i = 0; i < 5; i++)
    {
        M[get_st(i)] = 0; // indicate waiter by cook 
        M[get_st(i)+1] = 0; // storing size of queue for waiter 
        M[front(i)] = front(i)+2;
        M[rear(i)] = front(i);
    }
    M[CSTART] = 0; // size of cooking queue 
    M[CBACK] = 1100;
    M[CFRONT] = 1103;
    V(mutex);
    for (int i = 0; i < 2; i++)
    {
        if(fork() == 0) 
        {
            P(mutex);
            print_time(M[0]);
            print_space(i);
            printf("Cook %c is ready\n", 'C'+i);
            V(mutex);
            cmain(i); 
        }
    }
    for (int i = 0; i < 2; i++)
    {
        wait(NULL);
    }
    shmdt(M);
    return 0;
}
