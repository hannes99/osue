//
// Created by hannesl on 23.12.19.
//

#include "edge.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *asString(edge e) {
    char *ret = malloc(21);
    if (ret == NULL) {
        log_error("Could not allocate memory!");
        exit(EXIT_FAILURE);
    }
    sprintf(ret, "%d-%d", e.from, e.to);
    ret = realloc(ret, strlen(ret));
    if (ret == NULL) {
        log_error("Could not allocate memory!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

char *setAsString(edgeSet s) {
    unsigned long length = 0;
    char *ret = malloc(1);
    if (ret == NULL) {
        log_error("Could not allocate memory!");
        exit(EXIT_FAILURE);
    }
    ret[0] = '\0';
    int i;
    for (i = 0; i < s.size; i++) {
        char *part = asString(s.edges[i]);
        length += strlen(part) + 1;
        ret = realloc(ret, length);
        if (ret == NULL) {
            log_error("Could not allocate memory!");
            exit(EXIT_FAILURE);
        }
        strcat(ret, " \0");
        strcat(ret, part);
    }
    return ret;
}

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