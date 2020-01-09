/**
 * @file generator.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 23.12.2019
 *
 * @brief File contains an implementation of a generator generating random solutions for the arc set problem.
 *
 * Those solution are written from generator programs to a shared memory. Solutions
 * are buffered in a cyclic buffer of static size.
 */

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

/**
 * @brief Shuffles an array of a given size.
 *
 * @param array the array to be shuffled
 * @param n the size of the array
 */
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

/**
 * @brief Selects edges that form a feedback arc set
 *
 * @param nodes nodes in the graph
 * @param edges edgeSet to select the edges from
 */
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

/**
 * @brief Reads and converts the edges from argv to an edgeSet
 *
 * @param edges count of edges
 * @param nodes pointer to where the amount of nodes in the graph should be saved
 * @param argv the arguments that were passed
 * @return edgeSet containing the edges specified in argv
 */
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
    init_logger("generator", getpid());
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
    int done = 0;
    while (done == 0) {
        edgeSet vs = getValidEdges(nodes, &edges);
        if (sem_wait(fullSem) == -1) {
            log_perror("Could not wait for full sem");
            break;
        }
        if (sem_wait(writeSem) == -1) {
            log_perror("Could not wait for write sem");
            break;
        }
        done = mem->done;
        if (done == 0) {
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
            } else {
                if (sem_post(fullSem) == -1) {
                    log_perror("Could not post full sem");
                    exit(EXIT_FAILURE);
                }
            }
            shuffle(nodes, nodeCount);
        } else {
            if (sem_post(fullSem) == -1) {
                log_perror("Could not post full sem");
                exit(EXIT_FAILURE);
            }
        }
        free(vs.edges);
        sem_post(writeSem);
    }
    closeSMem(mem, shmfd);
    closeSem(readSem);
    closeSem(writeSem);
    closeSem(fullSem);
    log_error("DONE");
    exit(EXIT_SUCCESS);
}