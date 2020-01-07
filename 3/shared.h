//
// Created by hannesl on 23.12.19.
//

#ifndef OSUE_SHARED_H
#define OSUE_SHARED_H

#include "edge.h"

#define BUFFER_SIZE 20
#define SHM_NAME "/11808227arcshm"
#define READ_S "/11808227readsem"
#define WRITE_S "/11808227writesem"
#define BUFFER_FULL_S "/11808227bufferfullsem"

struct {
    long readPos;
    long writePos;
    limitedEdgeSet buffer[BUFFER_SIZE];
} typedef cyclicArcBuffer;

struct {
    int done;
    cyclicArcBuffer data;
} typedef shm;

shm *initSMem(int *shmfd);

shm *openSMem(int *shmfd);

void closeSMem(shm *mem, int fd);

void removeSMem(shm *mem, int fd);

#endif //OSUE_SHARED_H