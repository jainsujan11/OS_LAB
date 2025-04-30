#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#define PLAYING 0
#define CATCHMADE 1
#define CATCHMISSED 2
#define OUTOFGAME 3
// 4 status as mentioned in the question

struct procentry {
   pid_t pid;
   pid_t npid;
   int state;
};
// stores current child id, next child id and state

struct procentry cur;
int childnum;
void sigusr2_handler(int sig)
{
    // take a random number 0 to 999
    int num = rand() % 1000;
    // [0,799] -> 80% chance of catching
    // [800,999] -> 20% chance of missing
    int parent = getppid();
    if(num < 800){
        cur.state = CATCHMADE;
        kill(parent, SIGUSR1);
    } else {
        cur.state = CATCHMISSED;
        kill(parent, SIGUSR2);
    }
}

void sigusr1_handler(int sig)
{
    // first print the message and update the information
    if(cur.state == PLAYING){printf("....    ");}
    else if(cur.state == CATCHMADE){printf("CATCH   ");cur.state = PLAYING;}
    else if(cur.state == CATCHMISSED){printf("MISS    ");cur.state = OUTOFGAME;}
    else if(cur.state == OUTOFGAME){printf("        ");}
    fflush(stdout);
    // then send the signal to the next
    if(cur.pid == cur.npid){
        // send SIGINT to DUMMY NODE
        // read file dummycpid.txt
        FILE *fp = fopen("dummycpid.txt", "r");
        pid_t dummycpid;
        fscanf(fp, "%d", &dummycpid);
        fclose(fp);
        kill(dummycpid, SIGINT);
    }
    else kill(cur.npid, SIGUSR1);
}
void sigint_handler(int sig)
{
    if(cur.state == PLAYING)
    {
        printf("+++ Child %d: Yay! I am the winner !\n",childnum+1);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    srand(getpid());
    childnum = atoi(argv[1]);
    FILE *fp = fopen("childpid.txt", "r");
    int n;
    fscanf(fp, "%d", &n);
    pid_t *pids = malloc(n * sizeof(pid_t));
    for (int i = 0; i < n; i++) {
        fscanf(fp, "%d", &pids[i]);
    }
    fclose(fp);
    cur.pid = getpid();
    // error checking not needed otherwise
    if(getpid() != pids[childnum]){
        printf("Error: Child number does not match\n");
        return 1;
    }
    cur.state = PLAYING;
    cur.npid = pids[(childnum + 1) % n];
    if((childnum+1)%n == 0)
    {
        cur.npid = cur.pid;
    }
    while(1) 
    {
        pause(); // waiting to SIGUSR2
    }
    return 0;
}