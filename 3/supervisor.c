/**
 * @file supervisor.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 23.12.2019
 *
 * @brief File contains an implementation of a supervisor managing solutions of generators.
 *
 * Those solution are written from generator programs to a shared memory where the supervisor reads it from. Solutions
 * are buffered in a cyclic buffer of static size.
 */

#include "logger.h"
#include "shared.h"
#include "semaph.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static int running;
static shm *mem;
static sem_t *readSem, *writeSem, *fullSem;

void sigHandler(int signo) {
    running = 0;
    mem->done = 1;
    sem_post(readSem);
    sem_post(fullSem);
}

int main(int argc, char *argv[]) {
    init_logger("supervisor", getpid());
    int shmfd;
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        log_perror("Could not bind signal handler for SIGINT");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGTERM, sigHandler) == SIG_ERR) {
        log_perror("Could not bind signal handler for SIGTERM");
        exit(EXIT_FAILURE);
    }
    running = 1;
    mem = initSMem(&shmfd);
    readSem = initSem(READ_S, 0);
    writeSem = initSem(WRITE_S, 1);
    fullSem = initSem(BUFFER_FULL_S, BUFFER_SIZE);
    limitedEdgeSet best;
    best.size = LIMITED_EDGESET_SIZE;
    while (running == 1) {
        if (sem_wait(readSem) == -1) {
            log_perror("Could not wait for read sem");
            running = 0;
        }
        if (running == 1) {
            long pos = mem->data.readPos % BUFFER_SIZE;
            if (mem->data.buffer[pos].size < best.size) {
                best.size = mem->data.buffer[pos].size;
                if (best.size > 0) {
                    fprintf(stdout, "Solution with %d edges:", best.size);
                    for (int i = 0; i < best.size; i++) {
                        best.edges[i] = mem->data.buffer[pos].edges[i];
                        fprintf(stdout, " %d-%d", best.edges[i].from, best.edges[i].to);
                    }
                    fprintf(stdout, "\n");
                    fflush(stdout);
                } else {
                    fprintf(stdout, "The graph is acyclic!\n");
                    running = 0;
                    if (sem_post(writeSem) == -1) {
                        log_perror("Could not post writeSem");
                        running = 0;
                        mem->done = 1;
                    }
                }
            }
            if (sem_post(fullSem) == -1) {
                log_perror("Could not post fullSem");
                running = 0;
                mem->done = 1;
            }
            mem->data.readPos++;
        }
    }
    mem->done = 1;
    sleep(1);
    removeSMem(mem, shmfd);
    removeSem(READ_S);
    removeSem(WRITE_S);
    removeSem(BUFFER_FULL_S);
    exit(EXIT_SUCCESS);
}