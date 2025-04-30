// store visited info in a file
// use hard disk for the array
// initialize the array
// before every child, read the array fresh 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
// #include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        printf("Run with a node number\n");
        return 1;
    }
    char *node = (char *)argv[1];
    if (argc == 2) {
        FILE *foodep = fopen("foodep.txt", "r");
        int n;
        fscanf(foodep, "%d", &n);
        fclose(foodep);
        FILE *done = fopen("done.txt", "w");
        for (int i = 0; i < n; i++) {
            fputc('0', done);
        }
        fclose(done);
    }

    int root = atoi(node);
    // Find the line corresponding to "node"
    FILE *fp = fopen("foodep.txt", "r");
    char line[100];
    char *found = NULL;

    // Skip the first line (which has the total number n)
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        // Check if line starts with the string node (like "9:")
        if (strncmp(node, line, strlen(node)) == 0) {
            found = line;
            break;
        }
    }
    fclose(fp);
    int statuss;
    if (found) {
        char *token = strtok(found, " \t\n");   
        token = strtok(NULL, " \t\n");       

        int num_child = 0;
        int children[100];

        // For each dependency in the line
        while (token != NULL) {
            int child = atoi(token);
            FILE *done = fopen("done.txt", "r+");
            fseek(done, child - 1, SEEK_SET);
            char status = fgetc(done);
            fclose(done);
            if (status == '0') {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process 
                    char *args[] = {"rebuild", token, "1", NULL};
                    execv("rebuild", args);
                } else {
                    // Parent waits
                    wait(&statuss);
                    // print status
                    printf("Child %d exited with status %d\n", pid, WEXITSTATUS(statuss));
                }
            }
            // Keep track of this dependency for printing
            children[num_child++] = child;
            // Move to the next dependency
            token = strtok(NULL, " \t\n");
        }

        // Print the rebuild message
        printf("foo%d rebuilt", root);
        if (num_child > 0) {
            printf(" from ");
            for (int i = 0; i < num_child; i++) {
                if (i > 0) printf(", ");
                printf("foo%d", children[i]);
            }
        }
        printf("\n");
        // Mark this node as rebuilt
        FILE *done = fopen("done.txt", "r+");
        fseek(done, root - 1, SEEK_SET);
        fputc('1', done);
        fclose(done);
        exit(root);
    } else {
        printf("Invalid node is entered\n");
    }
    return 0;
}