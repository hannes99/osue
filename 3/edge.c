//
// Created by hannesl on 23.12.19.
//

#include "edge.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

edge readEdge(char* edgeString) {
    edge ret;
    if(sscanf(edgeString, "%d-%d", &ret.from, &ret.to) != 2) {
        log_error("Edge does not have the correct format!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void addToSet(edgeSet *s, edge e) {
    s->size++;
    s->edges = realloc(s->edges, sizeof(edge) * s->size);
    if (s->edges == NULL) {
        log_error("Could not allocate memory(addToSet)!");
        exit(EXIT_FAILURE);
    }
    s->edges[s->size - 1] = e;
}