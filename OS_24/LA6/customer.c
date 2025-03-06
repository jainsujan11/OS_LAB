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
#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */
struct sembuf pop, vop ;
int customer[256], waiter[5], cook, mutex;
int *M;


void signal_handler(int signo) {
	int shmkey = ftok("/",65);
    int shmid = shmget(shmkey, 2048*sizeof(int), 0777);
	shmdt(M);
	shmctl(shmid, IPC_RMID, 0);
	for (int i = 0; i < 256; i++)
	{
		semctl(customer[i], 0, IPC_RMID,0);
	}
	for (int i = 0; i < 5; i++)
	{
		semctl(waiter[i], 0, IPC_RMID,0);
	}
	semctl(cook, 0, IPC_RMID,0);
	semctl(mutex, 0, IPC_RMID,0);
}
int get_st(int i) {return 200*i+100;}
int front(int i) {return 200*i+102;}
int rear(int i) {return 200*i+103;}
void print_time(int add)
{
    // initial time is 11:00 am, write code to print the time after 'add' minutes
    // also primt am/pm
    int hr = 11, min = 0;
    min += add;
    hr += min/60;
    min %= 60;
    if(hr > 12)
    {
        hr -= 12;
        if(min < 10){printf("[%d:0%d pm]", hr, min);}
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
char get_wt(int i)
{
	return (char)(i+'U');
}
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
void cmain(int id,int cnt,int arrival)
{
	P(mutex);
	if(M[0] > 240)
	{
		print_time(M[0]);
		printf(" \t\t\t\t\t Customer %d leaves (late arrival)\n", id);
		V(mutex);
		exit(0);
	} 
	if(M[1] == 0)
	{
		print_time(M[0]);
		printf(" \t\t\t\t\t Customer %d leaves (no empty table)\n", id);
		V(mutex);
		exit(0);
	}
	print_time(M[0]);
	printf(" Customer %d arrives (count = %d)\n", id, cnt);
	assert(M[0] == arrival);
	M[1]--;
	int assg_wt = M[2];
	M[2] = (M[2]+1)%5;
	M[get_st(assg_wt)+1]++;
	M[rear(assg_wt)]+=2;
	M[M[rear(assg_wt)]] = id;
	M[M[rear(assg_wt)]+1] = cnt;
	V(mutex);
	V(waiter[assg_wt]);
	P(customer[id]);
	P(mutex);
	print_time(M[0]);
	printf("  Customer %d: Order placed to waiter %c\n", id, get_wt(assg_wt));
	V(mutex);
	// wait for food to be served by waiter
	P(customer[id]);
	P(mutex);
	print_time(M[0]);
	printf(" \tCustomer %d gets food [Waitng time = %d]\n", id, M[0]-arrival);
	int time = M[0];
	V(mutex);
	usleep(TIME*30);
	P(mutex);
	M[0] = time + 30;
	M[1]++;
	print_time(M[0]);
	printf(" \t\tCustomer %d finishes eating and leaves\n", id);
	V(mutex);
	exit(0);
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, signal_handler);
	signal(SIGSEGV, signal_handler);
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
	key_t key;

    // Attach existing customer semaphores
    for (int i = 0; i < 256; i++) {
        key = ftok("customer.c", i);
        if (key == -1) handle_error("ftok failed for customer semaphore");

        customer[i] = semget(key, 1, 0777);
        if (customer[i] == -1) handle_error("semget failed for customer semaphore");
    }

    // Attach existing waiter semaphores
    for (int i = 0; i < 5; i++) {
        key = ftok("waiter.c", i);
        if (key == -1) handle_error("ftok failed for waiter semaphore");

        waiter[i] = semget(key, 1, 0777);
        if (waiter[i] == -1) handle_error("semget failed for waiter semaphore");
    }

    // Attach existing cook semaphore
    key = ftok("cook.c", 1);
    if (key == -1) handle_error("ftok failed for cook semaphore");

    cook = semget(key, 1, 0777);
    if (cook == -1) handle_error("semget failed for cook semaphore");

    // Attach existing mutex semaphore
    key = ftok("/tmp", 65);
    if (key == -1) handle_error("ftok failed for mutex semaphore");

    mutex = semget(key, 1, 0777);
    if (mutex == -1) handle_error("semget failed for mutex semaphore");

    // Attach shared memory
    key = ftok("/", 65);
    if (key == -1) handle_error("ftok failed for shared memory");

    int shmid = shmget(key, 2048 * sizeof(int), 0777);
    if (shmid == -1) handle_error("shmget failed for shared memory");

    M = (int *)shmat(shmid, NULL, 0);
    if (M == (void *)-1) handle_error("shmat failed");
	// read from customer.txt
	FILE *fp = fopen("customers.txt", "r");
	int arr[256][3];
	int ptr = 0;
	while(1)
	{
		int id,arr_time,cnt;
		fscanf(fp, "%d %d %d", &id, &arr_time, &cnt);
		if(id == -1) break;
		arr[ptr][0] = id;
		arr[ptr][1] = arr_time;
		arr[ptr][2] = cnt;
		ptr++;

	}
	fclose(fp);
	int last_time = -1;
	int num_customer=0;
	int i=0;
	while(i<ptr)
	{
		int id,arr_time,cnt;
		id = arr[i][0];
		arr_time = arr[i][1];
		cnt = arr[i][2];
		P(mutex);
		if (mutex == -1) {
			perror("semget failed for mutex");
			exit(1);
		}
		if(last_time == -1){
	
			M[0] = arr_time;
			last_time = arr_time;
			V(mutex);
			if (mutex == -1) {
				perror("semget failed for mutex");
				exit(1);
			}
		}
		else if(last_time < arr_time){
			int cur_time = M[0];
			V(mutex);
			usleep((arr_time - cur_time)*TIME);
			P(mutex);
			M[0] = arr_time;
			last_time = arr_time;
			V(mutex);
		}
		else{
			V(mutex);
			if (mutex == -1) {
				perror("semget failed for mutex");
				exit(1);
			}
		}
		num_customer++;
		if(fork() == 0){
			cmain(id,cnt,arr_time);
		}
		i++;
	}
	for (int i = 0; i < num_customer; i++)
	{
		wait(NULL);
	}
	

	// detach all shared memory segments
	shmdt(M);
	shmctl(shmid, IPC_RMID, 0);
	for (int i = 0; i < 256; i++)
	{
		semctl(customer[i], 0, IPC_RMID,0);
	}
	for (int i = 0; i < 5; i++)
	{
		semctl(waiter[i], 0, IPC_RMID,0);
	}
	semctl(cook, 0, IPC_RMID,0);
	semctl(mutex, 0, IPC_RMID,0);

    return 0;
}
