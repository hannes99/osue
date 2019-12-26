//
// Created by hannesl on 23.12.19.
//

#ifndef OSUE_EDGE_H
#define OSUE_EDGE_H

#include <bits/types/FILE.h>

#define LIMITED_EDGESET_SIZE 8

typedef struct {
    int from;
    int to;
} edge;

typedef struct {
    edge *edges;
    int size;
} edgeSet;

typedef struct {
    edge edges[LIMITED_EDGESET_SIZE];
    int size;
} limitedEdgeSet;

void addToSet(edgeSet *s, edge e);

edge readEdge(char *edgeString);

#endif //OSUE_EDGE_H
