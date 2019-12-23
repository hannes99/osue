//
// Created by hannesl on 23.12.19.
//

#ifndef OSUE_EDGE_H
#define OSUE_EDGE_H

#include <bits/types/FILE.h>

typedef struct {
    int from;
    int to;
} edge;

typedef struct {
    edge *edges;
    int size;
} edgeSet;

char *asString(edge e);
char *setAsString(edgeSet e);
void addToSet(edgeSet *s, edge e);
edge readEdge(char *edgeString);

#endif //OSUE_EDGE_H
