/**
 * @file forksort.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 20.12.2019
 *
 * @brief File contains an implementation of forksort.
 *
 * This is a implementation of forksort, using fork() and pipes
 */

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/**
 * @brief Frees all the memory allocated by a list of strings
 *
 * @param list the list of strings
 * @param c length of the list
 */
void freeList(char **list, int c) {
    int i;
    for (i = c; i <= 0; i--) {
        free(list[i]);
    }
}

/**
 * @brief Closes both ends of a pipe
 *
 * @param pipe array[2] containing the two file-descriptors
 */
void closeBoth(int pipe[]) {
    if (close(pipe[0])) {
        log_perror("Could not close 0 of pipe");
        exit(EXIT_FAILURE);
    }
    if (close(pipe[1])) {
        log_perror("Could not close 1 of pipe");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Merges two arrays of strings into one
 *
 * It is assumed that the to arrays passed are already sorted, if they are the returned array will be sorted too.
 *
 * @param a the first array of strings
 * @param b the second array of strings
 * @param aLen the length of the first array
 * @param bLen the length of the second array
 */
char **merge(char **a, char **b, int aLen, int bLen) {
    int len = aLen + bLen;
    char **ret = malloc(len*sizeof(char*));
    if(ret==NULL)  {
        log_perror("Could not malloc memory for merged array");
        exit(EXIT_FAILURE);
    }
    int aIndex = 0;
    int bIndex = 0;
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

/**
 * @brief Reads strings separated by newline into a array. The stream from where the strings
 *        should be red has to be passed as an argument.
 *
 * Memory is allocated dynamically and accordingly to the amount needed.
 *
 * @param l pointer to where to save the amount of strings red
 * @param in the stream from where to read from
 * @return an array of strings in the same order they where red
 */
char **readWords(int *len, FILE *in) {
    char **list = NULL;
    int currentWordLen;
    int wordCount = 0;
    int c;

    while (feof(in) == 0) {
        c = fgetc(in);
        currentWordLen = 0;
        list = realloc(list, (wordCount+1)* sizeof(char*));
        if(list == NULL) {
            log_perror("Could not realloc list for words");
            exit(EXIT_FAILURE);
        }
        list[wordCount] = malloc(1);
        while (feof(in) == 0 && c != '\n') {
            list[wordCount] = realloc(list[wordCount], sizeof(char)*(currentWordLen+1));
            if(list[wordCount] == NULL) {
                log_perror("Could not realloc memory for word");
                exit(EXIT_FAILURE);
            }
            list[wordCount][currentWordLen] = (char) c;
            currentWordLen++;
            c = fgetc(in);
        }
        if (currentWordLen > 0) {
            list[wordCount] = realloc(list[wordCount], (currentWordLen+1) * sizeof(char));
            if(list[wordCount] == NULL) {
                log_perror("Could not realloc memory for null-termination");
                exit(EXIT_FAILURE);
            }
            list[wordCount][currentWordLen] = '\0';
            wordCount++;
        }
    }
    *len = wordCount;
    return list;
}

int main(int argc, char *argv[]) {
    int out_b = dup(STDOUT_FILENO);
    init_logger(argv[0], getpid());
    FILE *in = stdin;
    if(argc>1) {
        in = fopen(argv[1], "r");
    }
    int wordCount = 0;
    char **words = readWords(&wordCount, in);
    if (wordCount == 1) {
        write(out_b, words[0], strlen(words[0]));
        write(out_b, "\0", 1);
        freeList(words, wordCount);
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
        freeList(words, wordCount);
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c1Out) == -1) {
        log_perror("Pipe for c1Out failed");
        freeList(words, wordCount);
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2In) == -1) {
        log_perror("Pipe for c2In failed");
        freeList(words, wordCount);
        exit(EXIT_FAILURE);
    }
    if (pipe(p_c2Out) == -1) {
        log_perror("Pipe for c2Out failed");
        freeList(words, wordCount);
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
        execlp(argv[0], argv[0], NULL);
        log_perror("Could not exec in Child 1");
        freeList(words, wordCount);
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
            execlp(argv[0], argv[0], NULL);
            log_perror("Could not exec in Child 2");
            freeList(words, wordCount);
            exit(EXIT_FAILURE);
        } else if (pidC2 > 0) { // Parent
            if(close(p_c1Out[1]) == -1) {
                perror("Could not close 1 of c1Out pipe");
            }
            if(close(p_c2Out[1]) == -1) {
                perror("Could not close 1 of c2Out pipe");
            }
            int nHalf = wordCount / 2;
            int i;
            FILE *c1_stdin = fdopen(p_c1In[1], "w");
            FILE *c2_stdin = fdopen(p_c2In[1], "w");
            if (c1_stdin == NULL) {
                log_perror("Could not fdopen p_c1In[1]");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
            if (c2_stdin == NULL) {
                log_perror("Could not fdopen p_c2In[1]");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < wordCount; i++) {
                if (i < nHalf) {
                    int written = fprintf(c1_stdin, "%s\n", words[i]);
                    if (written < 1) {
                        log_error("Coudl not write anything to child 1!");
                        freeList(words, wordCount);
                        exit(EXIT_FAILURE);
                    }
                    fflush(c1_stdin);
                } else {
                    int written = fprintf(c2_stdin, "%s", words[i]);
                    if (written < 1) {
                        log_error("Coudl not write anything to child 2!");
                        freeList(words, wordCount);
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
                exit(EXIT_FAILURE);
            }
            if (fclose(c2_stdin) != 0) {
                log_perror("Could not close stdin for child 2");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }

            int firstChildStatus;
            int secondChildStatus;

            waitpid(pidC1, &secondChildStatus, WUNTRACED);
            waitpid(pidC2, &firstChildStatus, WUNTRACED);

            if (WEXITSTATUS(firstChildStatus) == EXIT_SUCCESS && WEXITSTATUS(secondChildStatus) == EXIT_SUCCESS) {
                freeList(words, wordCount);
                FILE *c2_stdout = fdopen(p_c2Out[0], "r");
                FILE *c1_stdout = fdopen(p_c1Out[0], "r");
                if (c1_stdout == NULL) {
                    log_perror("Could not fdopen stdout of c2");
                    freeList(words, wordCount);
                    exit(EXIT_FAILURE);
                }

                if (c2_stdout == NULL) {
                    log_perror("Could not fdopen stdout of c2");
                    freeList(words, wordCount);
                    exit(EXIT_FAILURE);
                }

                int child1Len;
                char **resultChild1 = readWords(&child1Len, c1_stdout);

                int child2Len;
                char **resultChild2 = readWords(&child2Len, c2_stdout);

                char **merged = merge(resultChild1, resultChild2, child1Len, child2Len);

                int totalLen = child1Len + child2Len;
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
                freeList(words, wordCount);
                exit(EXIT_SUCCESS);
            } else {
                log_error("One child didn't exit properly!");
                freeList(words, wordCount);
                exit(EXIT_FAILURE);
            }
        } else {
            log_error("Child 2 could not be forked!");
            freeList(words, wordCount);
            exit(EXIT_FAILURE);
        }
    } else {
        log_error("Child 1 could not be forked!");
        freeList(words, wordCount);
        exit(EXIT_FAILURE);
    }
}

