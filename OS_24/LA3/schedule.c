#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "queue.h"
#include "min_heap.h"


#define NEW_ARRIVAL 0
#define IO_COMPLETE 1
#define BURST_END 2
#define TIME_OUT 3


int n = 0;
int free_time = 0;
int cur_time = 0;
int idle_time = 0;
int wait_time = 0;
typedef struct pcb
{
    int id;
    int num_of_burst;
    int state;
    int bursts[20];
    int cur_burst;
    int next_time;
    int running_time;
    int turnaround_time;
    int rbt;
}pcb;

pcb* infotable;
queue ready_queue;

typedef struct{
    int H[1024];
}heap;
heap event_queue;

int comp1(int a,int b)
{
    if(infotable[a].next_time == infotable[b].next_time)
    {
        if(((infotable[a].state == NEW_ARRIVAL || infotable[a].state == IO_COMPLETE) && 
        (infotable[b].state == TIME_OUT || infotable[b].state == BURST_END)))
        {
            return 1;
        }
        if((infotable[b].state == NEW_ARRIVAL || infotable[b].state == IO_COMPLETE) && 
        (infotable[a].state == TIME_OUT || infotable[a].state == BURST_END))
        {
            return 0;
        }
        return infotable[a].id < infotable[b].id;
    }
    return infotable[a].next_time < infotable[b].next_time;
}
void shiftUp(int H[],int n) {
    while (n > 1 && comp1(H[n],H[Parent(n)])) {
        swap(&H[Parent(n)], &H[n]);
        n = Parent(n);
    }
}
void shiftDown(int H[],int n) {
    while (LeftChild(n) <= size) {
        int child = LeftChild(n);
        if (RightChild(n) <= size && comp1(H[RightChild(n)],H[LeftChild(n)])) {
            child = RightChild(n);
        }
        if (comp1(H[child],H[n])) {
            swap(&H[n], &H[child]);
        } else {
            break;
        }
        n = child;
    }
}
void readfile()
{
    FILE* file = fopen("proc.txt", "r");
    char line[256];
    fgets(line, sizeof(line), file);
    n = atoi(line);
    infotable = (pcb*)malloc((n+1)*sizeof(pcb));
    for (int i = 1; i <= n; i++)
    {
        fgets(line, sizeof(line), file);
        // printf("%s", line);
        char* token = strtok(line, " \t\n");
        infotable[i].id = atoi(token);
        infotable[i].cur_burst = 0;
        infotable[i].num_of_burst = 0;
        infotable[i].state = NEW_ARRIVAL;
        infotable[i].rbt = 0;
        token = strtok(NULL, " \t\n");
        infotable[i].next_time = atoi(token);
        infotable[i].running_time = 0;
        infotable[i].turnaround_time = -(infotable[i].next_time);
        token = strtok(NULL, " \t\n");
        while(atoi(token)!=-1)
        {
            infotable[i].bursts[infotable[i].num_of_burst] = atoi(token);
            infotable[i].running_time += atoi(token);
            infotable[i].num_of_burst++;
            token = strtok(NULL, " \t\n");
        }
    }
    fclose(file);
}
void init()
{
    
    for(int i=1;i<=n;i++)
    {
        Insert(event_queue.H, i);
    }
}
void handle(int cur)
{
    pcb* cur_pcb = &infotable[cur];
    // print details of cur_pcb
    if(cur_pcb -> state == NEW_ARRIVAL)
    {
        ready_queue = enqueue(ready_queue, cur);
        cur_pcb -> state = TIME_OUT;
        #ifdef VERBOSE
        char str[] = ": Process ";
        printf("%-10d",cur_time);
        printf("%s %d joins ready queue upon arrival\n", str, cur_pcb -> id);
        #endif
        cur_pcb -> rbt = cur_pcb -> bursts[cur_pcb -> cur_burst];
        cur_pcb -> cur_burst++;
    }
    else if(cur_pcb -> state == IO_COMPLETE)
    {
        #ifdef VERBOSE
        char str[] = ": Process ";
        printf("%-10d",cur_time);
        printf("%s %d joins ready queue after IO completion\n", str, cur_pcb -> id);
        #endif
        ready_queue = enqueue(ready_queue, cur);
        cur_pcb -> state = TIME_OUT;
        cur_pcb -> rbt = cur_pcb -> bursts[cur_pcb -> cur_burst];
        cur_pcb -> cur_burst++;
    } 
    else if(cur_pcb -> state == BURST_END)
    {
         
        if(cur_pcb -> cur_burst != cur_pcb -> num_of_burst)
        {
            cur_pcb -> state = IO_COMPLETE;
            cur_pcb -> next_time = cur_time + cur_pcb -> bursts[cur_pcb -> cur_burst]; 
            cur_pcb -> cur_burst++;
            Insert(event_queue.H, cur);
           
        }
        else{
            // this process is done
            cur_pcb -> turnaround_time += cur_time;
            int percent = round((cur_pcb -> turnaround_time * 100.0) / cur_pcb -> running_time); 
            wait_time += cur_pcb -> turnaround_time - cur_pcb -> running_time;
            char str[] = ": Process ";  
            printf("%-10d",cur_time);
            printf("%s", str);
            printf("%6d exits. Turnaround time = %5d (%d%%), Wait time = %d\n", cur_pcb -> id, cur_pcb -> turnaround_time,percent, cur_pcb -> turnaround_time - cur_pcb -> running_time);
        }
    }
    else{
        #ifdef VERBOSE
        char str[] = ": Process ";
        printf("%-10d",cur_time);
        printf("%s %d joins ready queue after timeout\n", str, cur_pcb -> id);
        #endif
        ready_queue = enqueue(ready_queue, cur);
    }
}
void round_robin(int q)
{
    readfile();
    ready_queue = initq();  
    init();
    #ifdef VERBOSE
    printf("%-10d : Starting\n",0);
    #endif
    while(size)
    {
        int cur_event = getMin(event_queue.H);
        Pop(event_queue.H);
        cur_time = infotable[cur_event].next_time;
        handle(cur_event);
        if(free_time <= cur_time && isEmpty(ready_queue))
        {
            #ifdef VERBOSE
            char str[] = ": CPU goes idle\n";
            if(isEmpty(ready_queue))
            {
                printf("%-10d",cur_time);
                printf("%s", str);
            }            
            #endif
        }
        if(free_time <= cur_time && !isEmpty(ready_queue))
        {
            int cur = front(ready_queue);
            idle_time += cur_time - free_time;
            ready_queue = dequeue(ready_queue);
            if(infotable[cur].rbt > q)
            {
                #ifdef VERBOSE
                char str[] = ": Process ";
                printf("%-10d",cur_time);
                printf("%s %d is scheduled to run for time %d\n", str, infotable[cur].id,q);
                #endif
                infotable[cur].rbt -= q;
                free_time = cur_time + q;
                infotable[cur].next_time = free_time;
                Insert(event_queue.H, cur);
            }
            else{
                #ifdef VERBOSE
                char str[] = ": Process ";
                printf("%-10d",cur_time);
                printf("%s %d is scheduled to run for time %d\n", str, infotable[cur].id,infotable[cur].rbt);
                #endif
                free_time = cur_time + infotable[cur].rbt;
                infotable[cur].state = BURST_END;
                infotable[cur].next_time = free_time;
                infotable[cur].rbt = 0;
                Insert(event_queue.H, cur);
            }
        }
    }
    float avg_wait_time = (float)wait_time / n;
    float cpu_util = 100 - ((float)idle_time / cur_time) * 100;
    printf("\nAverage wait time = %.2f\n", avg_wait_time);
    printf("Total turnaround time = %d\n", cur_time);
    printf("CPU idle time = %d\n", idle_time);
    printf("CPU utilization = %.2f%%\n", cpu_util);

    // free memory and reset global variables
    free(infotable);
    size = 0;
    free_time = 0;
    cur_time = 0;
    idle_time = 0;
    wait_time = 0;

}

int main()
{
    int q = INT_MAX;
    printf("**** FCFS Scheduling ****\n");
    round_robin(q);
    printf("\n");
    q = 10;
    printf("**** RR Scheduling with q = %d ****\n",q);
    round_robin(q);
    printf("\n");
    q = 5;
    printf("**** RR Scheduling with q = %d ****\n",q);
    round_robin(q);
    printf("\n");
    return 0;
}