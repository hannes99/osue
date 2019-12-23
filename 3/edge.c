//
// Created by hannesl on 23.12.19.
//

#include "edge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *asString(edge e) {
    char *ret = malloc(21);
    sprintf(ret, "%d-%d", e.from, e.to);
    ret = realloc(ret, strlen(ret));
    return ret;
}

char *setAsString(edgeSet s) {
    unsigned long length = 0;
    char *ret = malloc(1);
    ret[0] = '\0';
    int i;
    for(i = 0;i<s.size;i++) {
        char *part = asString(s.edges[i]);
        length += strlen(part)+1;
        ret = realloc(ret, length);
        strcat(ret, " \0");
        strcat(ret, part);
    }
    return ret;
}

edge readEdge(char* edgeString) {
    edge ret;
    if(sscanf(edgeString, "%d-%d", &ret.from, &ret.to) != 2) {
        exit(EXIT_FAILURE);
    }
    return ret;
}

void addToSet(edgeSet *s, edge e) {
    s->size++;
    s->edges = realloc(s->edges, sizeof(edge) * s->size);
    s->edges[s->size-1] = e;
}