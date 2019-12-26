//
// Created by hannesl on 23.12.19.
//

#ifndef OSUE_SHARED_H
#define OSUE_SHARED_H

#include "edge.h"

#define BUFFER_SIZE 20
#define SHM_NAME "/arcshm"
#define READ_SEM "/semread"
#define WRITE_SEM "/semwrite"
#define BUFFER_FULL_SEM "/sembufferf"

#include <semaphore.h>

struct {
    sem_t readSem;
    sem_t writeSem;
    sem_t bufferFullSem;
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
