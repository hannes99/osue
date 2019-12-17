//
// Created by hannesl on 14.12.19.
//

#include "logger.h"

#define PROGRAM_NAME "forksort"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


void freeList(char **list, int c) {
    int i;
    for (i = 0; i < c; i++) {
        free(list[i]);
    }
    free(list);
}

void closeBoth(int pipe[]) {
    if (close(pipe[0])) {
        log_perror("Could not close 0 of pipe");
        exit(EXIT_FAILURE);
    }
    if (close(pipe[1])) {
        log_perror("Could not close 0 of pipe");
        exit(EXIT_FAILURE);
    }
}

void *asureMem(void *p, unsigned long n, size_t size) {
    void *ret = NULL;
    if (p == NULL) {
        ret = malloc(n * size);
    } else {
        ret = realloc(p, n * size);
    }
    if (ret == NULL) {
        log_error("Could not allocate memory!");
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
    return ret;
}

char **readWords(int *len, FILE *in) {
    fprintf(stderr, "ONLY ONCE %d\n", getpid());
    char **list = NULL;
    int currentWordLen;
    int wordCount = 0;
    char c;
    while (feof(in) == 0) {
        fprintf(stderr, "%d WAIT FOR INP1UT...", getpid());
        c = fgetc(in);
        fprintf(stderr, "%d OK\n", getpid());
        currentWordLen = 0;
        list = asureMem(list, ++wordCount, sizeof(char *));
        list[wordCount - 1] = asureMem(list[wordCount - 1], 0, sizeof(char));
        while (feof(in) == 0 && c != '\n') {
            list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
            list[wordCount - 1][currentWordLen - 1] = (char) c;
            fprintf(stderr, "%d WAIT FOR IN2PUT...", getpid());
            c = fgetc(in);
            fprintf(stderr, "%d OK\n", getpid());
        }
        if(currentWordLen>0) {
            list[wordCount - 1] = asureMem(list[wordCount - 1], ++currentWordLen, sizeof(char));
            list[wordCount - 1][currentWordLen - 1] = '\0';
            fprintf(stderr, "   %d: red: %s\n", getpid(), list[wordCount - 1]);
        } else {
            wordCount--;
        }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    init_logger(argv[0]);
    FILE *in = stdin;
    if(argc>1) {
        in = fopen(argv[1],"rb");
    }
    FILE *out = stdout;
    int wordCount = 0;
    char **words = readWords(&wordCount, in);
    fprintf(stderr, "   %d: L: %d\n", getpid(), wordCount);
    if (wordCount == 1) {
        fprintf(stderr, "   %d: DONE\n", getpid());
        fprintf(out, "%s\n", words[0]);
        exit(EXIT_SUCCESS);
    } else if (wordCount == 0) {
        log_error("Called with now words!");
        exit(EXIT_FAILURE);
    }

    int p_c1In[2];
    int p_c1Out[2];
    int p_c2In[2];
    int p_c2Out[2];

    if (pipe(p_c1In) == -1) {
        log_perror("Pipe for c1In failed");
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c1Out) == -1) {
        log_perror("Pipe for c1Out failed");
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2In) == -1) {
        log_perror("Pipe for c2In failed");
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2Out) == -1) {
        log_perror("Pipe for c2Out failed");
        exit(EXIT_FAILURE);
    }

    int pidC1 = fork();
    if (pidC1 == 0) { // Child 1
        closeBoth(p_c2In);
        closeBoth(p_c2Out);
        if (close(p_c1In[1]) == -1) {
            perror("Could not close 1 of c1In pipe");
        }
        if (close(p_c1Out[0]) == -1) {
            perror("Could not close 0 of c1Out pipe");
        }
        dup2(p_c1In[0], STDIN_FILENO);
        dup2(p_c2Out[1], STDOUT_FILENO);
        close(p_c2Out[1]);
        execlp(argv[0], argv[0], NULL);
        log_perror("Could not exec in Child 2");
        exit(EXIT_FAILURE);
    } else if (pidC1 > 0) {
        int pidC2 = fork();
        if (pidC2 == 0) { // Child 2
            closeBoth(p_c1In);
            closeBoth(p_c1Out);
            if (close(p_c2In[1]) == -1) {
                perror("Could not close 1 of c2In pipe");
            }
            if (close(p_c2Out[0]) == -1) {
                perror("Could not close 0 of c2Out pipe");
            }
            dup2(p_c2In[0], STDIN_FILENO);
            dup2(p_c2Out[1], STDOUT_FILENO);
            close(p_c2Out[1]);
            execlp(argv[0], argv[0], NULL);
            log_perror("Could not exec in Child 2");
            exit(EXIT_FAILURE);
        } else if (pidC2 > 0) { // Parent
            int nHalf = wordCount/2;
            int i;
            FILE* c1_stdin = fdopen(p_c1In[1], "wb");
            FILE* c2_stdin = fdopen(p_c2In[1], "wb");
            if(c1_stdin == NULL) {
                log_perror("Could not fdopen p_c1In[1]");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
            if(c2_stdin == NULL) {
                log_perror("Could not fdopen p_c2In[1]");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
            for(i = 0;i<wordCount;i++) {
                fprintf(stderr, "   %d: %s to ", getpid(), words[i]);
                if (i<nHalf) {
                    int written = fprintf(c1_stdin, "%s\n", words[i]);
                    fprintf(stderr, "%d (%d)\n", pidC1, written);
                } else {
                    int written = fprintf(c2_stdin, "%s\n", words[i]);
                    fprintf(stderr, "%d (%d)\n", pidC2, written);
                }
            }
            if(fclose(c1_stdin) != 0) {
                log_perror("Could not close stdin for child 1");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
            if(fclose(c2_stdin) != 0) {
                log_perror("Could not close stdin for child 2");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }

            int firstChildStatus;
            int secondChildStatus;

            waitpid(pidC2, &secondChildStatus, WUNTRACED);
            waitpid(pidC1, &firstChildStatus, WUNTRACED);
            fprintf(stderr, "So am I still waiting!\n");

            if (WEXITSTATUS(firstChildStatus) == EXIT_SUCCESS && WEXITSTATUS(secondChildStatus) == EXIT_SUCCESS) {
                FILE* c1_stdout = fdopen(p_c1Out[0], "rb");
                FILE* c2_stdout = fdopen(p_c2Out[0], "rb");
                if(c1_stdout == NULL) {
                    log_perror("Could not fdopen p_c1Out[0]");
                    freeList(words, wordCount);
                    exit(EXIT_FAILURE);
                }
                if(c2_stdout == NULL) {
                    log_perror("Could not fdopen p_c2Out[0]");
                    freeList(words, wordCount);
                    exit(EXIT_FAILURE);
                }
                int child1Len;
                char **resultChild1 = readWords(&child1Len, c1_stdout);

                int child2Len;
                char **resultChild2 = readWords(&child2Len, c2_stdout);

                fprintf(stderr, " C1: %d, C2: %d\n", child1Len, child2Len);

                int totalLen = child1Len + child2Len;
                char **merged = merge(resultChild1, resultChild2, totalLen);
                fprintf(stderr, "%d\n", totalLen);
                sleep(1);
                for(i = 0;i<totalLen;i++) {
                    fprintf(stdout, "%s\n", merged[i]);
                }
                exit(EXIT_SUCCESS);
            } else {
                log_error("One child didn't exit properly!");
                exit(EXIT_FAILURE);
            }
        } else {
            log_error("Child 2 could not be forked!");
            exit(EXIT_FAILURE);
        }
    } else {
        log_error("Child 1 could not be forked!");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);

}

