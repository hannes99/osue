//
// Created by hannesl on 23.12.19.
//

#include "edge.h"
#include "logger.h"
#include "shared.h"
#include "semaph.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#define VERBOSE 0


int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = n - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

edgeSet getValidEdges(const int nodes[], edgeSet *edges) {
    int i, o;
    edgeSet ret;
    ret.size = 0;
    ret.edges = NULL;
    for (i = 0; i < edges->size; i++) {
        o = 0;
        while (nodes[o] != edges->edges[i].from) {
            if (nodes[o++] == edges->edges[i].to) {
                addToSet(&ret, edges->edges[i]);
                break;
            }
        }
    }
    return ret;
}

edgeSet readEdges(int edges, int *nodes, char *argv[]) {
    edgeSet ret;
    ret.size = 0;
    ret.edges = NULL;
    int i;
    for (i = 0; i < edges; i++) {
        addToSet(&ret, readEdge(argv[i + 1]));
        *nodes = max(*nodes, max(ret.edges[i].from, ret.edges[i].to));
    }
    *nodes = *nodes + 1;
    return ret;
}

int main(int argc, char *argv[]) {
    init_logger("Arc", getpid());
    srand(time(0) + getpid());
    int shmfd;
    shm *mem = openSMem(&shmfd);
    sem_t *readSem = openSem(READ_S);
    sem_t *writeSem = openSem(WRITE_S);
    sem_t *fullSem = openSem(BUFFER_FULL_S);
    int nodeCount = 0;
    int i;
    edgeSet edges = readEdges(argc - 1, &nodeCount, argv);
    int nodes[nodeCount];
    for (i = 0; i < nodeCount; i++) {
        nodes[i] = i;
    }
    int run = 0;
    while (run == 0) {
        edgeSet vs = getValidEdges(nodes, &edges);
        if (run==1) sem_wait(fullSem);
        sem_wait(writeSem);
        run = mem->done;
        long pos = mem->data.writePos % BUFFER_SIZE;
        if (vs.size <= LIMITED_EDGESET_SIZE) {
            if (VERBOSE) fprintf(stdout, "%d\n", vs.size);
            mem->data.buffer[pos].size = vs.size;
            for (int i = 0; i < vs.size; i++) {
                mem->data.buffer[pos].edges[i] = vs.edges[i];
            }
            mem->data.writePos++;
            if (sem_post(readSem) == -1) {
                log_perror("Could not post read sem");
                exit(EXIT_FAILURE);
            }
        }
        free(vs.edges);
        sem_post(writeSem);
        shuffle(nodes, nodeCount);
    }
    closeSMem(mem, shmfd);
    closeSem(readSem);
    closeSem(writeSem);
    closeSem(fullSem);
    fprintf(stdout, "DONE");
    exit(EXIT_SUCCESS);
}