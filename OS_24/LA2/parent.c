#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


struct procentry {
   pid_t pid;
   int state;
};
struct procentry *table;
/* this table stores pid of children and whether they are playing or not
    0 -> playing
    1 -> out of game
*/
int next_child = 0;
int remaining;
int received = 0; // flag to handle paused and pre-paused states
void sigusr1_handler(int sig)
{
    // do nothing but dont exit
    // child catches the ball
    received = 1;
}
void sigusr2_handler(int sig)
{
    table[next_child].state = 1;
    remaining--;
    received = 1;
}
int main(int argc, char const *argv[])
{
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    if (argc == 1) {
        printf("Run with number of children\n");
        return 1;
    }
    int n = atoi(argv[1]);
    FILE *fp = fopen("childpid.txt", "w");
    fprintf(fp, "%d\n", n);
    table = (struct procentry*)malloc(n * sizeof(struct procentry));
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            sleep(1); // Wait for parent to finish writing to file
            char child_num[10];
            sprintf(child_num, "%d", i);
            char *args[] = {"./child", child_num, NULL};
            execvp("./child", args);
        } else if (pid > 0) {
            // Parent process
            table[i].pid = pid;
            table[i].state = 0;
            fprintf(fp, "%d\n", pid);
            fflush(stdout);
        }
    }
    fclose(fp);
    printf("Parent: %d child processes created\n", n);
    printf("Parent: Waiting for child processes to read the database\n");
    fflush(stdout);
    sleep(2);


    /* GAME STARTS NOW */
    remaining = n;
    for (int i = 0; i < n; i++)
    {
        if(i<9)printf("       %d", i+1);
        else printf("      %d", i+1);
    }
    printf("\n");
    while(remaining > 1)
    {
        // send SIGUSR2 to next_child
        received = 0;
        while(table[next_child].state != 0)
        {
            next_child = (next_child + 1) % n;
        }
        kill(table[next_child].pid, SIGUSR2);
        if(received == 0)pause();
        // now we have to start the printing process 
        // so we fork a dummy child here 
        printf("+------");
        for (int i = 0; i < n; i++)
        {
            printf("--------");
        }
        printf("+\n");
        pid_t pid = fork();
        if (pid > 0) {
            FILE *dummy_fp = fopen("dummycpid.txt", "w");
            fprintf(dummy_fp, "%d\n", pid);
            fclose(dummy_fp);
            printf("|    ");
            fflush(stdout);
            kill(table[0].pid, SIGUSR1);
            waitpid(pid, NULL, 0);
            printf("  |\n");
            fflush(stdout);
        }
        else if(pid == 0)
        {
            char *args[] = {"./dummy", NULL};
            execvp("./dummy", args);
        }
        next_child = (next_child + 1) % n;
    }
    printf("+------");
    for (int i = 0; i < n; i++)
    {
        printf("--------");
    }
    printf("+\n");
    for (int i = 0; i < n; i++)
    {
        if(i<9)printf("       %d", i+1);
        else printf("      %d", i+1);
    }
    printf("\n");
    // just send SIGINT signals to all the children
    for (int i = 0; i < n; i++)
    {
        kill(table[i].pid, SIGINT);
    }
    return 0;
}