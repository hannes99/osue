//
// Created by hannesl on 14.12.19.
//

#include "logger.h"

#define PROGRAM_NAME "forksort"
#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

static FILE *logout;


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
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
    if (close(pipe[1])) {
        log_perror("Could not close 0 of pipe");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
}

char **merge(char **a, char **b, int aLen, int len) {
    char **ret = NULL;
    ret = malloc(len * sizeof(char));
    if(ret==NULL) exit(EXIT_FAILURE);
    int aIndex = 0;
    int bIndex = 0;
    int bLen = len - aLen;
    int i;
    for (i = 0; i < len; i++) {
        if (aIndex < aLen && (bIndex >= bLen || strcmp(a[aIndex], b[bIndex]) < 0)) {
            ret[i] = a[aIndex++];
        } else {
            ret[i] = b[bIndex++];
        }
    }
    return ret;
}

char **readWords(int *len, FILE *in) {
    char **list = NULL;
    int currentWordLen;
    int wordCount = 0;
    int c;

    if (DEBUG) {
        fprintf(logout, "READING...\n");
        fflush(logout);
    }

    while (feof(in) == 0) {
        if (DEBUG) {
            fprintf(logout, "\t\tX");
            fflush(logout);
        }
        c = fgetc(in);
        currentWordLen = 0;
        list = realloc(list, (wordCount+1)* sizeof(char*));
        list[wordCount] = malloc(1);
        while (feof(in) == 0 && c != '\n') {
            list[wordCount] = realloc(list[wordCount], sizeof(char)*(currentWordLen+1));
            list[wordCount][currentWordLen] = (char) c;
            currentWordLen++;
            if (DEBUG) {
                fprintf(logout, "%c", list[wordCount][currentWordLen - 1]);
                fflush(logout);
            }
            c = fgetc(in);
            if (DEBUG) {
                fprintf(logout, ".");
                fflush(logout);
            }
        }
        if (currentWordLen > 0) {
            list[wordCount] = realloc(list[wordCount], (currentWordLen+1)* sizeof(char));
            list[wordCount][currentWordLen] = '\0';
            wordCount++;
        }
        if (DEBUG) { fprintf(logout, "\n"); }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    int out_b = dup(1);
    init_logger(argv[0]);
    if (DEBUG == 1) {
        char logPath[50];
        sprintf(logPath, "logs/%d.txt", getpid());
        logout = fopen(logPath, "wb");
    }
    FILE *in = stdin;
    if(argc>1) {
        in = fopen(argv[1], "rb");
    }
    int wordCount = 0;
    char **words = readWords(&wordCount, in);
    if(DEBUG) {fprintf(logout, "CALLED!%d", wordCount);}
    if (wordCount == 1) {
        fprintf(logout, " DONE(%s)!\n", words[0]);
        write(out_b, words[0], strlen(words[0]));
        if (DEBUG) { fclose(logout); }
        exit(EXIT_SUCCESS);
    } else if (wordCount == 0) {
        log_error("Called with now words!");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
    fprintf(logout, " NOT DONE!\n");

    int p_c1In[2];
    int p_c1Out[2];
    int p_c2In[2];
    int p_c2Out[2];

    if (pipe(p_c1In) == -1) {
        log_perror("Pipe for c1In failed");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c1Out) == -1) {
        log_perror("Pipe for c1Out failed");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2In) == -1) {
        log_perror("Pipe for c2In failed");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2Out) == -1) {
        log_perror("Pipe for c2Out failed");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    }

    int pidC1 = fork();
    if (pidC1 == 0) { // Child 1
        closeBoth(p_c2In);
        closeBoth(p_c2Out);
        if (close(p_c1In[1]) == -1) {
            perror("Could not close 1 of c1In pipe");
        }

        dup2(p_c1In[0], STDIN_FILENO);
        dup2(p_c1Out[1], STDOUT_FILENO);
        //close(p_c1Out[1]);
        execlp(argv[0], argv[0], NULL);
        log_perror("Could not exec in Child 2");
        if (DEBUG) { fclose(logout); }
        exit(EXIT_FAILURE);
    } else if (pidC1 > 0) {
        int pidC2 = fork();
        if (pidC2 == 0) { // Child 2
            closeBoth(p_c1In);
            closeBoth(p_c1Out);
            if (close(p_c2In[1]) == -1) {
                perror("Could not close 1 of c2In pipe");
            }

            dup2(p_c2In[0], STDIN_FILENO);
            dup2(p_c2Out[1], STDOUT_FILENO);
            //close(p_c2Out[1]);
            execlp(argv[0], argv[0], NULL);
            log_perror("Could not exec in Child 2");
            if (DEBUG) { fclose(logout); }
            exit(EXIT_FAILURE);
        } else if (pidC2 > 0) { // Parent
            if (DEBUG) {
                fprintf(logout, "STARTED %d as child 1\n", pidC1);
                fprintf(logout, "STARTED %d as child 2\n", pidC2);
                fflush(logout);
            }
            if(close(p_c1Out[1]) == -1) {
                perror("Could not close 1 of c1Out pipe");
            }
            if(close(p_c2Out[1]) == -1) {
                perror("Could not close 1 of c2Out pipe");
            }
            int nHalf = wordCount / 2;
            int i;
            FILE *c1_stdin = fdopen(p_c1In[1], "wb");
            FILE *c2_stdin = fdopen(p_c2In[1], "wb");
            if (c1_stdin == NULL) {
                log_perror("Could not fdopen p_c1In[1]");
                freeList(words, wordCount);
                if (DEBUG) { fclose(logout); }
                exit(EXIT_FAILURE);
            }
            if (c2_stdin == NULL) {
                log_perror("Could not fdopen p_c2In[1]");
                freeList(words, wordCount);
                if (DEBUG) { fclose(logout); }
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < wordCount; i++) {
                if (i < nHalf) {
                    int written = fprintf(c1_stdin, "%s\n", words[i]);
                    if (DEBUG) {
                        fprintf(logout, "\t %s to c1(%d)\n", words[i], written);
                        fflush(logout);
                    }
                    if (written < 1) {
                        log_error("Coudl not write anything to child 1!");
                        freeList(words, wordCount);
                        if (DEBUG) { fclose(logout); }
                        exit(EXIT_FAILURE);
                    }
                    fflush(c1_stdin);
                } else {
                    int written = fprintf(c2_stdin, "%s", words[i]);
                    if (DEBUG) {
                        fprintf(logout, "\t %s to c2(%d)\n", words[i], written);
                        fflush(logout);
                    }
                    if (written < 1) {
                        log_error("Coudl not write anything to child 2!");
                        freeList(words, wordCount);
                        if (DEBUG) { fclose(logout); }
                        exit(EXIT_FAILURE);
                    }
                    if(i<wordCount-1) {
                        fprintf(c2_stdin, "\n");
                    }
                    fflush(c2_stdin);
                }
            }
            if (fclose(c1_stdin) != 0) {
                log_perror("Could not close stdin for child 1");
                freeList(words, wordCount);
                if (DEBUG) { fclose(logout); }
                exit(EXIT_FAILURE);
            }
            if (fclose(c2_stdin) != 0) {
                log_perror("Could not close stdin for child 2");
                freeList(words, wordCount);
                if (DEBUG) { fclose(logout); }
                exit(EXIT_FAILURE);
            }

            int firstChildStatus;
            int secondChildStatus;

            waitpid(pidC1, &secondChildStatus, WUNTRACED);
            if (DEBUG) {
                fprintf(logout, "C1 terminated\n");
                fflush(logout);
            }
            waitpid(pidC2, &firstChildStatus, WUNTRACED);
            if (DEBUG) {
                fprintf(logout, "C2 terminated\n");
                fflush(logout);
            }

            if (WEXITSTATUS(firstChildStatus) == EXIT_SUCCESS && WEXITSTATUS(secondChildStatus) == EXIT_SUCCESS) {
                freeList(words, wordCount);
                FILE *c2_stdout = fdopen(p_c2Out[0], "rb");
                FILE *c1_stdout = fdopen(p_c1Out[0], "rb");
                if (c1_stdout == NULL) {
                    log_perror("Could not fdopen p_c1Out[0]");
                    freeList(words, wordCount);
                    if (DEBUG) { fclose(logout); }
                    exit(EXIT_FAILURE);
                }

                if (c2_stdout == NULL) {
                    log_perror("Could not fdopen p_c2Out[0]");
                    freeList(words, wordCount);
                    if (DEBUG) { fclose(logout); }
                    exit(EXIT_FAILURE);
                }

                int child1Len;
                char **resultChild1 = readWords(&child1Len, c1_stdout);
                if (DEBUG) {
                    fprintf(logout, "got %d words from c1 back\n", child1Len);
                    fflush(logout);
                }

                int child2Len;
                char **resultChild2 = readWords(&child2Len, c2_stdout);
                if (DEBUG) {
                    fprintf(logout, "got %d words from c2 back\n", child2Len);
                    fflush(logout);
                }

                int totalLen = child1Len + child2Len;
                char **merged = merge(resultChild1, resultChild2, child1Len, totalLen);

                if(DEBUG) {
                    fprintf(logout, "OUTPUT:\n");
                    fflush(logout);
                    for (i = 0; i < totalLen; i++) {
                        fprintf(logout, "%s\n", merged[i]);
                        fflush(logout);
                    }
                }

                for (i = 0; i < totalLen; i++) {
                    if(write(out_b, merged[i], strlen(merged[i])) != strlen(merged[i])){
                        log_error("Didnt write word successfully!");
                    }
                    if(i<totalLen-1) {
                        if(write(out_b, "\n", 1) != 1){
                            log_error("Didnt write new line successfully!");
                        }
                    }
                }

                if (DEBUG) { fclose(logout); }
                exit(EXIT_SUCCESS);
            } else {
                if (DEBUG) {
                    fprintf(logout, "A child terminated with error!\n");
                    fflush(logout);
                }
                log_error("One child didn't exit properly!");
                if (DEBUG) { fclose(logout); }

                exit(EXIT_FAILURE);
            }
        } else {
            log_error("Child 2 could not be forked!");
            if (DEBUG) { fclose(logout); }
            exit(EXIT_FAILURE);
        }
    } else {
        log_error("Child 1 could not be forked!");
        if (DEBUG) { fclose(logout); }

        exit(EXIT_FAILURE);
    }
    if (DEBUG) { fclose(logout); }
    exit(EXIT_SUCCESS);

}

