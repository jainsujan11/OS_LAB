#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>   
#include <time.h>     
#include <unistd.h>    
#include <pthread.h>

#define TIME 100000
/* SEMAPHORE IMPLEMENTATION */
typedef struct {
    int value;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} semaphore;

void V(semaphore* s)
{
    pthread_mutex_lock(&(s->mtx));
    s->value++;
    pthread_mutex_unlock(&(s->mtx));
    pthread_cond_signal(&(s->cv));
}
void P(semaphore* s)
{
    pthread_mutex_lock(&(s->mtx));
    while(s->value == 0){
        pthread_cond_wait(&(s->cv),&(s->mtx));
    }
    s->value--;
    pthread_mutex_unlock(&(s->mtx));
}
/* Global Variables and Arrays */
int n,m;
int* BA;int* BC;int* BT;
pthread_barrier_t* BB;
pthread_barrier_t EOS;
int done = 0; // global variable to keep track of number of visitors done 
/* Static Initialization */
semaphore semboat = {0,PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER};
semaphore semrider = {0,PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER};
pthread_mutex_t bmtx = PTHREAD_MUTEX_INITIALIZER;

/* --- HELPER FUNCTIONS --- */
void pboat(int i,char* str)
{
    char *a = "Boat";
    printf("%-10s%4d\t%s\n",a,i,str);
}
void prider(int i,char* str)
{
    char *a = "Visitor";
    printf("%-10s%4d\t%s\n",a,i,str);
}
/* Boat thread */
void* boat(void* arg)
{
    int i = (intptr_t)arg;
    pboat(i+1,"Ready");
    while(1)
    {
        V(&semrider);
        P(&semboat);
        pthread_mutex_lock(&bmtx);
        // making it available only after signal is received as discussed in assg as solution to subtle issue in pseudocode 
        BA[i] = 1;
        BC[i] = -1;
        pthread_mutex_unlock(&bmtx);
        pthread_barrier_wait(&BB[i]); // waiting on a barrier 
        pthread_mutex_lock(&bmtx);
        BA[i] = 0;
        int rtime = BT[i];
        int cur = BC[i];
        pthread_mutex_unlock(&bmtx);
        char str[100];
        sprintf(str,"Start of ride for visitor %2d",cur+1);
        pboat(i+1,str);
        usleep(TIME*rtime);
        sprintf(str,"End of ride for visitor %2d (ride time = %d)",cur+1,rtime);
        pboat(i+1,str);
        pthread_mutex_lock(&bmtx);
        if(done == n)
        {
            pthread_mutex_unlock(&bmtx);
            pthread_barrier_wait(&EOS);
        }
        else{
            pthread_mutex_unlock(&bmtx);
        }
    }
}
/* Busy Wait Implementation of Rider thread */
void* rider(void* arg)
{
    int j = (intptr_t)arg;
    int vtime = rand()%91 + 30;
    int rtime = rand()%61 + 15;
    char str[100];
    sprintf(str,"Starts sightseeing for %3d minutes",vtime);
    prider(j+1,str);
    usleep(vtime*TIME);
    sprintf(str,"Ready to ride a boat (ride time = %d)",rtime);
    prider(j+1,str);
    V(&semboat); // signalling the boat 
    P(&semrider);   // waiting on rider 
    int found = -1 ;
    // busy wait 
    while(1){
        pthread_mutex_lock(&bmtx);
        for (int i = 0; i < m; i++)
        {
            if(BA[i] == 1 && BC[i] == -1)
            {
                found = i;
                break;
            }
        }
        if(found == -1)
        {
            // search again after some sleep 
            pthread_mutex_unlock(&bmtx);
            usleep(TIME/100);
        }
        else{
            break;
        }
    }
    sprintf(str,"Finds boat %d",found+1);
    prider(j+1,str);
    // writes info 
    BC[found] = j;
    BT[found] = rtime;
    pthread_mutex_unlock(&bmtx);
    pthread_barrier_wait(&BB[found]);
    usleep(TIME*rtime);
    sprintf(str,"Leaving");
    prider(j+1,str);
    pthread_mutex_lock(&bmtx);
    done++;
    pthread_mutex_unlock(&bmtx);
    return NULL;
}
void create()
{
    for (int i = 0; i < m; i++)
    {
        // creating boat threads 
        pthread_t tid;
        pthread_create(&tid,NULL,boat,(void *)(__intptr_t)i);
    }
    for (int i = 0; i < n; i++)
    {
        // creating rider threads 
        pthread_t tid;
        pthread_create(&tid,NULL,rider,(void *)(__intptr_t)i);
    }
}
int main(int argc, char const *argv[])
{
    srand(time(NULL));
    if(argc != 3)
    {
        printf("Error\nRun as ./a.out m n\n");
        return 0;
    }
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    /* initializing bmutex to 1*/
    pthread_mutex_trylock(&bmtx);   
    pthread_mutex_unlock(&bmtx);

    pthread_barrier_init(&EOS,NULL,2);

    BA = (int *)malloc(sizeof(int)*m);
    BC = (int *)malloc(sizeof(int)*m);
    BT = (int *)malloc(sizeof(int)*m);
    BB = (pthread_barrier_t *)malloc(sizeof(pthread_barrier_t)*m); 
    for (int i = 0; i < m; i++)
    {
        // initializing 
        pthread_barrier_init(&BB[i], NULL, 2);
    }
    create();
    pthread_barrier_wait(&EOS);
    printf("End of Day !! All rides completed\n");
    pthread_barrier_destroy(&EOS);
    for (int i = 0; i < m; i++)
    {
        pthread_barrier_destroy(&BB[i]);  // Destroy each barrier
    }
    free(BA);free(BB);free(BT);free(BC);
    pthread_mutex_destroy(&bmtx);

    return 0;
}
