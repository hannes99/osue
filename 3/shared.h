//
// Created by hannesl on 23.12.19.
//

#ifndef OSUE_SHARED_H
#define OSUE_SHARED_H

#include "edge.h"

#define BUFFER_SIZE 20
#define SHM_NAME "/arcshm"
#define READ_S "/readsem"
#define WRITE_S "/writesem"
#define BUFFER_FULL_S "/bufferfullsem"

struct {
    //TODO di semaphoers solletn bitte net in sm seine, jeder tuat se jo selbr auf, in sm uanfoch lai die namen fa die semaphores speichern
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
