//
// Created by hannesl on 23.12.19.
//

#include "edge.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


int max(int a, int b) {
    if(a > b) {
        return a;
    }
    return b;
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
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
    for(i = 0;i<edges->size;i++) {
        o = 0;
        while(nodes[o] != edges->edges[i].from) {
            if(nodes[o++] == edges->edges[i].to) {
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
    for(i = 0;i<edges;i++) {
        addToSet(&ret, readEdge(argv[i+1]));
        *nodes = max(*nodes, max(ret.edges[i].from, ret.edges[i].to));
    }
    return ret;
}

int main(int argc, char *argv[]) {
    srand(time(0));
    int nodeCount = 0;
    int i;
    edgeSet edges = readEdges(argc-1, &nodeCount, argv);
    int nodes[nodeCount];
    for(i = 1;i<=nodeCount;i++) {
        nodes[i] = i;
    }

    while(1) {
        //sleep(1);
        shuffle(nodes, nodeCount);
        edgeSet vs = getValidEdges(nodes, &edges);
        if(vs.size == 0) {
            fprintf(stdout, "Solution(%d): %s\n", vs.size, setAsString(vs));
        }
    }

    fprintf(stdout, "Nodes: %d\n", nodeCount);
    for(i = 0;i<argc-1;i++) {
        fprintf(stdout, "%s\n", asString(edges.edges[i]));
    }
    exit(EXIT_SUCCESS);
}
