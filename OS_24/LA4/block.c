#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


int block,bin,bout,rn1,rn2,cn1,cn2;
int A[3][3], B[3][3];
int stdout_copy;
void draw_board()
{
    printf("+---+---+---+\n");
    for (int i = 0; i < 3; i++) {
        printf("|");
        for (int j = 0; j < 3; j++) {
            if (B[i][j] == 0) {
                printf("   |");  // Print spaces instead of 0
            } else {
                printf(" %d |", B[i][j]);
            }
        }
        printf("\n+---+---+---+\n");
    }
}

int check_read_only(int c)
{
    int r = c/3;
    int col = c - r*3;
    if(A[r][col]!=0) return 1;
    return 0;
}

int check_block_conflict(int d)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if(B[i][j] == d) return 1;
        }
    }
    return 0;
}

int check_row_conflict(int rn,int cell,int d)
{
    int i = cell/3;
    stdout_copy = dup(1);
    close(1);
    dup(rn);
    printf("r %d %d %d\n",i,d,bout);
    close(1);
    dup(stdout_copy);
    char msg[10];
    scanf(" %[^\n]s",msg);
    if(msg[0] == '0') return 0;
    else return 1;
}

int check_column_conflict(int cn,int cell,int d)
{
    int j = cell - (cell/3)*3;
    stdout_copy = dup(1);
    close(1);
    dup(cn);
    printf("c %d %d %d\n",j,d,bout);
    close(1);
    dup(stdout_copy);
    char msg[10];
    scanf(" %[^\n]s",msg);
    if(msg[0] == '0') return 0;
    else return 1;
}

// by default, 1 -> stdout 
int main(int argc, char const *argv[])
{
    block = atoi(argv[1]);
    // keep read end from coord write end
    bin = atoi(argv[2]);bout = atoi(argv[3]);
    rn1 = atoi(argv[4]);rn2 = atoi(argv[5]);
    cn1 = atoi(argv[6]);cn2 = atoi(argv[7]);
    printf("Block %d ready\n", block);
    close(0); // never going to read from stdin 
    dup(bin); // read from coord write end
    while(1)
    {
        char s[100];
        scanf(" %[^\n]s", s);
        if(s[0] == 'n'){
            // populate A and B
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    A[i][j] = s[2+2*(i*3+j)] - '0';
                    B[i][j] = A[i][j];
                }
            }
            draw_board();
        }
        else if(s[0] == 'p')
        {
            int c = s[2] - '0';
            int d = s[4] - '0';
            int flag = 1;
            if(check_read_only(c)) printf("Readonly cell\n");
            else if(check_block_conflict(d)) printf("Block Conflict\n");
            else if(check_row_conflict(rn1,c,d) || check_row_conflict(rn2,c,d)) printf("Row conflict\n");
            else if(check_column_conflict(cn1,c,d) || check_column_conflict(cn2,c,d)) printf("Column conflict\n");
            else{
                flag = 0;
                int row = c/3;
                int col = c - row*3;
                B[row][col] = d;
            }
            if(flag) sleep(2);
            draw_board();
        }
        else if(s[0] == 'r')
        {
            int row = s[2] - '0';
            int digit = s[4] - '0';
            int write = s[6] - '0';
            stdout_copy = dup(1);
            close(1);
            dup(write);
            int flag = 0;
            for (int col = 0; col < 3; col++)
            {
                if(B[row][col] == digit) flag = 1;
            }
            if(flag) printf("1\n");
            else printf("0\n");
            close(1);
            dup(stdout_copy);
        }
        else if(s[0] == 'c')
        {
            int col = s[2] - '0';
            int digit = s[4] - '0';
            int write = s[6] - '0';
            stdout_copy = dup(1);
            close(1);
            dup(write);
            int flag = 0;
            for (int row = 0; row < 3; row++)
            {
                if(B[row][col] == digit) flag = 1;
            }
            if(flag) printf("1\n");
            else printf("0\n");
            close(1);
            dup(stdout_copy);
        }
        else if(s[0] == 'q'){
            printf("Bye... \n");
            sleep(2);
            exit(block);
        }
    }
    return 0;
}

