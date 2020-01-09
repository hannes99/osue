/**
 * @file edge.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 23.12.2019
 *
 * @brief File contains an implementation of a an edge and two types of edge-sets.
 *
 * This is an implementation of an edge and two types of sets for those edges
 * one with dynamic size and one with static size.
 */

#include "edge.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Creates an edge from a correctly formatted string
 *
 * @param edgeString the string describing the edge
 */
edge readEdge(char *edgeString) {
    edge ret;
    if (sscanf(edgeString, "%d-%d", &ret.from, &ret.to) != 2) {
        log_error("Edge does not have the correct format!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

/**
 * @brief Adds an edge to an edgeSet and adjusts it's size accordingly
 *
 * @param s the edgeSet to which the edge should be added
 * @param e the edge that should be added
 */
void addToSet(edgeSet *s, edge e) {
    s->size++;
    s->edges = realloc(s->edges, sizeof(edge) * s->size);
    if (s->edges == NULL) {
        log_error("Could not allocate memory(addToSet)!");
        exit(EXIT_FAILURE);
    }
    s->edges[s->size - 1] = e;
}