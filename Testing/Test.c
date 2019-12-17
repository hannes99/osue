//
// Created by hannesl on 14.12.19.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define PROGRAM_NAME "test"

void *asureMem(void *p, unsigned long n, size_t size) {
    void *ret = NULL;
    if (p == NULL) {
        ret = malloc(n * size);
    } else {
        ret = realloc(p, n * size);
    }
    if (ret == NULL) {
        fprintf(stderr, "Couldn't allocate memory!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

int main(int argc, char *argv[]) {
    if(argc == 3) {
        int x = fgetc(stdin);
        fprintf(stdout, "Ladadi %c", x);
        exit(EXIT_SUCCESS);
    }

    int c1in[2];
    int c1out[2];
    int c2in[2];
    int c2out[2];

    pipe(c1in);
    pipe(c1out);
    pipe(c2in);
    pipe(c2out);

    int pathLength = 100;
    char *pathToProgram = NULL;
    pathToProgram = asureMem(pathToProgram, pathLength, 1);
    while(getcwd(pathToProgram, pathLength) == NULL) {
        pathLength += 100;
        pathToProgram = asureMem(pathToProgram, pathLength, 1);
    }
    pathToProgram = asureMem(pathToProgram, strlen(pathToProgram)+strlen(PROGRAM_NAME)+1, 1);
    strcat(pathToProgram, "/");
    strcat(pathToProgram, PROGRAM_NAME);

    int c1 = fork();

    if(c1 > 0) {
        int c2 = fork();
        if(c2 > 0) { // Parent
            close(c1out[1]);
            close(c2out[1]);
            int status;

            FILE* child1_in = fdopen(c1in[1], "w");
            FILE* child1_out = fdopen(c1out[0], "r");
            FILE* child2_in = fdopen(c2in[1], "w");
            FILE* child2_out = fdopen(c2out[0], "r");

            int r = fputc('a', child1_in);
            fprintf(stderr, "wrote a to 1 %d\n", r);
            r = fputc('b', child2_in);
            fprintf(stderr, "wrote a to 2 %d\n", r);
            fclose(child1_in);
            fclose(child2_in);

            fprintf(stdout, "WAITING...");
            wait(&status);
            wait(&status);
            fprintf(stdout, " done\n");
            fflush(stdout);

            int c = fgetc(child1_out);
            while(feof(child1_out) == 0) {
                fprintf(stdout, "%c", c);
                fflush(stdout);
                c = fgetc(child1_out);
            }
            fprintf(stdout, "\nund\n");

            c = fgetc(child2_out);
            while(feof(child2_out) == 0) {
                fprintf(stdout, "%c", c);
                fflush(stdout);
                c = fgetc(child2_out);
            }

        } else { // Child 2
            close(c2in[1]);
            close(c2out[0]);
            close(c1in[1]);
            close(c1in[0]);
            close(c1out[1]);
            close(c1out[0]);
            dup2(c2in[0], STDIN_FILENO);
            dup2(c2out[1], STDOUT_FILENO);
            execlp(pathToProgram, PROGRAM_NAME, "x");
        }
    } else { // Child 1
        close(c1in[1]);
        close(c1out[0]);
        close(c2in[1]);
        close(c2in[0]);
        close(c2out[1]);
        close(c2out[0]);
        dup2(c1in[0], STDIN_FILENO);
        dup2(c1out[1], STDOUT_FILENO);
        execlp(pathToProgram, PROGRAM_NAME, "x");
    }

}
