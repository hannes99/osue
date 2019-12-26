//
// Created by hannesl on 23.12.19.
//

#include "logger.h"
#include "shared.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static int running;
static shm *mem;

void sigHandler(int signo) {
    running = 0;
    mem->done = 1;
    sem_post(&mem->data.readSem);
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
    limitedEdgeSet best;
    best.size = 8;
    while (running) {
        sem_wait(&mem->data.readSem);
        long pos = mem->data.readPos % BUFFER_SIZE;
        if (mem->data.buffer[pos].size < best.size) {
            best.size = mem->data.buffer[pos].size;
            fprintf(stdout, "Solution with %d edges:", best.size);
            for (int i = 0; i < best.size; i++) {
                best.edges[i] = mem->data.buffer[pos].edges[i];
                fprintf(stdout, " %d-%d", best.edges[i].from, best.edges[i].to);
            }
            fprintf(stdout, "\n");
            fflush(stdout);
            if (best.size == 0) {
                running = 0;
                sem_post(&mem->data.writeSem);
            }
        }
        sem_post(&mem->data.bufferFullSem);
        mem->data.readPos++;
    }
    mem->done = 1;
    sleep(1);
    removeSMem(mem, shmfd);
}