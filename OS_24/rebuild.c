// store visited info in a file
// use hard disk for the array
// initialize the array
// before every child, read the array fresh 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char const *argv[])
{
    if (argc == 1) {
        printf("Run with a node number\n");
        return 1;
    }
    char * node = argv[1];
    if(argc == 2)
    {
        FILE *foodep = fopen("foodep.txt", "r");
        int n;
        fscanf(foodep, "%d", &n);
        fclose(foodep);
        FILE *done = fopen("done.txt", "w");
        for (int i = 0; i < n; i++) {
            fprintf(done, "0");
        }
        fclose(done);
    }
    int root;
    root = atoi(node);
    FILE *fp = fopen("foodep.txt", "r");
    char line[100];
    char *found = NULL;
    // Skip the first line of the file
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        // search for the node name in just start of the line, like compare len of node with line[0:len]
        if (strncmp(node, line, strlen(node)) == 0) {
            found = line;
            break;
        }
    }
    fclose(fp);
    // found[strlen(found) - 1] = '\0';
    // printf("%s",found);
    if (found != NULL) {
        char *token = strtok(found, " \t\n");
        // printf("%s",token);
        token = strtok(NULL, " \n\t"); 
        // printf("%s",token);
        int num_child = 0;
        int children[100];
        while (token!=NULL) {
            pid_t pid = fork();
            if(pid == 0){
                int child = atoi(token);
                // char child_str[10];
                // printf("%s\n",token);
                children[num_child++] = child;
                FILE *done = fopen("done.txt", "r+");
                fseek(done, child-1, SEEK_SET);
                char status = fgetc(done);
                // printf("%c\n",status);
                if (status == '0') {
                    char *args[] = {"rebuild",token,"1", NULL};
                    execv("rebuild",args);
                }
                fclose(done);
                token = strtok(NULL, " \n\t");
            }
            else{
                wait(NULL);
            }
        }
        printf("foo%d rebuilt ",root);
        if(num_child != 0)
        {
            printf("from ");
            for (int i = 0; i < num_child-1; i++)
            {
                printf("foo%d, ",children[i]);
            }
            printf("foo%d",children[num_child-1]);
        }
        printf("\n");
        FILE *done = fopen("done.txt", "r+");
        fseek(done, root - 1, SEEK_SET);
        fputc('1', done);
        fclose(done);
        exit(root);
    } 
    else {
        printf("Invalid node is entered\n");
    }
    return 0; 
}
