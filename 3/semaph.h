//
// Created by hannesl on 27.12.19.
//

#ifndef OSUE_SEMAPH_H
#define OSUE_SEMAPH_H

#include <semaphore.h>

sem_t *initSem(char *name, size_t initSize);

sem_t *openSem(char *name);

void removeSem(char *name);

void closeSem(sem_t *sem);

#endif //OSUE_SEMAPH_H
