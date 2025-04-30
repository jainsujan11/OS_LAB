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
int get_st(int i) {return 200*i+100;}
int front(int i) {return 200*i+102;}
int rear(int i) {return 200*i+103;}
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
	for (int j = 1; j < i; j++)
	{
		printf("\t");
	}
}
void wmain(int waiter_id)
{
	while (1)	
	{
		P(waiter[waiter_id]);
		P(mutex);
		// end of session condition
		if(M[0] > 240 && M[get_st(waiter_id)+1] == 0 && M[get_st(waiter_id)] == 0)
		{
			print_time(M[0]);
			print_space(waiter_id);
			printf("Waiter %c leaving (no more customer to serve)\n",'U'+waiter_id);
			V(mutex);
			exit(0);
		}
		// denotes some order is ready to be delivered (Cell FR is assg)
		if(M[get_st(waiter_id)] != 0)
		{
			print_time(M[0]);
			print_space(waiter_id);
			printf("Waiter %c: Serving food to Customer %d\n", 'U'+waiter_id, M[get_st(waiter_id)]);
			V(customer[M[get_st(waiter_id)]]);	
			M[get_st(waiter_id)] = 0; // indicate that the order has been served
			V(mutex);
		}
		// denotes PO cell in assg
		else if(M[get_st(waiter_id)+1] != 0)
		{
			// read the order from shared memory
			int cust_id = M[M[front(waiter_id)]];
			int cnt = M[M[front(waiter_id)]+1];
			int cur_time = M[0];
			V(mutex);
			usleep(TIME);
			P(mutex);
			M[0] = cur_time + 1;
			print_time(M[0]);
			print_space(waiter_id);
			printf("Waiter %c: Placing order for Customer %d (count = %d)\n", 'U'+waiter_id, cust_id, cnt);
			V(customer[cust_id]);
			// removing order fro queue
			M[front(waiter_id)] += 2;
			M[get_st(waiter_id)+1]--;
			// writing order in cooks queue
			M[CBACK]+=3;
			M[CSTART]++;
			M[M[CBACK]] = waiter_id;
			M[M[CBACK]+1] = cust_id;
			M[M[CBACK]+2] = cnt;
			V(cook);
			V(mutex);
		}
	}
	exit(0);
}
int main(int argc, char const *argv[])
{
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
	
    key_t key;

    // Attach existing customer semaphores
    for (int i = 0; i < 256; i++) {
        key = ftok("customer.c", i);
        customer[i] = semget(key, 1, 0777);
    }

    // Attach existing waiter semaphores
    for (int i = 0; i < 5; i++) {
        key = ftok("waiter.c", i);
        waiter[i] = semget(key, 1, 0777);
    }

    // Attach existing cook semaphore
    key = ftok("cook.c", 1);
    cook = semget(key, 1, 0777);

    // Attach existing mutex semaphore
    key = ftok("/tmp", 65);
    mutex = semget(key, 1, 0777);

    // Attach shared memory
    key = ftok("/", 65);
    int shmid = shmget(key, 2048 * sizeof(int), 0777);
    M = (int *)shmat(shmid, 0, 0);
	for (int i = 0; i < 5; i++)
	{
		if (fork() == 0)
		{
			wmain(i);
		}
		else{
			P(mutex);
			print_time(M[0]);
			print_space(i);
			printf("Waiter %c is ready\n", 'U'+i);
			V(mutex);
		}
	}
	for (int i = 0; i < 5; i++)
	{
		wait(NULL);
	}
	shmdt(M);
    return 0;
}
