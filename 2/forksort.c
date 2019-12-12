//
// Created by hannesl on 11.12.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define DEBUG 0
#define DEBUG_FILE "test.in"

void *asureMem(void *p, int n, size_t size) {
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
    read(STDIN_FILENO, &c, 1);
    int currentWordLen = 0;
    int wordCount = 0;
    while (c != EOF) {
        currentWordLen = 0;
        list = asureMem(list, ++wordCount, sizeof(char *));
        list[wordCount - 1] = asureMem(list[wordCount - 1], 0, sizeof(char));
        while (c != EOF && c != '\n') {
            list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
            list[wordCount - 1][currentWordLen - 1] = (char) c;
            read(STDIN_FILENO, &c, 1);
        }
        list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
        list[wordCount - 1][currentWordLen - 1] = '\0';
        fprintf(stderr, "  RED: %s\n", list[wordCount - 1]);
        if (c != EOF) {
            read(STDIN_FILENO, &c, 1);
        }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    int wordCount = 0;
    char **l = readWords(&wordCount);
    if (wordCount == 1) {
        fprintf(stdout, "%s", l[0]);
        freeList(l, wordCount);
        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "HELLO: %s", l[0]);

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
        int toChild = fdChild1[1];
        for (i = 0; i < wordCount; i++) {
            fprintf(stderr, " %d\n",i);

            if (i == nHalf) {
                fprintf(stderr, "SWITCH\n");
                toChild = fdChild2[1];
                close(fdChild1[1]);
            }
            int sent = write(toChild, l[i], strlen(l[i]));
            fprintf(stderr, "TO(%d)(%d): %s\n", toChild, sent, l[i]);
            write(toChild, "\n", 1);
        }
        close(fdChild2[1]);

        fprintf(stderr, "PARENT: WAITING(2)\n");
        wait(&statusChild1);
        fprintf(stderr, "PARENT: WAITING(1)\n");
        wait(&statusChild2);
        fprintf(stderr, "PARENT: DONE WAITING\n");

        if (WEXITSTATUS(statusChild1) == EXIT_SUCCESS && WEXITSTATUS(statusChild2) == EXIT_SUCCESS) {
            dup2(STDIN_FILENO, fdChild1[0]);
            int child1Len;
            char **resultChild1 = readWords(&child1Len);
            fprintf(stderr, "PARENT: C1_LEN:%d\n",child1Len);
            close(fdChild1[0]);

            dup2(STDIN_FILENO, fdChild2[0]);
            int child2Len;
            char **resultChild2 = readWords(&child2Len);
            fprintf(stderr, "PARENT: C2_LEN:%d\n",child2Len);
            close(fdChild2[0]);

            int totalLen = child1Len + child2Len;
            char **merged = merge(resultChild1, resultChild2, totalLen);

            for (int k = 0; i < totalLen; i++) {
                fprintf(stdout, "%s\n", merged[k]);
            }
            fputc(EOF, stdout);
        } else {
            exit(EXIT_FAILURE);
        }
    } else if (n1 == 0 && n2 > 0) { // Child 1
        sleep(5);
        fprintf(stderr, " CHILD1\n");
        dup2(STDIN_FILENO, fdChild1[1]);
        dup2(STDOUT_FILENO, fdChild1[0]);
        execlp("forksort", "forksort");
    } else if (n1 > 0 && n2 == 0) { // Child 2
        sleep(2);
        fprintf(stderr, " CHILD2\n");
        dup2(STDIN_FILENO, fdChild2[1]);
        dup2(STDOUT_FILENO, fdChild2[0]);
        execlp("forksort", "forksort");
    }
    exit(EXIT_SUCCESS);
}
