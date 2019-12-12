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
    for (int i = 0; i < len; i++) {
        if (strcmp(a[aIndex], b[bIndex]) < 0) {
            ret[i] = a[aIndex++];
        } else {
            ret[i] = b[bIndex++];
        }
    }
    return ret;
}

void freeList(char **list, int c) {
    int i;
    for (i = 0; i < c; i++) {
        free(list[i]);
    }
    free(list);
}

char **readWords(int *len) {
    char **list = NULL;
    int c;
    FILE* in = fdopen(STDIN_FILENO, "r");
    c = fgetc(in);
    fprintf(stderr, "  c: %c\n",c);
    int currentWordLen = 0;
    int wordCount = 0;
    while (c != EOF) {
        currentWordLen = 0;
        list = asureMem(list, ++wordCount, sizeof(char *));
        list[wordCount - 1] = asureMem(list[wordCount - 1], 0, sizeof(char));
        while (c != EOF && c != '\n') {
            list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
            list[wordCount - 1][currentWordLen - 1] = (char) c;
            fprintf(stderr, "     redc: %d\n", c);
            c = fgetc(in);
        }
        list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
        list[wordCount - 1][currentWordLen - 1] = '\0';
        fprintf(stderr, "  RED: %s\n", list[wordCount - 1]);
        if (c != EOF) {
            fprintf(stderr, "   waiting for input...\n");
            c = fgetc(in);
        }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    FILE* out = fdopen(STDOUT_FILENO, "w");
    fprintf(stderr, "HALALALALO\n");
    int wordCount = 0;
    char **l = readWords(&wordCount);
    fprintf(stderr, "    L: %d\n", wordCount);
    if (wordCount == 1) {
        fputs(l[0], out);
        fputc(EOF, out);
        fflush(out);
        freeList(l, wordCount);
        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "HELLO: %s", l[0]);

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

    int fdChild1[2];
    int fdChild2[2];
    int statusChild1;
    int statusChild2;
    int i;
    int nHalf = wordCount / 2;

    pipe(fdChild1);
    pipe(fdChild2);

    int n1 = fork();
    int n2 = fork();

    if (n1 > 0 && n2 > 0) { // Parent
        FILE* toChild = fdopen(fdChild1[1], "w");
        for (i = 0; i < wordCount; i++) {
            if (i == nHalf) {
                fprintf(stderr, "SWITCH\n");
                fputc(EOF, toChild);
                fflush(toChild);
                toChild = fdopen(fdChild2[1], "w");
            }
            int sent = fputs(l[i], toChild);
            fputc('\n', toChild);
            fprintf(stderr, "TO(%d): %s\n", sent, l[i]);
        }
        fputc(EOF, toChild);
        fflush(toChild);
        close(fdChild2[1]);

        fprintf(stderr, "PARENT: WAITING(2)\n");
        wait(&statusChild1);
        fprintf(stderr, "PARENT: WAITING(1)\n");
        wait(&statusChild2);
        fprintf(stderr, "PARENT: DONE WAITING\n");

        if (WEXITSTATUS(statusChild1) == EXIT_SUCCESS && WEXITSTATUS(statusChild2) == EXIT_SUCCESS) {
            dup2(fdChild1[0], STDIN_FILENO);
            int child1Len;
            char **resultChild1 = readWords(&child1Len);
            fprintf(stderr, "PARENT: C1_LEN:%d\n",child1Len);
            close(fdChild1[0]);

            dup2(fdChild2[0], STDIN_FILENO);
            int child2Len;
            char **resultChild2 = readWords(&child2Len);
            fprintf(stderr, "PARENT: C2_LEN:%d\n",child2Len);
            close(fdChild2[0]);

            int totalLen = child1Len + child2Len;
            char **merged = merge(resultChild1, resultChild2, totalLen);

            FILE* out = fdopen(STDOUT_FILENO, "w");
            for (int k = 0; i < totalLen; i++) {
                fprintf(out, "%s\n", merged[k]);
            }
            fputc(EOF, out);
            fflush(out);
        } else {
            exit(EXIT_FAILURE);
        }
    } else if (n1 == 0 && n2 > 0) { // Child 1
        sleep(1);
        fprintf(stderr, " CHILD1\n");
        dup2(fdChild1[0], STDIN_FILENO);
        dup2(fdChild1[1], STDIN_FILENO);
        fprintf(stderr," C1 path: %s\n", pathToProgram);
        if(execlp(pathToProgram, PROGRAM_NAME)==-1) {
            fprintf(stderr, "Could not execlp C1\n");
        }
    } else if (n1 > 0 && n2 == 0) { // Child 2
        sleep(2);
        fprintf(stderr, " CHILD2\n");
        dup2(fdChild2[0], STDIN_FILENO);
        dup2(fdChild2[1], STDIN_FILENO);
        fprintf(stderr," C2 path: %s\n", pathToProgram);
        if(execlp(pathToProgram, PROGRAM_NAME)==-1) {
            fprintf(stderr, "Could not execlp C2\n");
        }
    }
    exit(EXIT_SUCCESS);
}
