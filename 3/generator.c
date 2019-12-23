//
// Created by hannesl on 23.12.19.
//

#include "edge.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = n - 1; i > 0; i--) {
            // Pick a random index from 0 to i
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
    int nodeCount = 0;
    int i;
    edgeSet edges = readEdges(argc - 1, &nodeCount, argv);
    int nodes[nodeCount];
    for (i = 0; i < nodeCount; i++) {
        nodes[i] = i;
    }
    int best = argc;
    while (best > 0) {
        edgeSet vs = getValidEdges(nodes, &edges);
        if (vs.size < best) {
            best = vs.size;
            fprintf(stdout, "Best solution: %d\n", vs.size);
        }
        shuffle(nodes, nodeCount);
    }
    fprintf(stdout, "Graph was acyclic!\n");
    exit(EXIT_SUCCESS);
}
