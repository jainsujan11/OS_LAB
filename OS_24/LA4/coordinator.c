#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "boardgen.c"

// global 9 pipe 
int block[9][2];
// storing child PIDs 
pid_t cpid[9];
int stdout_copy;
// A and S as in assg
int A[9][9], S[9][9];

/*
    Reading only occurs from stdin so closed all read ends of pipes for coordinator
    By default, coordinator writes to stdout but we change when required
*/

void draw_board()
{
    printf("\t+---+---+---+\n");
    for (int i = 0; i < 3; i++) {
        printf("\t|");
        for (int j = 0; j < 3; j++) {
            printf(" %d |", i*3+j);
        }
        printf("\n\t+---+---+---+\n");
    }
}
void help_message()
{
    printf("Commands Supported\n");
    printf("\t%-10s%s","n","Start new game\n");
    printf("\t%-10s%s","p b c d","Put a digit d [1-9] at cell c [0-8] of block b [0-8]\n");
    printf("\t%-10s%s","s","Show solution\n");
    printf("\t%-10s%s","h","Print this help message\n");
    printf("\t%-10s%s","q","Quit\n");

    printf("Numbering scheme for blocks and cells\n");
    draw_board();
}

void fork_child()
{
    char bfdin[32], bfdout[32], rn1fdout[32], rn2fdout[32], cn1fdout[32], cn2fdout[32]; 
    // block 0
    sprintf(bfdin, "%d", block[0][0]); sprintf(bfdout, "%d", block[0][1]);
    sprintf(rn1fdout, "%d", block[1][1]); sprintf(rn2fdout, "%d", block[2][1]);
    sprintf(cn1fdout, "%d", block[3][1]); sprintf(cn2fdout, "%d", block[6][1]);
    cpid[0] = fork();
    if(cpid[0] == 0){
        execlp("xterm","xterm", "-T", "Block 0", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1000+300", "-bg", "black","-fg", "#00FF00", "-e", "./block", "0", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 1
    sprintf(bfdin, "%d", block[1][0]); sprintf(bfdout, "%d", block[1][1]);
    sprintf(rn1fdout, "%d", block[0][1]); sprintf(rn2fdout, "%d", block[2][1]);
    sprintf(cn1fdout, "%d", block[4][1]); sprintf(cn2fdout, "%d", block[7][1]);
    cpid[1] = fork();
    if(cpid[1] == 0){
        execlp("xterm","xterm", "-T", "Block 1", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1250+300", "-bg", "black","-fg", "#00FF00", "-e", "./block", "1", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 2
    sprintf(bfdin, "%d", block[2][0]); sprintf(bfdout, "%d", block[2][1]);
    sprintf(rn1fdout, "%d", block[0][1]); sprintf(rn2fdout, "%d", block[1][1]);
    sprintf(cn1fdout, "%d", block[5][1]); sprintf(cn2fdout, "%d", block[8][1]);
    cpid[2] = fork();
    if(cpid[2] == 0){
        execlp("xterm","xterm", "-T", "Block 2", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1500+300", "-bg", "black","-fg", "#00FF00", "-e", "./block", "2", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 3
    sprintf(bfdin, "%d", block[3][0]); sprintf(bfdout, "%d", block[3][1]);
    sprintf(rn1fdout, "%d", block[4][1]); sprintf(rn2fdout, "%d", block[5][1]);
    sprintf(cn1fdout, "%d", block[0][1]); sprintf(cn2fdout, "%d", block[6][1]);
    cpid[3] = fork();
    if(cpid[3] == 0){
        execlp("xterm","xterm", "-T", "Block 3", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1000+550", "-bg", "black","-fg", "#00FF00", "-e", "./block", "3", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 4
    sprintf(bfdin, "%d", block[4][0]); sprintf(bfdout, "%d", block[4][1]);
    sprintf(rn1fdout, "%d", block[3][1]); sprintf(rn2fdout, "%d", block[5][1]);
    sprintf(cn1fdout, "%d", block[1][1]); sprintf(cn2fdout, "%d", block[7][1]);
    cpid[4] = fork();
    if(cpid[4] == 0){
        execlp("xterm","xterm", "-T", "Block 4", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1250+550", "-bg", "black","-fg", "#00FF00", "-e", "./block", "4", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 5
    sprintf(bfdin, "%d", block[5][0]); sprintf(bfdout, "%d", block[5][1]);
    sprintf(rn1fdout, "%d", block[3][1]); sprintf(rn2fdout, "%d", block[4][1]);
    sprintf(cn1fdout, "%d", block[2][1]); sprintf(cn2fdout, "%d", block[8][1]);
    cpid[5] = fork();
    if(cpid[5] == 0){
        execlp("xterm","xterm", "-T", "Block 5", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1500+550", "-bg", "black","-fg", "#00FF00", "-e", "./block", "5", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 6
    sprintf(bfdin, "%d", block[6][0]); sprintf(bfdout, "%d", block[6][1]);
    sprintf(rn1fdout, "%d", block[7][1]); sprintf(rn2fdout, "%d", block[8][1]);
    sprintf(cn1fdout, "%d", block[0][1]); sprintf(cn2fdout, "%d", block[3][1]);
    cpid[6] = fork();
    if(cpid[6] == 0){
        execlp("xterm","xterm", "-T", "Block 6", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1000+800", "-bg", "black","-fg", "#00FF00", "-e", "./block", "6", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 7
    sprintf(bfdin, "%d", block[7][0]); sprintf(bfdout, "%d", block[7][1]);
    sprintf(rn1fdout, "%d", block[6][1]); sprintf(rn2fdout, "%d", block[8][1]);
    sprintf(cn1fdout, "%d", block[1][1]); sprintf(cn2fdout, "%d", block[4][1]);
    cpid[7] = fork();
    if(cpid[7] == 0){
        execlp("xterm","xterm", "-T", "Block 7", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1250+800", "-bg", "black","-fg", "#00FF00", "-e", "./block", "7", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
    // block 8
    sprintf(bfdin, "%d", block[8][0]); sprintf(bfdout, "%d", block[8][1]);
    sprintf(rn1fdout, "%d", block[6][1]); sprintf(rn2fdout, "%d", block[7][1]);
    sprintf(cn1fdout, "%d", block[2][1]); sprintf(cn2fdout, "%d", block[5][1]);
    cpid[8] = fork();
    if(cpid[8] == 0){
        execlp("xterm","xterm", "-T", "Block 8", "-fa", "Monospace", "-fs", "15", "-geometry", "17x8+1500+800", "-bg", "black","-fg", "#00FF00", "-e", "./block", "8", bfdin, bfdout, rn1fdout, rn2fdout, cn1fdout, cn2fdout, NULL);
    }
}

void quit()
{
    stdout_copy = dup(1);
    for (int i = 0; i < 9; i++)
    {
        // passing to all childs
        close(1);
        dup(block[i][1]);
        printf("q\n");
    }
    close(1);
    dup(stdout_copy);
    exit(0);
}

void send_board(int A[9][9])
{
    stdout_copy = dup(1);
    // passing 9 integers of a block
    for (int i = 0; i < 9; i++)
    {
        close(1);
        dup(block[i][1]);
        printf("n ");
        // 3x3 block
        int r = i/3, c = i-3*r;
        r*=3;c*=3;
        for (int j = r; j < r+3; j++)
        {
            for (int k = c; k < c+3; k++)
            {
                printf("%d ", A[j][k]);
            }
        }
        printf("\n");
    }
    close(1);
    dup(stdout_copy);
}

int check(int b,int c,int d)
{
    if(b>=0 && b<=8 && c>=0 && c <= 8 && d >= 1 && d <= 9) return 1;
    return 0;
}

void place(int b,int c,int d)
{
    // placing a digit by coordinator 
    stdout_copy = dup(1);
    close(1);
    dup(block[b][1]);
    printf("p %d %d\n",c,d);
    close(1);
    dup(stdout_copy);
}

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 9; i++)
    {
        // making pipes 
        pipe(block[i]);
    }
    help_message();
    fork_child();
    for (int i = 0; i < 9; i++)
    {
        close(block[i][0]); // coordinator reads only from stdin so no need for them 
    }
    while(1)
    {
        printf("Fooduko> ");
        char s[100];
        scanf(" %[^\n]s", s);
        // doing as per instructions 
        if(s[0] == 'n'){
            newboard(A, S);
            send_board(A);
        }
        else if(s[0] == 'p'){
            int b, c, d;
            sscanf(s, "p %d %d %d", &b, &c, &d);
            if(!check(b,c,d)) printf("ERROR!! Invalid Entry\n");
            else place(b,c,d);
        }
        else if(s[0] == 's'){
            send_board(S);
        }
        else if(s[0] == 'h'){help_message();}
        else if(s[0] == 'q'){quit();}
    }
    return 0;
}