//
// Created by hannesl on 11.12.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define DEBUG 1

#define PROGRAM_NAME "forksort"

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

char **merge(char **a, char **b, int len) {
    char **ret = NULL;
    ret = asureMem(ret, len, sizeof(char *));
    int aIndex = 0;
    int bIndex = 0;
    int i;
    for (i = 0; i < len; i++) {
        if (strcmp(a[aIndex], b[bIndex]) < 0) {
            ret[i] = a[aIndex++];
        } else {
            ret[i] = b[bIndex++];
        }
    }
    fprintf(stderr, "===========\n");
    for(i=0;i<len;i++) {
        fprintf(stderr, "%s, ", ret[i]);
    }
    fprintf(stderr, "===========\n");

    return ret;
}

void freeList(char **list, int c) {
    int i;
    for (i = 0; i < c; i++) {
        free(list[i]);
    }
    free(list);
}

char **readWords(int *len, FILE* in) {
    char **list = NULL;
    int c;
    //fprintf(stderr, "   waiting for pipe...");
    c = fgetc(in);
    //fprintf(stderr, " OK\n");

    int currentWordLen = 0;
    int wordCount = 0;
    while (feof(in) == 0) {
        currentWordLen = 0;
        list = asureMem(list, ++wordCount, sizeof(char *));
        list[wordCount - 1] = asureMem(list[wordCount - 1], 0, sizeof(char));
        while (feof(in) == 0 && c != '\n') {
            list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
            list[wordCount - 1][currentWordLen - 1] = (char) c;
            c = fgetc(in);
        }
        list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
        list[wordCount - 1][currentWordLen - 1] = '\0';
        if (feof(in) == 0) {
            c = fgetc(in);
        }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    FILE* in = stdin;
    if(argc==2) {
        in = fopen(argv[1], "r");
    }
    int wordCount = 0;
    char **l = readWords(&wordCount, in);
    if (wordCount == 1) {
        fprintf(stderr, "#only got 1 word %s", l[0]);
        fprintf(stdout, "%s\n", l[0]);
        freeList(l, wordCount);
        fprintf(stderr, " SUCCESS\n");
        exit(EXIT_SUCCESS);
    }
    fprintf(stderr, "#got %d words\n", wordCount);

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

    int child1_stdin[2];
    int child1_stdout[2];
    int child2_stdin[2];
    int child2_stdout[2];
    int statusChild1;
    int statusChild2;
    int i;
    int nHalf = wordCount / 2;

    if(pipe(child1_stdin)!=0) {
        fprintf(stderr, "Pipe failed!");
        exit(EXIT_FAILURE);
    }
    if(pipe(child1_stdout)!=0) {
        fprintf(stderr, "Pipe failed!");
        exit(EXIT_FAILURE);
    }
    if(pipe(child2_stdin)!=0) {
        fprintf(stderr, "Pipe failed!");
        exit(EXIT_FAILURE);
    }
    if(pipe(child2_stdout)!=0) {
        fprintf(stderr, "Pipe failed!");
        exit(EXIT_FAILURE);
    }

    int c1 = fork();

    if (c1 > 0) {
        int c2 = fork();
        if(c2 == 0) { // Child 2
            close(child2_stdin[1]);
            close(child2_stdout[0]);
            close(child1_stdin[1]);
            close(child1_stdin[0]);
            close(child1_stdout[1]);
            close(child1_stdout[0]);
            dup2(child2_stdin[0], STDIN_FILENO);
            close(child2_stdin[0]);
            dup2(child2_stdout[1], STDOUT_FILENO);
            close(child2_stdout[1]);
            if(execlp(pathToProgram, PROGRAM_NAME, NULL)==-1) {
                fprintf(stderr, "Could not execlp C2\n");
            }
        } else { // Parent
            if(close(child1_stdout[1]) != 0) {
                fprintf(stderr, "Could not close pipe!\n");
            }
            if(close(child2_stdout[1]) != 0) {
                fprintf(stderr, "Could not close pipe!\n");
            }

            FILE* child1_in = fdopen(child1_stdin[1], "w");
            FILE* child1_out = fdopen(child1_stdout[0], "r");
            FILE* child2_in = fdopen(child2_stdin[1], "w");
            FILE* child2_out = fdopen(child2_stdout[0], "r");


            if(child1_in == NULL) {
                perror("Child 1");
            }
            if(child1_out == NULL) {
                perror("Child 1");
            }
            if(child2_in == NULL) {
                perror("Child 2");
            }
            if(child2_out == NULL) {
                perror("Child 2");
            }

            for (i = 0; i < wordCount; i++) {
                if (i >= nHalf) {
                    fprintf(child1_in, "%s\n", l[i]);
                } else {
                    fprintf(child2_in, "%s\n", l[i]);
                }
            }
            if(fclose(child1_in) != 0) {
                fprintf(stderr, "Could not close pipe!\n");
            }
            if(fclose(child2_in) != 0) {
                fprintf(stderr, "Could not close pipe!\n");
            }

            wait(&statusChild1);
            wait(&statusChild2);

            if (WEXITSTATUS(statusChild1) == EXIT_SUCCESS && WEXITSTATUS(statusChild2) == EXIT_SUCCESS) {
                int child1Len;
                char **resultChild1 = readWords(&child1Len, child1_out);
                fprintf(stderr, "PARENT: C1_LEN:%d\n",child1Len);

                int child2Len;
                char **resultChild2 = readWords(&child2Len, child2_out);
                fprintf(stderr, "PARENT: C2_LEN:%d\n",child2Len);

                int totalLen = child1Len + child2Len;
                char **merged = merge(resultChild1, resultChild2, totalLen);

                for (int k = 0; i < totalLen; i++) {
                    fprintf(stdout, "%s\n", merged[k]);
                }
                freeList(l, wordCount);
                exit(EXIT_SUCCESS);
            } else {
                exit(EXIT_FAILURE);
            }
        }
    } else if (c1 == 0) { // Child 1
        close(child1_stdin[1]);
        close(child1_stdout[0]);
        close(child2_stdin[1]);
        close(child2_stdin[0]);
        close(child2_stdout[1]);
        close(child2_stdout[0]);
        if(dup2(child1_stdin[0], STDIN_FILENO)==-1){
            fprintf(stderr, "dup2 failed!");
        }
        if(dup2(child1_stdout[1], STDOUT_FILENO)==-1){
            fprintf(stderr, "dup2 failed!");
        }
        if (execlp(pathToProgram, PROGRAM_NAME, NULL) == -1) {
            fprintf(stderr, "Could not execlp C1\n");
        }
    }
    exit(EXIT_FAILURE);
}
